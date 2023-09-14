#!/usr/bin/env python3

import argparse
import atexit
from contextlib import ExitStack
from json import dump
from os import makedirs, path
import random
import signal
import sys
from time import time

import joblib
import numpy as np
import rpyc
import tensorflow as tf
# for some reason "import tensorflow.python.debug" doesn't work (maybe it's a
# class or something?)
from tensorflow.python import debug as tfdbg

from asnets.models import PropNetworkWeights, PropNetwork
from asnets.supervised import SupervisedTrainer, SupervisedObjective, \
    ProblemServiceConfig
from asnets.multiprob import ProblemServer, to_local, parent_death_pact
from asnets.utils.prof_utils import can_profile
from asnets.utils.py_utils import set_random_seeds
import tqdm


class CachingPolicyEvaluator(object):
    """Can be used to ensure that we evaluate policy only once for each state
    encountered at test time."""

    def __init__(self, policy, det_sample):
        self.det_sample = det_sample
        self.policy = policy
        self.cache = {}
        self._misses = 0
        self._hits = 0

    def get_action(self, obs, count=1):
        assert obs.ndim == 1
        obs_key = obs.tobytes()
        if obs_key in self.cache:
            self._hits += 1
            act_dist = self.cache[obs_key]
        else:
            self._misses += 1
            in_obs = obs[None, :]
            act_dist = self.policy(in_obs, training=False).numpy().squeeze(0)
            self.cache[obs_key] = act_dist
        # we cache action *distribution* instead of action so that we can draw
        # a different random sample each time (caching should be transparent!)
        if self.det_sample:
            # If it is the first time we enter this state, the take the best action
            if count == 1:
                action = int(np.argmax(act_dist))
            # Otherwise, take the next-best action
            else:
                if count >= act_dist.shape[-1]:
                    action = -1
                else:
                    inds = np.argpartition(-act_dist, count)[:count]
                    vals = act_dist[inds]
                    action = int(inds[np.argsort(vals)[0]])
        else:
            num_actions = act_dist.shape[-1]
            act_indices = np.arange(num_actions)
            action = int(np.random.choice(act_indices, p=act_dist))
        return action


@can_profile
def run_trial(policy_evaluator, problem_server, limit=1000, det_sample=False):
    """Run policy on problem. Returns (cost, path), where cost may be None if
    goal not reached before horizon."""
    problem_service = problem_server.service
    # 'obs' is actually a numpy vector that's already prepared to be stuffed
    # into our network
    init_cstate = to_local(problem_service.env_reset())
    obs = init_cstate.to_network_input()
    # total cost of this run
    cost = 0
    states = {} # {state: (position, count)}
    path = []
    cur_state = init_cstate
    for step in range(0, limit):
        # If a state appears before (a loop), remove the loop and try the next-best action
        if str(cur_state) in states.keys():
            loop_index, count = states[str(cur_state)]
            path = path[:loop_index]
            count += 1
            states[str(cur_state)] = (loop_index, count)
        # record a state when it first appears in the plan
        else:
            count = 1
            states[str(cur_state)] = (len(path), count)
        action = policy_evaluator.get_action(obs, count)
        # if cannot break the loop, return "fail"
        if action < 0:
            break
        new_cstate, step_cost = to_local(problem_service.env_step(action))
        new_obs = new_cstate.to_network_input()
        path.append(to_local(problem_service.action_name(action)))
        obs = new_obs
        cur_state = new_cstate
        cost += step_cost
        if new_cstate.is_goal:
            # path.append('GOAL! :D')
            return cost, True, path
        # we can run out of time or run out of actions to take
        if new_cstate.is_terminal:
            break
    # path.append('FAIL! D:')
    return cost, False, path



def run_trials(policy, problem_server, trials, limit=1000, det_sample=False):
    policy_evaluator = CachingPolicyEvaluator(
        policy=policy, det_sample=det_sample)
    all_exec_times = []
    all_costs = []
    all_goal_reached = []
    paths = []
    for _ in tqdm.trange(trials, desc='trials', leave=True):
        start = time()
        cost, goal_reached, path = run_trial(policy_evaluator, problem_server,
                                             limit, det_sample)
        elapsed = time() - start
        paths.append(path)
        all_exec_times.append(elapsed)
        all_costs.append(cost)
        all_goal_reached.append(goal_reached)
        print("%d trials of length %d took %fs" % (trials, limit, elapsed))

    meta_dict = {
        'turn_limit': limit,
        'trials': trials,
        'all_goal_reached': all_goal_reached,
        'all_exec_times': all_exec_times,
        'all_costs': all_costs,
    }
    return meta_dict, paths


def unique_name(args, digits=6):
    rand_num = random.randint(1, (1 << (4 * (digits + 1)) - 1))
    suffix = '{:x}'.format(rand_num).zfill(digits)
    if args.timeout is None:
        time_str = 'inf'
    else:
        time_str = '%d' % round(args.timeout)
    mo_str = ','.join('%s=%s' % (k, v) for k, v in args.model_opts.items())
    if args.problems:
        all_probs_comma = ','.join(args.problems)
        if len(all_probs_comma) > 50:
            all_probs_comma = all_probs_comma[:47] + '...'
        start = 'P[{}]'.format(all_probs_comma)
    else:
        names = []
        for pf in args.pddls:
            # remove directory path
            bn = path.basename(pf)
            pf_suffix = '.pddl'
            if bn.endswith(pf_suffix):
                # chop off extension
                bn = bn[:-len(pf_suffix)]
            if bn:
                names.append(bn)
        all_names_comma = ','.join(names)
        if len(all_names_comma) > 50:
            all_names_comma = all_names_comma[:47] + '...'
        start = 'P[%s]' % all_names_comma
    prefix = '{}-S[{},{},{}]-MO[{}]-T[{}]'.format(
        start, args.supervised_lr, args.supervised_bs, args.ssipp_teacher_heur,
        mo_str, time_str)
    start_time_str = str(int(time() / 60 - 24881866)).zfill(8)
    return prefix + '-' + start_time_str + '-' + suffix


def opt_str(in_str):
    rv = {}
    for item in in_str.split(','):
        item = item.strip()
        if not item:
            continue
        name, value = item.split('=', 1)
        rv[name] = value
    return rv


def sup_objective_str(in_str):
    return SupervisedObjective[in_str]


def int_or_float(arg_str):
    """Convert string to non-negative integer (preferred) or float."""
    if arg_str.isnumeric():
        return int(arg_str)
    try:
        result = float(arg_str)
        if result < 0:
            raise ValueError("value can't be negative")
        return result
    except ValueError:
        raise argparse.ArgumentTypeError(
            "Could not convert argument '%s' to non-negative int or float" %
            (arg_str, ))


parser = argparse.ArgumentParser(description='Trainer for ASNets')
parser.add_argument(
    '-p',
    '--problem',
    dest='problems',
    action='append',
    help='name of problem to solve (can use this flag many times)')
parser.add_argument(
    '--opt-patience',
    type=int,
    default=10,
    help="if best observed undiscounted mean reward is >=1, *and* there has "
    "been no improvement for this many epochs, then we stop.")
parser.add_argument(
    '--max-opt-epochs',
    type=int,
    default=100,
    help="absolute maximum number of epochs to do optimisation for")
parser.add_argument(
    '--supervised-lr',
    type=float,
    default=0.0005,
    help='learning rate for supervised learning')
parser.add_argument(
    '--lr-step',
    nargs=2,
    action='append',
    type=int_or_float,
    default=[],
    dest='lr_steps',
    help='specifying "k r" will step down to LR `r` after `k` epochs (can be '
    'given multiple times)')
parser.add_argument(
    '--supervised-bs',
    type=int,
    default=128,
    help='batch size for supervised learning')
parser.add_argument(
    '--ssipp-teacher-heur',
    default='lm-cut',
    choices=['lm-cut', 'h-add', 'h-max', 'simpleZero', 'smartZero'],
    help='heuristic to use for SSiPP teacher in supervised mode')
parser.add_argument(
    '--fd-teacher-heuristic',
    default='astar-hadd',
    choices=['astar-hadd', 'lama-2011', 'lama-first', 
             'lama-w5', 'lama-w3', 'lama-w2', 'lama-w1',
             'astar-lmcut','astar-lmcount','astar-hadd',
             'gbf-lmcut', 'gbf-hadd'],
    help='heuristic to use for fd teacher in supervised mode')
parser.add_argument(
    '--supervised-early-stop',
    type=int,
    default=12,
    help='halt after this many epochs with succ. rate >0.8 & no increase (0 '
    'disables)')
parser.add_argument(
    '--save-every',
    type=int,
    default=0,
    metavar='N',
    help='save models every N epochs, in addition to normal saves for best '
    'success rate')
parser.add_argument(
    '--seed',
    type=int,
    default=None,
    help='base random seed to use for main proc & subprocs')
parser.add_argument(
    '-A',
    '--optimiser-opts',
    default={},
    type=opt_str,
    help='additional arguments for optimiser')
parser.add_argument(
    '--resume-from', default=None, help='snapshot pickle to resume from')
parser.add_argument(
    '-t',
    '--timeout',
    type=float,
    default=None,
    help='maximum training time (disabled by default)')
parser.add_argument(
    '-O',
    '--model-opts',
    type=opt_str,
    default={},
    help='options for model (e.g. p1=v1,p2=v2,p3=v3)')
parser.add_argument(
    '--no-skip',
    action='store_false',
    dest='skip',
    help='disable skip connections')
parser.add_argument(
    '--num-layers', type=int, default=2, help='number of layers for network')
parser.add_argument(
    '--hidden-size',
    type=int,
    default=16,
    help='hidden size of latent representation')
parser.add_argument(
    '--dropout',
    type=int_or_float,
    default=0.0,
    help='enable dropout during both learning & rollouts')
parser.add_argument(
    '--sup-objective',
    type=sup_objective_str,
    default=SupervisedObjective.ANY_GOOD_ACTION,
    help='objective for supervised training (choices: %s)' % ', '.join(
        [obj.name for obj in SupervisedObjective]))
parser.add_argument(
    '--no-use-teacher-envelope',
    dest='use_teacher_envelope',
    default=True,
    action='store_false',
    help='disable pulling entire envelope of teacher policy '
    'into experience buffer each time ASNet visits a state, '
    'and instead pull in just a single rollout under the '
    'teacher policy')
parser.add_argument(
    '--det-eval',
    action='store_true',
    default=False,
    help='use deterministic action selection for evaluation')
parser.add_argument(
    '-H',
    '--heuristic',
    type=str,
    default=None,
    help='SSiPP heuristic to give to ASNet')
parser.add_argument(
    '--minimal-file-saves',
    default=False,
    action='store_true',
    help="don't create TB files, final snapshot, or other extraneous "
    "(and expensive) run info")
parser.add_argument(
    '--no-use-lm-cuts',
    dest='use_lm_cuts',
    default=True,
    action='store_false',
    help="don't add flags indicating which actions are in lm-cut cuts")
parser.add_argument(
    '--use-act-history',
    default=False,
    action='store_true',
    help='add features for past execution count of each action')
parser.add_argument(
    '--save-training-set',
    default=None,
    help='save pickled training set to this file')
parser.add_argument(
    '--use-saved-training-set',
    default=None,
    help='instead of collecting experience, used this pickled training set '
    '(produced by --save-training-set)')
parser.add_argument(
    '-R', '--rounds-eval', type=int, default=100, help='number of eval rounds')
parser.add_argument(
    '-L', '--limit-turns', type=int, default=100, help='max turns per round')
parser.add_argument(
    '-e', '--expt-dir', default=None, help='path to store experiments in')
parser.add_argument(
    '--dK', default='dk', help='prefix of the domain knowledge file'
)
parser.add_argument(
    '--debug',
    default=False,
    action='store_true',
    help='enable tensorflow debugger')
parser.add_argument(
    '--no-train',
    default=False,
    action='store_true',
    help="don't train, just evaluate")
parser.add_argument(
    '--l1-reg', type=float, default=0.0, help='l1 regulariser weight')
parser.add_argument(
    '--target-rollouts-per-epoch',
    type=int,
    default=75,
    help='target number of ASNet rollouts to add to training set at each epoch'
)
parser.add_argument(
    # start with token regulariser to ensure opt problem is bounded below
    '--l2-reg',
    type=float,
    default=1e-5,
    help='l2 regulariser weight')
parser.add_argument(
    # this encourages equations to go to zero completely unless they're
    # actually needed (ideally use this in conjunction with a larger --l1-reg)
    '--l1-l2-reg',
    type=float,
    default=0.0,
    help='l1-l2 (group sparse) regulariser weight')
parser.add_argument(
    '--teacher-planner',
    choices=('ssipp', 'fd', 'domain-specific'),
    default='ssipp',
    help='choose between several different teacher planners')
parser.add_argument(
    '--opt-batch-per-epoch',
    default=1000,
    type=int,
    help='number of batches of optimisation per epoch')
parser.add_argument(
    '--net-debug',
    action='store_true',
    default=False,
    help='put in place additional assertions etc. to help debug network')
parser.add_argument(
    '--teacher-timeout-s',
    type=int,
    # default is small b/c anything less than "nearly instant" is going to take
    # a lot of cumulative time
    default=60,
    help='teacher timeout, in seconds (must be >0; default 10)')
parser.add_argument(
    '--plan-file-name',
    default='plan_sas',
    help="plan output file name"
)
parser.add_argument(
    '--limit-train-obs-size',
    default=99999,
    help="limit the problem size. If it is too big, skip the problem."
)
parser.add_argument(
    'pddls', nargs='+', help='paths to PDDL domain/problem definitions')


def eval_single(args, policy, problem_server, unique_prefix, elapsed_time,
                iter_num, weight_manager, scratch_dir):
    # now we evaluate the learned policy
    print('Evaluating policy')
    trial_results, paths = run_trials(
        policy,
        problem_server,
        args.rounds_eval,
        limit=args.limit_turns,
        det_sample=args.det_eval,
    )

    print('Trial results:')
    print('\n'.join('%s: %s' % (k, v) for k, v in trial_results.items()))

    out_dict = {
        'no_train': args.no_train,
        'args_problems': args.problems,
        'problem': to_local(problem_server.service.get_current_problem_name()),
        'timeout': args.timeout,
        'hidden_size': args.hidden_size,
        'num_layers': args.num_layers,
        'all_args': sys.argv[1:],
        # elapsed_* also includes time/iterations spent looking for better
        # results after converging
        'elapsed_opt_time': elapsed_time,
        'elapsed_opt_iters': iter_num,
        'trial_paths': paths
    }
    out_dict.update(trial_results)
    result_path = path.join(scratch_dir, 'results.json')
    with open(result_path, 'w') as fp:
        dump(out_dict, fp, indent=2)
    # also write out lists of actions taken during final trial
    actions_path = path.join(args.plan_file_name)
    for i, alist in enumerate(paths):
        if trial_results["all_goal_reached"][i]:
            with open(f'{actions_path}.{i}', 'w') as fp:
                fp.write('(')
                fp.write(')\n('.join(alist))
                fp.write(')')


@can_profile
def make_policy(args,
                obs_dim,
                act_dim,
                dom_meta,
                prob_meta,
                dg_extra_dim=None,
                weight_manager=None):
    # size of input and output
    obs_dim = int(obs_dim)
    act_dim = int(act_dim)

    # can make normal FC MLP or an action/proposition network
    hs = args.hidden_size
    num_layers = args.num_layers
    dropout = args.dropout
    print('hidden_size: %d, num_layers: %d, dropout: %f' % (hs, num_layers,
                                                            dropout))
    if weight_manager is not None:
        print('Re-using same weight manager')
    elif args.resume_from:
        print('Reloading weight manager (resuming training)')
        weight_manager = joblib.load(args.resume_from)
    else:
        print('Creating new weight manager (not resuming)')
        # TODO: should save all network metadata with the network weights or
        # within a separate config class, INCLUDING heuristic configuration
        weight_manager = PropNetworkWeights(
            dom_meta,
            hidden_sizes=[(hs, hs)] * num_layers,
            # extra inputs to each action module from data generators
            extra_dim=dg_extra_dim,
            skip=args.skip)
    custom_network = PropNetwork(
        weight_manager, prob_meta, dropout=dropout, debug=args.net_debug)

    # weight_manager will sometimes be None
    return custom_network, weight_manager


def get_problem_names(pddl_files):
    """Return a list of problem names from some PDDL files by spooling up
    background process."""
    config = ProblemServiceConfig(
        pddl_files, None, teacher_planner='ssipp', random_seed=None)
    server = ProblemServer(config)
    try:
        server.service.initialise()
        names = to_local(server.service.get_problem_names())
        assert isinstance(names, list)
        assert all(isinstance(name, str) for name in names)
    finally:
        server.stop()
    return names


class SingleProblem(object):
    """Wrapper to store all information relevant to training on a single
    problem."""

    def __init__(self, name, problem_server):
        self.name = name
        # need a handle to problem server so that it doesn't get GC'd (which
        # would kill the child process!)
        self.problem_server = problem_server
        self.problem_service = problem_server.service
        self.prob_meta, self.dom_meta = to_local(
            self.problem_service.get_meta())
        self.obs_dim = to_local(self.problem_service.get_obs_dim())
        self.act_dim = to_local(self.problem_service.get_act_dim())
        self.dg_extra_dim = to_local(self.problem_service.get_dg_extra_dim())
        # will get filled in later
        self.policy = None


@can_profile
def make_services(args):
    """Make a ProblemService for each relevant problem."""
    # first get names
    if not args.problems:
        print("No problem name given, will use all discovered problems")
        problem_names = get_problem_names(args.pddls)
    else:
        problem_names = args.problems
    print("Loading problems %s" % ', '.join(problem_names))

    # now get contexts for each problem and a manager for their weights
    servers = []

    def kill_servers():
        for server in servers:
            try:
                server.stop()
            except Exception as e:
                print("Got exception %r while trying to stop %r" % (e, server))

    atexit.register(kill_servers)

    only_one_good_action = args.sup_objective \
        == SupervisedObjective.THERE_CAN_ONLY_BE_ONE
    async_calls = []
    for prob_id, problem_name in enumerate(problem_names, start=1):
        random_seed = None if args.seed is None \
            else args.seed + prob_id
        service_config = ProblemServiceConfig(
            args.pddls,
            problem_name,
            random_seed=random_seed,
            heuristic_name=args.heuristic,
            teacher_heur=args.ssipp_teacher_heur,
            use_lm_cuts=args.use_lm_cuts,
            fd_heuristic_name=args.fd_teacher_heuristic,
            teacher_planner=args.teacher_planner,
            teacher_timeout_s=args.teacher_timeout_s,
            only_one_good_action=only_one_good_action,
            use_act_history=args.use_act_history,
            use_teacher_envelope=args.use_teacher_envelope)
        problem_server = ProblemServer(service_config)
        servers.append(problem_server)
        # must call initialise()
        init_method = rpyc.async_(problem_server.service.initialise)
        async_calls.append(init_method())

    # wait for initialise() calls to finish
    for async_call in async_calls:
        async_call.wait()
        # this property lookup is necessary to trigger any exceptions that
        # might have occurred during init (.wait() will not throw exceptions
        # from the child process; it only throws an exception on timeout)
        async_call.value

    # do this as a separate loop so that we can wait for services to spool
    # up in background
    problems = []
    weight_manager = None
    for problem_name, problem_server in zip(problem_names, servers):
        print('Starting service and policy for %s' % problem_name)
        problem = SingleProblem(problem_name, problem_server)
        print(f"Problem {problem_name} has obs_dim {problem.obs_dim}")
        if args.no_train or problem.obs_dim <= int(args.limit_train_obs_size):
            problem.policy, weight_manager = make_policy(
                args,
                problem.obs_dim,
                problem.act_dim,
                problem.dom_meta,
                problem.prob_meta,
                problem.dg_extra_dim,
                weight_manager=weight_manager)
            problems.append(problem)

    return problems, weight_manager


@can_profile
def main_supervised(args, unique_prefix, snapshot_dir, scratch_dir):
    print('Training supervised')

    start_time = time()
    problems, weight_manager = make_services(args)
    # layer = problems[0].policy
    # inputtf = tf.zeros([347, ])
    # outputtf = layer(inputtf[None])
    # print(outputtf)

    # print([var.name for var in layer.trainable_weights])

    # need to create FileWriter *after* creating the policy network itself, or
    # the network will not show up in TB (I assume that the `Graph` view is
    # just a snapshot of the global TF op graph at the time a given
    # `FileWriter` is instantiated)
    summary_path = path.join(scratch_dir, 'tensorboard')
    if args.minimal_file_saves:
        sample_writer = None
    else:
        sample_writer = tf.summary.create_file_writer(summary_path)

    if not args.no_train:
        print('Training supervised with strategy %r and heuristic %r' % \
                (args.sup_objective, args.fd_teacher_heuristic))
        sup_trainer = SupervisedTrainer(
            problems=problems,
            weight_manager=weight_manager,
            summary_writer=sample_writer,
            strategy=args.sup_objective,
            batch_size=args.supervised_bs,
            lr=args.supervised_lr,
            lr_steps=args.lr_steps,
            l1_reg_coeff=args.l1_reg,
            l2_reg_coeff=args.l2_reg,
            l1_l2_reg_coeff=args.l1_l2_reg,
            target_rollouts_per_epoch=args.target_rollouts_per_epoch,
            opt_batches_per_epoch=args.opt_batch_per_epoch,
            save_training_set=args.save_training_set,
            use_saved_training_set=args.use_saved_training_set,
            start_time=start_time,
            early_stop=args.supervised_early_stop,
            save_every=args.save_every,
            scratch_dir=scratch_dir,
            snapshot_dir=snapshot_dir,
            dk = args.dK,
        )
        best_rate, elapsed_time, iter_num = sup_trainer.train(
            max_epochs=args.max_opt_epochs)
    else:
        assert not args.dropout, \
            f"--no-train provided, but we have dropout of {args.dropout}?"
        # need to fill up stats values with garbage :P
        elapsed_time = iter_num = None
        # normally trainers do this
        # sess.run(tf.compat.v1.global_variables_initializer())

    # evaluate
    if weight_manager is not None and not args.minimal_file_saves:
        weight_manager.save(path.join(snapshot_dir, 'snapshot_final.pkl'))
    for problem in tqdm.tqdm(problems, desc='Evaluation'):
        print('Solving %s' % problem.name)
        eval_single(args, problem.policy, problem.problem_server,
                    unique_prefix + '-' + problem.name, elapsed_time,
                    iter_num, weight_manager, scratch_dir)


def main():
    rpyc.core.protocol.DEFAULT_CONFIG.update({
        # this is required for rpyc to allow pickling
        'allow_pickle': True,
        # "allow_all_attrs": True,
        "allow_public_attrs" : True,
        # required for some large problems where get_action() (passed as
        # synchronous callback to child processes) can take a very long time
        # the first time it is called
        'sync_request_timeout': 1800,
    })

    # ALWAYS die when parent dies; useful when running under run_experiment
    # etc. (this should never outlive run_experiment!)
    parent_death_pact(signal.SIGKILL)

    args = parser.parse_args()

    if args.seed is not None:
        set_random_seeds(args.seed)
    else:
        # here "defaults" probably just means seeding based on time (although
        # possibly each library might be a little different)
        print("No random seed provided; defaults will be used")

    unique_prefix = unique_name(args)
    print('Unique prefix:', unique_prefix)

    if args.minimal_file_saves:
        # --minimal-file-saves is mostly there to avoid writing out a
        # checkpoint & TB file for each evaluation run when doing *many*
        # evaluations, so it doesn't make much sense to specify it on training
        # runs, where checkpoints are always written anyway (they have to be!)
        assert args.no_train, \
            "--minimal-file-saves without --no-train is weird; is this a bug?"

    if args.expt_dir is None:
        args.expt_dir = 'experiment-results'
    scratch_dir = path.join(args.expt_dir, unique_prefix)
    makedirs(scratch_dir, exist_ok=True)

    # where to save models
    snapshot_dir = path.join(scratch_dir, 'snapshots')
    makedirs(snapshot_dir, exist_ok=True)
    print('Snapshot directory:', snapshot_dir)

    main_supervised(args, unique_prefix, snapshot_dir, scratch_dir)


def _main():
    global prof_utils

    # these will be useful for nefarious hacking when running under kernprof
    from asnets.utils import prof_utils
    prof_utils._run_asnets_globals = globals()

    # now run actual program
    main()


if __name__ == '__main__':
    _main()
