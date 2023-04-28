import pytest
import mdpsim as m
import os

parsed = False


def parse():
    global parsed
    if not parsed:
        my_path = os.path.dirname(os.path.abspath(__file__))
        tt_path = os.path.join(my_path, '..', 'examples', 'triangle-tire.pddl')
        m.parse_file(tt_path)
        parsed = True


@pytest.fixture
def tt_domain():
    parse()
    return m.get_domains()['triangle-tire']


@pytest.fixture
def tt2_problem():
    parse()
    return m.get_problems()['triangle-tire-2']


def test_domain(tt_domain):
    assert repr(tt_domain) == 'Domain(<triangle-tire>)'

    # type name access
    types = tt_domain.types
    assert set(types) == {'location'}

    # domain access
    real_names = {'vehicle-at', 'spare-in', 'road', 'not-flattire'}
    preds = tt_domain.predicates
    assert len(preds) == len(real_names)
    by_name = {pred.name: pred for pred in preds}
    assert by_name.keys() == real_names
    road = by_name['road']
    assert road.arg_types == ['location', 'location']
    not_flattire = by_name['not-flattire']
    assert not_flattire.arg_types == []

    # lifted action access
    all_lifted = tt_domain.lifted_actions
    lifted_names = {"move-car", "changetire"}
    assert len(all_lifted) == len(lifted_names)
    lift_dict = {l.name: l for l in all_lifted}
    assert lift_dict.keys() == lifted_names
    lifted_move = lift_dict['move-car']
    # params are "from"/"to" (both locations)
    params_types = lifted_move.parameters_and_types
    assert len(params_types) == 2
    types = [t for _, t in params_types]
    for t in types:
        assert t == 'location'
    v1, v2 = params_types[0][0], params_types[1][0]
    assert v1 == v1 and hash(v1) == hash(v1)
    assert v1 != v2 and hash(v1) != hash(v2)
    assert v2 == v2 and hash(v2) == hash(v2)
    assert 'Variable' in repr(v1)
    # involved predicates
    # TODO: there's actually a bug here; some of the vehicle-at atoms get
    # double-counted despite being identical! Need to actually associate
    # objects with where they appear in the domain spec :/
    move_pred_names_expt = ['vehicle-at', 'road', 'not-flattire', 'vehicle-at',
                            'vehicle-at', 'not-flattire']
    move_pred_names = [
        p.predicate.name for p in lifted_move.involved_propositions
    ]
    assert move_pred_names == move_pred_names_expt


def test_problem(tt2_problem):
    assert repr(tt2_problem) == 'Problem(<triangle-tire-2>)'
    assert tt2_problem.name == 'triangle-tire-2'
    assert tt2_problem.domain.name == 'triangle-tire'

    # TODO: add individual tests for propositions and ground actions (actually
    # check that their implementations work)
    props = tt2_problem.propositions
    assert len(props) == 49

    acts = tt2_problem.ground_actions
    assert len(acts) == 33

    # check that 'involved actions' thing works
    action = None
    for ground_act in tt2_problem.ground_actions:
        if ground_act.lifted_action.name == 'move-car':
            action = ground_act
    assert action is not None, "could not find a move-car action"

    # make sure that road, vehicle-at and not-flattire propositions are all
    # relevant
    involved_preds = {
        prop.predicate.name for prop in action.involved_propositions
    }
    assert 'road' in involved_preds
    assert 'vehicle-at' in involved_preds
    assert 'not-flattire' in involved_preds


def test_state_crap(tt2_problem):
    # got these by running it myself :)
    num_actions = 33
    num_props = 49
    assert tt2_problem.num_actions == num_actions
    assert tt2_problem.num_props == num_props

    state = tt2_problem.init_state()
    prop_mask = tt2_problem.prop_truth_mask(state)
    prop_mask_strs = {(p.identifier, t) for p, t in prop_mask}
    assert ('(not-flattire)', True) in prop_mask_strs

    # now try stepping
    act_d = {
        a.identifier: a for a in tt2_problem.ground_actions
    }
    good_act = act_d['(move-car l-1-1 l-1-2)']
    assert isinstance(good_act, m.GroundAction)
    assert tt2_problem.applicable(state, good_act)
    new_state = tt2_problem.apply(state, good_act)
    assert isinstance(new_state, m.State)
    new_prop_mask = tt2_problem.prop_truth_mask(new_state)
    new_prop_mask_strs = {(p.identifier, t) for p, t in new_prop_mask}
    assert ('(vehicle-at l-1-2)', True) in new_prop_mask_strs

    bad_act = act_d['(move-car l-3-1 l-3-2)']
    assert not tt2_problem.applicable(state, bad_act)
    try:
        tt2_problem.apply(state, bad_act)
        assert False, 'should have got ValueError'
    except ValueError:
        # all good
        pass
