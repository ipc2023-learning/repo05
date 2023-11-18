#!/usr/bin/env python3
"""Run an experiment using the standard Python-based configuration format (see
`experiments/` subdirectory for example.)"""

import argparse
import datetime
from hashlib import md5
from importlib import import_module
import logging
from os import path, makedirs, listdir, getcwd
import os
from shutil import copytree
from subprocess import Popen, PIPE, TimeoutExpired
import sys
from time import time
from pathlib import Path
import shutil


_log = logging.getLogger(__name__)
THIS_DIR = path.dirname(path.abspath(__file__))
PLANNER_ROOT = path.abspath(path.join(THIS_DIR, '..', '..'))
# hack to ensure we can find 'experiments' module
sys.path.append(PLANNER_ROOT)


def extract_by_prefix(lines, prefix):
    for line in lines:
        if line.startswith(prefix):
            return line[len(prefix):]


def run_asnets_local(flags, root_dir, need_snapshot, timeout, is_train,
                     enforce_ncpus, cwd):
    """Run ASNets code on current node."""
    cmdline = []
    cmdline.extend(['python3', '-m', 'asnets.scripts.run_asnets'] + flags)
    _log.info('Running command line "%s"' % ' '.join(cmdline))

    # we use this for logging
    unique_suffix = md5(' '.join(cmdline).encode('utf8')).hexdigest()
    dest_dir = path.join(root_dir, 'runs', unique_suffix)
    _log.info('Will write results to %s' % dest_dir)
    makedirs(dest_dir, exist_ok=True)
    with open(path.join(dest_dir, 'cmdline'), 'w') as fp:
        fp.write(' '.join(cmdline))
    stdout_path = path.join(dest_dir, 'stdout')
    stderr_path = path.join(dest_dir, 'stderr')

    dfpg_proc = tee_out_proc = tee_err_proc = None
    start_time = time()
    try:
        print(cmdline)
        # print to stdout/stderr *and* save as well
        dfpg_proc = Popen(cmdline, stdout=PIPE, stderr=PIPE, cwd=cwd)
        # first tee for stdout
        tee_out_proc = Popen(['tee', stdout_path], stdin=dfpg_proc.stdout)
        # second tee for stderr
        tee_err_proc = Popen(['tee', stderr_path], stdin=dfpg_proc.stderr)

        # close descriptors from this proc (they confuse child error handling);
        # see https://stackoverflow.com/q/23074705
        dfpg_proc.stdout.close()
        dfpg_proc.stderr.close()

        # twiddle, twiddle, twiddle
        timed_out = False
        bad_retcode = False
        try:
            dfpg_proc.wait(timeout=timeout)
        except TimeoutExpired:
            # uh, oops; better kill everything
            _log.info('Run timed out after %ss!' % timeout)
            timed_out = True
    finally:
        # "cleanup"
        for proc in [tee_out_proc, tee_err_proc, dfpg_proc]:
            if proc is None:
                continue
            proc.poll()
            if proc.returncode is None:
                # make sure it's dead
                _log.info('Force-killing a process')
                proc.terminate()
            proc.wait()
            retcode = proc.returncode
            if retcode != 0:
                _log.info('Process exited with code %s: %s' %
                      (retcode, ' '.join(proc.args)))
                bad_retcode = True

    # write out extra info
    elapsed_time = time() - start_time
    with open(path.join(dest_dir, 'elapsed_secs'), 'w') as fp:
        fp.write('%f\n' % elapsed_time)
    with open(path.join(dest_dir, 'termination_status'), 'w') as fp:
        fp.write('timed_out: %s\nbad_retcode: %s\n' % (timed_out, bad_retcode))
    if is_train:
        with open(path.join(dest_dir, 'is_train'), 'w') as fp:
            # presence of 'is_train' file (in this case containing just a
            # newline) is sufficient to indicate that this was a train run
            fp.write("")

    # get stdout for... reasons
    with open(stdout_path, 'r') as fp:
        stdout = fp.read()
    lines = stdout.splitlines()

    # copy all info in custom dir into original prog's output dir (makes it
    # easier to associated)
    run_subdir = extract_by_prefix(lines, 'Unique prefix: ')
    if run_subdir is None:
        raise Exception("Couldn't find unique prefix for problem!")
    run_dir = path.join(root_dir, run_subdir)
    copytree(dest_dir, path.join(run_dir, 'run-info'))

    if need_snapshot:
        # parse output to figure out where it put the last checkpoint
        final_checkpoint_dir = extract_by_prefix(lines, 'Snapshot directory: ')
        if final_checkpoint_dir is None:
            msg = "cannot find final snapshot from stdout; check logs!"
            raise Exception(msg)
        # choose latest snapshot
        by_num = {}
        snaps = [
            path.join(final_checkpoint_dir, bn)
            for bn in listdir(final_checkpoint_dir)
            if bn.startswith('snapshot_')
        ]
        for snap in snaps:
            bn = path.basename(snap)
            num_s = bn.split('_')[1].rsplit('.', 1)[0]
            if num_s == 'final':
                # always choose this
                num = float('inf')
            else:
                num = int(num_s)
            by_num[num] = snap
        if len(by_num) == 0:
            msg = "could not find any snapshots in '%s'" % final_checkpoint_dir
            raise Exception(msg)
        # if this fails then we don't have any snapshots
        final_checkpoint_path = by_num[max(by_num.keys())]

        return final_checkpoint_path


def build_arch_flags(arch_mod, rollout, teacher_heuristic, is_train):
    """Build flags which control model arch and training strategy."""
    flags = []
    assert arch_mod.SUPERVISED, "only supervised training supported atm"
    if is_train:
        flags.extend(['--dropout', str(arch_mod.DROPOUT)])
    if not arch_mod.SKIP:
        flags.append('--no-skip')
    if arch_mod.DET_EVAL:
        flags.append('--det-eval')
    if not arch_mod.USE_LMCUT_FEATURES:
        flags.append('--no-use-lm-cuts')
    if arch_mod.USE_ACT_HISTORY_FEATURES:
        flags.append('--use-act-history')
    if arch_mod.TEACHER_EXPERIENCE_MODE == 'ROLLOUT':
        flags.append('--no-use-teacher-envelope')
    elif arch_mod.TEACHER_EXPERIENCE_MODE != 'ENVELOPE':
        raise ValueError(
            f"Unknown experience mode '{arch_mod.TEACHER_EXPERIENCE_MODE}'; "
            "try 'ROLLOUT' or 'ENVELOPE'")
    if arch_mod.L1_REG:
        assert isinstance(arch_mod.L1_REG, (float, int))
        l1_reg = str(arch_mod.L1_REG)
    else:
        l1_reg = '0.0'
    if arch_mod.L2_REG:
        assert isinstance(arch_mod.L2_REG, (float, int))
        l2_reg = str(arch_mod.L2_REG)
    else:
        l2_reg = '0.0'
    flags.extend([
        '--num-layers', str(arch_mod.NUM_LAYERS),
        '--hidden-size', str(arch_mod.HIDDEN_SIZE),
        '--target-rollouts-per-epoch', str(int(rollout)),
        '--l2-reg', l2_reg,
        '--l1-reg', l1_reg,
        '-R', str(arch_mod.EVAL_ROUNDS),
        '-L', str(arch_mod.ROUND_TURN_LIMIT),
        '-t', str(arch_mod.TIME_LIMIT_SECONDS),
        '--supervised-lr', str(arch_mod.SUPERVISED_LEARNING_RATE),
        '--supervised-bs', str(arch_mod.SUPERVISED_BATCH_SIZE),
        '--supervised-early-stop', str(arch_mod.SUPERVISED_EARLY_STOP),
        '--save-every', str(arch_mod.SAVE_EVERY_N_EPOCHS),
        '--ssipp-teacher-heur', arch_mod.SSIPP_TEACHER_HEURISTIC,
        '--opt-batch-per-epoch', str(arch_mod.OPT_BATCH_PER_EPOCH),
        '--teacher-planner', arch_mod.TEACHER_PLANNER,
        '--fd-teacher-heuristic', teacher_heuristic,
        '--sup-objective', arch_mod.TRAINING_STRATEGY,
        '--max-opt-epochs', str(arch_mod.MAX_OPT_EPOCHS),
        '--limit-train-obs-size', str(arch_mod.LIMIT_TRAIN_OBS_SIZE)
    ])  # yapf: disable
    if arch_mod.LEARNING_RATE_STEPS:
        for k, r in arch_mod.LEARNING_RATE_STEPS:
            assert k > 0, r > 0
            assert isinstance(k, int)
            assert isinstance(k, (int, float))
            flags.extend(['--lr-step', str(k), str(r)])
    return flags


def add_prefix(prefix, filenames):
    """Add a prefix directory to a bunch of filenames."""
    return [path.join(prefix, fn) for fn in filenames]


def build_prob_flags_train(prob_mod):
    """Build up some train flags for ASNets."""
    pddls = add_prefix(prob_mod.PDDL_DIR, prob_mod.COMMON_PDDLS)
    train_pddls = add_prefix(prob_mod.PDDL_DIR, prob_mod.TRAIN_PDDLS)
    pddls.extend(train_pddls)
    other_flags = []
    if prob_mod.TRAIN_NAMES:
        for tn in prob_mod.TRAIN_NAMES:
            other_flags.extend(['-p', tn])
    return other_flags + pddls


def build_prob_flags_test(prob_mod, allowed_idxs=None):
    """Build a list of flag sets, with one flag set for each requested
    experiment."""
    pddls = add_prefix(prob_mod.PDDL_DIR, prob_mod.COMMON_PDDLS)
    rv = []
    for idx, path_and_name in enumerate(prob_mod.TEST_RUNS):
        pddl_paths, prob_name = path_and_name
        if allowed_idxs is not None and idx not in allowed_idxs:
            _log.info('Will skip item %d: %s' % (idx, path_and_name))
            continue
        prob_flag = []
        if prob_name is not None:
            prob_flag = ['-p', prob_name]
        these_pddls = add_prefix(prob_mod.PDDL_DIR, pddl_paths)
        rv.append((idx, prob_flag + pddls + these_pddls))
    return rv


def get_prefix_dir(checkpoint_path):
    """Turn path like experiments-results/experiments.actprop_2l-.../.../... into
    experiment-results/experiments.actprop_2l.../"""
    real_path = path.abspath(checkpoint_path)
    parts = real_path.split(path.sep)
    for idx in range(len(parts) - 1)[::-1]:
        part = parts[idx]
        if part.startswith('experiments.'):
            return path.sep.join(parts[:idx + 1])
    raise ValueError("Couldn't find experiments. prefix in '%s'" %
                     checkpoint_path)


def parse_idx_list(idx_list):
    idx_strs = [int(s) for s in idx_list.split(',') if s.strip()]
    return idx_strs


parser = argparse.ArgumentParser(description='Run an experiment with ASNets')
parser.add_argument('--resume-from',
                    default=None,
                    help='resume experiment from given checkpoint path')
parser.add_argument(
    '--restrict-test-probs',
    default=None,
    type=parse_idx_list,
    help='takes comma-separated list of evaluation problem numbers to test')
parser.add_argument(
    '--job-ncpus',
    type=int,
    default=None,
    help='number of CPUs *per job* (must be <= --ray-ncpus; default is 1)')
parser.add_argument(
    '--enforce-job-ncpus',
    default=False,
    action='store_true',
    help='enforce --job-ncpus usage by using taskset/sched_setaffinity to '
    'pin jobs to unique cores')
parser.add_argument(
    '--ray-connect',
    default=None,
    help='connect Ray to this Redis DB instead of starting new cluster')
parser.add_argument(
    '--ray-ncpus',
    default=None,
    type=int,
    help='restrict Ray pool to use this many CPUs *in total* (only valid if '
    'spinning up new Ray cluster)')
parser.add_argument(
    'arch_module',
    metavar='arch-module',
    help='import path for Python file with architecture config (e.g. '
    '"experiments.actprop_1l")')
parser.add_argument(
    'domain_knowledge',
    help='DK')
parser.add_argument(
    'domain_file',
    help='DOMAIN')
parser.add_argument(
    'problems',
    nargs='+',
    help='TASK1, TASK2, ...')

def main():

    args = parser.parse_args()

    _log.info('logbook started')

    # 1. load config
    _log.info('Importing architecture from %s' % args.arch_module)
    arch_mod = import_module(args.arch_module)

    # 2. spool up Ray
    new_cluster = args.ray_connect is None
    ray_kwargs = {}
    if not new_cluster:
        ray_kwargs["redis_address"] = args.ray_connect
        assert args.ray_ncpus is None, \
            "can't provide --ray-ncpus and --ray-connect"
    else:
        if args.ray_ncpus is not None:
            assert args.job_ncpus is None \
                    or args.job_ncpus <= args.ray_ncpus, \
                    "must have --job-ncpus <= --ray-ncpus if both given"
            ray_kwargs["num_cpus"] = args.ray_ncpus

    main_inner(arch_mod=arch_mod,
               domain=args.domain_file,
               problems=args.problems,
               job_ncpus=args.job_ncpus,
               domain_knowledge_name=args.domain_knowledge,
               enforce_job_ncpus=args.enforce_job_ncpus)
    _log.info('Fin :-)')


def main_inner(*,
               arch_mod,
               domain,
               problems,
               job_ncpus,
               enforce_job_ncpus,
               domain_knowledge_name="DK"):
    root_cwd = getcwd()

    arch_name = arch_mod.__name__
    prob_name = Path(str(domain)).stem

    time_str = datetime.datetime.now().isoformat()
    prefix_dir = 'experiment-results/%s-%s-%s' % (prob_name, arch_name,
                                                    time_str)
    prefix_dir = path.join(root_cwd, prefix_dir)
    _log.info('Will put experiment results in %s' % prefix_dir)

    # 3. train network
    _log.info('\n\n\n\n\n\nTraining network')
    train_flags_base = [
        # log and snapshot dirs
        '-e', prefix_dir,
    ]  # yapf: disable
    trained_problems = [problems.pop(0)]
    trained_problems.append(problems.pop(0))
    unsolved_problems = []
    final_checkpoint = None
    tolerence = 0
    while problems:
        train_flags = []
        train_flags.extend(train_flags_base)
        train_flags.extend(build_arch_flags(arch_mod, rollout=arch_mod.TARGET_ROLLOUTS_PER_EPOCH, teacher_heuristic=arch_mod.FD_TEACHER_HEURISTIC, is_train=True))
        train_flags.extend(['--dK', domain_knowledge_name])
        if os.path.exists(domain_knowledge_name):
            train_flags.extend(['--resume-from', domain_knowledge_name])
        train_flags.append(domain)
        train_flags.extend(trained_problems)
        new_problem = problems.pop(0)
        train_flags.append(new_problem)
        trained_problems.append(new_problem)
        print(train_flags)
        try:
            final_checkpoint = run_asnets_local(
                    flags=train_flags,
                    # we make sure it runs cmd in same dir as us,
                    # because otherwise Ray subprocs freak out
                    cwd=root_cwd,
                    root_dir=prefix_dir,
                    need_snapshot=True,
                    is_train=True,
                    enforce_ncpus=enforce_job_ncpus,
                    timeout=arch_mod.TIME_LIMIT_SECONDS)
            _log.info('Last valid checkpoint is %s' % final_checkpoint)

            shutil.copy(final_checkpoint, domain_knowledge_name)
            # return the prefix_dir because hype.py needs that to figure out where to
            # point collate_results at
        except Exception as e:
            print(f"Something wrong: {e}")
            unsolved_problems.append(train_flags.pop())
            tolerence += 1
            if tolerence > 10:
                break
            else:
                continue
    
    # try to solve unsolved cases again with much more aggresive parameters
    still_not_ok = True
    rollout = arch_mod.TARGET_ROLLOUTS_PER_EPOCH
    heus = arch_mod.FD_TEACHER_HEURISTIC
    while still_not_ok:
        train_flags = []
        train_flags.extend(train_flags_base)
        rollout = round(rollout * 0.9)
        train_flags.extend(build_arch_flags(arch_mod, rollout=rollout, teacher_heuristic=heus, is_train=True))
        train_flags.extend(['--teacher-timeout-s', "120"])
        train_flags.extend(['--dK', domain_knowledge_name])
        if os.path.exists(domain_knowledge_name):
            train_flags.extend(['--resume-from', domain_knowledge_name])
        train_flags.append(domain)
        train_flags.extend(trained_problems)
        train_flags.extend(unsolved_problems)
        print(train_flags)
        try:
            final_checkpoint = run_asnets_local(
                    flags=train_flags,
                    # we make sure it runs cmd in same dir as us,
                    # because otherwise Ray subprocs freak out
                    cwd=root_cwd,
                    root_dir=prefix_dir,
                    need_snapshot=True,
                    is_train=True,
                    enforce_ncpus=enforce_job_ncpus,
                    timeout=arch_mod.TIME_LIMIT_SECONDS)
            _log.info('Last valid checkpoint is %s' % final_checkpoint)

            shutil.copy(final_checkpoint, domain_knowledge_name)
            # return the prefix_dir because hype.py needs that to figure out where to
            # point collate_results at
            still_not_ok = False
        except Exception as e:
            print(f"Something wrong: {e}")
            tolerence += 1
            if tolerence > 20:
                break
            else:
                continue


    return prefix_dir


if __name__ == '__main__':
    main()
