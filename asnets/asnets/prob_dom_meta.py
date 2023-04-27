"""Stores metadata for problems and domains in a pure-Python format. This
information is theoretically all obtainable from MDPSim extension. However,
I've introduced these extra abstractions so that I can pickle that information
and pass it between processes. C++ extension data structures (including those
from the MDPSim extension) can't be easily pickled, so passing around
information taken *straight* from the extension would not work."""

from functools import lru_cache, total_ordering
from typing import Any, Dict, Iterable, List, Set, Tuple
from typing_extensions import Self


@total_ordering
class BoundProp:
    """Represents a ground proposition."""

    def __init__(self, pred_name: str, arguments: Iterable[str]):
        """Create a new bound proposition.

        Args:
            pred_name (str): The name of the predicate.
            arguments (Iterable[str]): The arguments of the predicate.
        """
        self.pred_name = pred_name
        self.arguments = tuple(arguments)
        self.unique_ident = self._compute_unique_ident()

        assert isinstance(self.pred_name, str)
        assert all(isinstance(p, str) for p in self.arguments)

    def __repr__(self) -> str:
        """Return a string representation of this proposition.

        Returns:
            str: The string representation.
        """
        return 'BoundProp(%r, %r)' % (self.pred_name, self.arguments)

    def _compute_unique_ident(self) -> str:
        """Compute a unique identifier for this proposition. Will match SSiPP-
        style names (think "foo bar baz" rather than sexpr-style "(foo bar
        baz)"). This is used for hashing and comparisons.

        Returns:
            str: The unique identifier.
        """
        unique_id = ' '.join((self.pred_name, ) + self.arguments)
        return unique_id

    def __eq__(self, other: Self) -> bool:
        """Check if this proposition is equal to another.

        Args:
            other (Self): The other proposition.

        Returns:
            bool: True if this proposition is equal to the other, False
            otherwise.
        """
        if not isinstance(other, BoundProp):
            return NotImplemented
        return self.unique_ident == other.unique_ident

    def __lt__(self, other: Self) -> bool:
        """Check if this proposition is less than another.

        Args:
            other (Self): The other proposition.

        Returns:
            bool: True if this proposition is less than the other, False
            otherwise.
        """
        if not isinstance(other, BoundProp):
            return NotImplemented
        return self.unique_ident < other.unique_ident

    def __hash__(self) -> int:
        """Return a hash of this proposition.

        Returns:
            int: The hash.
        """
        return hash(self.unique_ident)


class UnboundProp:
    """Represents a proposition which may have free parameters (e.g. as it will
    in an action). .bind() will ground it."""

    def __init__(self, pred_name: str, params: Iterable[str]):
        """Create a new unbound proposition.

        Args:
            pred_name (str): The name of the predicate.
            params (Iterable[str]): The parameters of the predicate.
        """
        # TODO: what if some parameters are already bound? This might happen
        # when you have constants, for instance. Maybe cross that bridge when I
        # get to it.
        self.pred_name = pred_name
        self.params = tuple(params)

        assert isinstance(self.pred_name, str)
        assert all(isinstance(p, str) for p in self.params)

    def __repr__(self):
        return 'UnboundProp(%r, %r)' % (self.pred_name, self.params)

    def bind(self, bindings: Dict[str, str]) -> BoundProp:
        """Bind this proposition to a set of bindings.

        Args:
            bindings (Dict[str, str]): A mapping from parameter names to
            their values.

        Raises:
            ValueError: If a parameter is not bound.

        Returns:
            BoundProp: The bound proposition.
        """
        assert isinstance(bindings, dict), \
            "expected dict of named bindings"
        args = []
        for param_name in self.params:
            if param_name[0] != '?':
                # already bound to constant
                arg = param_name
            else:
                if param_name not in bindings:
                    raise ValueError(
                        "needed bind for parameter %s, didn't get one" %
                        param_name)
                arg = bindings[param_name]
            args.append(arg)
        return BoundProp(self.pred_name, args)

    def __eq__(self, other: Self) -> bool:
        """Check if this proposition is equal to another.

        Args:
            other (Self): The other proposition.

        Returns:
            bool: True if the propositions are equal, False otherwise.
        """
        if not isinstance(other, UnboundProp):
            return NotImplemented
        return self.pred_name == other.pred_name \
            and self.params == other.params

    def __hash__(self) -> int:
        """Return a hash of this proposition.

        Returns:
            int: a hash of this proposition.
        """
        return hash(self.pred_name) ^ hash(self.params)


@total_ordering
class BoundAction:
    """Represents a ground action."""

    def __init__(self, prototype: 'UnboundAction', arguments: Iterable[str],
                 props: Iterable[BoundProp]):
        """Create a new ground action.

        Args:
            prototype (UnboundAction): The unbound action that this is a
            ground instance of.
            arguments (Iterable[str]): The arguments to this action.
            props (Iterable[BoundProp]): The ground propositions that is
            relevant to this action.
        """
        self.prototype = prototype
        self.arguments = tuple(arguments)
        self.props = tuple(props)
        self.unique_ident = self._compute_unique_ident()

        assert isinstance(prototype, UnboundAction)
        assert all(isinstance(a, str) for a in self.arguments)
        assert all(isinstance(p, BoundProp) for p in self.props)

    def __repr__(self) -> bool:
        """Return a string representation of this action.

        Returns:
            bool: a string representation of this action.
        """
        return 'BoundAction(%r, %r, %r)' \
            % (self.prototype, self.arguments, self.props)

    def __str__(self) -> str:
        """Return a string representation of this action.

        Returns:
            str: a string representation of this action.
        """
        return 'Action %s(%s)' % (self.prototype.schema_name, ', '.join(
            self.arguments))

    def _compute_unique_ident(self) -> str:
        """Compute a unique identifier for this action.

        Returns:
            str: a unique identifier for this action.
        """
        unique_id = ' '.join((self.prototype.schema_name, ) + self.arguments)
        return unique_id

    def __eq__(self, other: Self) -> bool:
        """Compare this action to another action.

        Args:
            other (Self): the other action to compare to.

        Returns:
            bool: True if this action is equal to the other action, False
            otherwise.
        """
        if not isinstance(other, BoundAction):
            return NotImplemented
        return self.unique_ident == other.unique_ident

    def __lt__(self, other: Self) -> bool:
        """Compare this action to another action.

        Args:
            other (Self): the other action to compare to.

        Returns:
            bool: True if this action is less than the other action, False
            otherwise.
        """
        if not isinstance(other, BoundAction):
            return NotImplemented
        return self.unique_ident < other.unique_ident

    def __hash__(self) -> int:
        """Return a hash of this action.

        Returns:
            int: a hash of this action.
        """
        return hash(self.unique_ident)

    def num_slots(self) -> int:
        """Return the number of slots this action takes up in the state.

        Returns:
            int: the number of slots this action takes up in the state.
        """
        return len(self.props)


@total_ordering
class UnboundAction:
    """Represents an action that *may* be lifted. Use .bind() with an argument
    list to ground it."""

    def __init__(self, schema_name: str, param_names: Iterable[str],
                 rel_props: Iterable[UnboundProp]):
        """Create a new UnboundAction.

        Args:
            schema_name (str): The schema name of this action.
            param_names (Iterable[str]): The names of the parameters of this
            action.
            rel_props (Iterable[UnboundProp]): The propositions that are
            relavent to this action.
        """
        self.schema_name = schema_name
        self.param_names = tuple(param_names)
        self.rel_props = tuple(rel_props)

        assert isinstance(schema_name, str)
        assert all(isinstance(a, str) for a in self.param_names)
        assert all(isinstance(p, UnboundProp) for p in self.rel_props)

    def __repr__(self) -> str:
        """Return a string representation of this action.

        Returns:
            str: a string representation of this action.
        """
        return 'UnboundAction(%r, %r, %r)' \
            % (self.schema_name, self.param_names, self.rel_props)

    def _ident_tup(self) -> Tuple[str, Tuple[str], Tuple[UnboundProp]]:
        """Return a tuple that uniquely identifies this action.

        Returns:
            Tuple[str, Tuple[str], Tuple[UnboundProp]]: a tuple that uniquely
            identifies this action.
        """
        return (self.schema_name, self.param_names, self.rel_props)

    def __eq__(self, other: Self) -> bool:
        """Return whether two UnboundAction instances are equal.

        Args:
            other (Self): the action to compare to.

        Returns:
            bool: whether self is equal to other.
        """
        if not isinstance(other, UnboundAction):
            return NotImplemented
        return self._ident_tup() == other._ident_tup()

    def __lt__(self, other: Self) -> bool:
        """Return whether this action is less than another action.

        Args:
            other (Self): the other action to compare to.

        Returns:
            bool: whether this action is less than the other action.
        """
        if not isinstance(other, UnboundAction):
            return NotImplemented
        # avoid using self.rel_props because that would need ordering on
        # UnboundProp instances
        return (self.schema_name, self.param_names) \
            < (other.schema_name, other.param_names)

    def __hash__(self) -> int:
        """Return the hash of the action.

        Returns:
            int: the hash of the action.
        """
        return hash(self._ident_tup())

    def bind(self, arguments: List[str] | str) -> BoundAction:
        """Bind the action to a list of arguments.

        Args:
            arguments (List[str] | str): the arguments to bind.

        Raises:
            TypeError: if arguments is not a list of str.

        Returns:
            BoundAction: the bound action.
        """
        if not isinstance(arguments, (list, str)):
            raise TypeError('expected args to be list or str')
        bindings = dict(zip(self.param_names, arguments))
        props = [prop.bind(bindings) for prop in self.rel_props]
        return BoundAction(self, arguments, props)

    def num_slots(self) -> int:
        """Return the number of slots in the action.

        Returns:
            int: the number of slots in the action.
        """
        return len(self.rel_props)


class DomainMeta:
    """Represents the meta-information of a domain.
    """

    def __init__(self, name: str, unbound_acts: Iterable[UnboundAction],
                 pred_names: Iterable[str]):
        """Create a new DomainMeta.

        Args:
            name (str): name of the domain.
            unbound_acts (Iterable[UnboundAction]): unbound actions in the
            domain.
            pred_names (Iterable[str]): names of predicates in the domain.
        """
        self.name = name
        self.unbound_acts: Tuple[UnboundAction] = tuple(unbound_acts)
        self.pred_names = tuple(pred_names)

    def __repr__(self) -> str:
        """Return a string representation of this domain.

        Returns:
            str: a string representation of this domain.
        """
        return 'DomainMeta(%s, %s, %s)' \
            % (self.name, self.unbound_acts, self.pred_names)

    @lru_cache(None)
    def rel_act_slots(self, predicate_name: str) \
            -> List[Tuple[UnboundAction, int]]:
        """Map predicate name to names of relevant action schemas (without 
        duplicates).

        Args:
            predicate_name (str): name of predicate.

        Returns:
            List[Tuple[UnboundAction, int]]: list of tuples of unbound action 
            and slot number.
        """
        assert isinstance(predicate_name, str)
        rv: List[Tuple[UnboundAction, int]] = []
        for ub_act in self.unbound_acts:
            act_rps = self.rel_pred_names(ub_act)
            for slot, other_predicate_name in enumerate(act_rps):
                if predicate_name != other_predicate_name:
                    continue
                rv.append((ub_act, slot))
            # FIXME: maybe the "slots" shouldn't be integers, but rather tuples
            # of names representing parameters of the predicate like in
            # commented code below? Could then make those names consistent with
            # naming of the unbound action's parameters.
            # for ub_prop in ub_act.rel_props:
            #     if ub_prop.pred_name != predicate_name:
            #         continue
            #     slot_ident = ub_prop.params
            #     rv.append((ub_act, slot_ident))
        return rv

    @lru_cache(None)
    def rel_pred_names(self, action: UnboundAction) -> List[str]:
        """Return the names of the predicates relevant to the action.

        Args:
            action (UnboundAction): the action.

        Returns:
            List[str]: the names of the predicates relevant to the action.
        """
        assert isinstance(action, UnboundAction)
        rv = []
        for unbound_prop in action.rel_props:
            # it's important that we include duplicates here!
            rv.append(unbound_prop.pred_name)
        return rv

    @property
    @lru_cache(None)
    def all_unbound_props(self) \
            -> Tuple[List[UnboundProp], Dict[str, UnboundProp]]:
        """Return all unbound props in the domain.

        Returns:
            Tuple[List[UnboundProp], Dict[str, UnboundProp]]: the list of all
            unbound props and a dictionary mapping predicate names to unbound
            props.
        """
        unbound_props: List[UnboundProp] = []
        ub_prop_set: Set[UnboundProp] = set()
        ub_prop_dict: Dict[str, UnboundProp] = {}
        for unbound_act in self.unbound_acts:
            for ub_prop in unbound_act.rel_props:
                if ub_prop not in ub_prop_set:
                    unbound_props.append(ub_prop)
                    ub_prop_dict[ub_prop.pred_name] = ub_prop
                    # the set is just to stop double-counting
                    ub_prop_set.add(ub_prop)
        return unbound_props, ub_prop_dict

    def unbound_prop_by_name(self, predicate_name: str) -> UnboundProp:
        """Return the unbound prop with the given name.

        Args:
            predicate_name (str): the name of the predicate.

        Returns:
            UnboundProp: the unbound prop with the given name.
        """
        _, ub_prop_dict = self.all_unbound_props
        return ub_prop_dict[predicate_name]

    def _ident_tup(self) -> Tuple[str, Tuple[UnboundAction], Tuple[str]]:
        """Return a tuple that uniquely identifies this domain.

        Returns:
            Tuple[str, Tuple[UnboundAction], Tuple[str]]: a tuple that
            uniquely identifies this domain.
        """
        return (self.name, self.unbound_acts, self.pred_names)

    def __eq__(self, other: Self) -> bool:
        """Return whether this domain is equal to another.

        Args:
            other (Self): the other domain.

        Returns:
            bool: whether this domain is equal to another.
        """
        if not isinstance(other, DomainMeta):
            return NotImplemented
        return self._ident_tup() == other._ident_tup()

    def __hash__(self) -> int:
        """Return a hash of this domain.

        Returns:
            int: a hash of this domain.
        """
        return hash(self._ident_tup())


class ProblemMeta:
    # Some notes on members/properties
    # - rel_props is dict mapping ground action name => [relevant prop names]
    # - rel_acts is dict mapping prop name => [relevant ground action name]

    def __init__(self, name: str, domain: DomainMeta,
                 bound_acts_ordered: Iterable[BoundAction],
                 bound_props_ordered: Iterable[BoundProp],
                 goal_props: Iterable[BoundProp]):
        """Initialize a problem meta.

        Args:
            name (str): Name of the problem.
            domain (DomainMeta): Domain of the problem.
            bound_acts_ordered (Iterable[BoundAction]): The bound actions in
            the problem, in lexical order.
            bound_props_ordered (Iterable[BoundProp]): The bound propositions
            in the problem, in lexical order.
            goal_props (Iterable[BoundProp]): The goal propositions in the
            problem.
        """
        self.name = name
        self.domain = domain
        self.bound_acts_ordered = tuple(bound_acts_ordered)
        self.bound_props_ordered = tuple(bound_props_ordered)
        self.goal_props = tuple(goal_props)

        self._unique_id_to_index: Dict[str, int] = {
            bound_act.unique_ident: idx
            for idx, bound_act in enumerate(self.bound_acts_ordered)
        }

        # sanity checks
        assert set(self.goal_props) <= set(self.bound_props_ordered)

    def __repr__(self) -> str:
        """Return a string representation of this problem meta.

        Returns:
            str: a string representation of this problem meta.
        """
        return 'ProblemMeta(%s, %s, %s, %s, %s)' \
            % (self.name, self.domain, self.bound_acts_ordered,
               self.bound_props_ordered, self.goal_props)

    @property
    def num_props(self) -> int:
        """Return the number of propositions in the problem.

        Returns:
            int: the number of propositions in the problem.
        """
        return len(self.bound_props_ordered)

    @property
    def num_acts(self) -> int:
        """Return the number of actions in the problem.

        Returns:
            int: the number of actions in the problem.
        """
        return len(self.bound_acts_ordered)

    @lru_cache(None)
    def schema_to_acts(self, unbound_action: UnboundAction) \
            -> List[BoundAction]:
        """Return the bound actions that are instances of the given unbound
        action.

        Returns:
            List[BoundAction]: the bound actions that are instances of the
            unbound action.
        """
        assert isinstance(unbound_action, UnboundAction)
        return [
            a for a in self.bound_acts_ordered if a.prototype == unbound_action
        ]

    @lru_cache(None)
    def pred_to_props(self, pred_name: str) -> List[BoundProp]:
        """Return the bound propositions that have the given predicate name.

        Args:
            pred_name (str): the predicate name.

        Returns:
            List[BoundProp]: the bound propositions that have the given
            predicate name.
        """
        assert isinstance(pred_name, str)
        return [
            p for p in self.bound_props_ordered if p.pred_name == pred_name
        ]

    def prop_to_pred(self, bound_prop: BoundProp) -> str:
        """Return the predicate name of the given bound proposition.

        Args:
            bound_prop (BoundProp): the bound proposition.

        Returns:
            str: the predicate name of the given bound proposition.
        """
        assert isinstance(bound_prop, BoundProp)
        return bound_prop.pred_name

    def act_to_schema(self, bound_act: BoundAction) -> UnboundAction:
        """Return the unbound action that is the schema of the given bound
        action.

        Args:
            bound_act (BoundAction): the bound action.

        Returns:
            UnboundAction: the unbound action that is the schema of the given
            bound action.
        """
        assert isinstance(bound_act, BoundAction)
        return bound_act.prototype

    def rel_props(self, bound_act: BoundAction) -> List[BoundProp]:
        """Return the relevant bound propositions for the given bound action.

        Args:
            bound_act (BoundAction): the bound action.

        Returns:
            List[BoundProp]: the relevant bound propositions for the given
            bound action.
        """
        assert isinstance(bound_act, BoundAction)
        # no need for special grouping like in rel_acts, since all props can be
        # concatenated before passing them in
        return bound_act.props

    @lru_cache(None)
    def rel_act_slots(self, bound_prop: BoundProp) \
            -> List[Tuple[UnboundAction, int, List[BoundAction]]]:
        """Return the relevant actions and slots for the given bound 
        proposition.

        Args:
            bound_prop (BoundProp): the bound proposition.

        Returns:
            List[Tuple[UnboundAction, int, List[BoundAction]]]: the relevant
            unbound actions, slots, and bound actions for the given bound
            proposition.
        """
        assert isinstance(bound_prop, BoundProp)
        rv = []
        pred_name = self.prop_to_pred(bound_prop)
        for unbound_act, slot in self.domain.rel_act_slots(pred_name):
            bound_acts_for_schema = []
            for bound_act in self.schema_to_acts(unbound_act):
                if bound_prop == self.rel_props(bound_act)[slot]:
                    # TODO: is this the best way to do this? See comment in
                    # DomainMeta.rel_acts.
                    bound_acts_for_schema.append(bound_act)
            rv.append((unbound_act, slot, bound_acts_for_schema))
        # list of tuples, each of the form (unbound action, slot, list of
        # ground actions)
        return rv

    @lru_cache(None)
    def prop_to_pred_subtensor_ind(self, bound_prop: BoundProp) -> int:
        """Return the index of the given bound proposition in the tensor of
        propositions for the given predicate name.

        Args:
            bound_prop (BoundProp): the bound proposition.

        Returns:
            int: the index of the given bound proposition in the tensor of
            propositions for the given predicate name.
        """
        assert isinstance(bound_prop, BoundProp)
        pred_name = self.prop_to_pred(bound_prop)
        prop_vec = self.pred_to_props(pred_name)
        return prop_vec.index(bound_prop)

    @lru_cache(None)
    def act_to_schema_subtensor_ind(self, bound_act: BoundAction) -> int:
        """Return the index of the given bound action in the tensor of actions
        for the given unbound action.

        Args:
            bound_act (BoundAction): the bound action.

        Returns:
            int: the index of the given bound action in the tensor of actions
            for the given unbound action.
        """
        assert isinstance(bound_act, BoundAction)
        unbound_act = self.act_to_schema(bound_act)
        schema_vec = self.schema_to_acts(unbound_act)
        return schema_vec.index(bound_act)

    @lru_cache(None)
    def _props_by_name(self) -> Dict[str, BoundProp]:
        """Return a dictionary mapping the unique identifier of each bound
        proposition to the bound proposition.

        Returns:
            Dict[str, BoundProp]: a dictionary mapping the unique identifier of
            each bound proposition to the bound proposition.
        """
        all_props = {
            prop.unique_ident: prop
            for prop in self.bound_props_ordered
        }
        return all_props

    @lru_cache(None)
    def bound_prop_by_name(self, string: str) -> BoundProp:
        """Return the bound proposition with the given unique identifier.

        Args:
            strin (str): the unique identifier of the bound proposition.

        Returns:
            BoundProp: the bound proposition with the given unique identifier.
        """
        all_props = self._props_by_name()
        return all_props[string]

    def act_unique_id_to_index(self, string: str) -> int:
        """Return the index of the given bound action in the list of all
        bound actions.

        Args:
            string (str): the unique identifier of the bound action.

        Returns:
            int: the index of the given bound action in the list of all bound
            actions.
        """
        return self._unique_id_to_index[string]


def make_unbound_prop(mdpsim_lifted_prop: Any) -> UnboundProp:
    """Make an UnboundProp from an MDPsim PyProposition. Does not check that the
    PyProposition is actually lifted and allows for some terms to be bound, such
    as in the case of constants.

    Args:
        mdpsim_lifted_prop (Any): A PyProposition from MDPsim.

    Returns:
        UnboundProp: The corresponding UnboundProp.
    """
    pred_name: str = mdpsim_lifted_prop.predicate.name
    terms: List[str] = [t.name for t in mdpsim_lifted_prop.terms]
    return UnboundProp(pred_name, terms)


def make_bound_prop(mdpsim_ground_prop: Any) -> BoundProp:
    """Make a BoundProp from an MDPsim PyProposition. Verifies that all terms
    are actually bound.

    Args:
        mdpsim_ground_prop (Any): A PyProposition from MDPsim.

    Returns:
        BoundProp: The corresponding BoundProp.
    """
    pred_name: str = mdpsim_ground_prop.predicate.name
    arguments: List[str] = []
    for term in mdpsim_ground_prop.terms:
        term_name = term.name
        arguments.append(term_name)
        # make sure it's really a binding
        assert not term.name.startswith('?'), \
            "term '%s' starts with '?'---sure it's not free?"
    bound_prop = BoundProp(pred_name, arguments)
    return bound_prop


def make_unbound_action(mdpsim_lifted_act: Any) -> UnboundAction:
    """Make an UnboundAction from an mdpsim PyLiftedAction.

    Args:
        mdpsim_lifted_act (Any): A PyLiftedAction object from MDPSim.

    Returns:
        UnboundAction: The corresponding UnboundAction.
    """
    schema_name: str = mdpsim_lifted_act.name
    param_names: List[str] = [
        param.name for param, _ in mdpsim_lifted_act.parameters_and_types
    ]
    rel_props = []
    rel_prop_set = set()
    for mdpsim_prop in mdpsim_lifted_act.involved_propositions:
        unbound_prop = make_unbound_prop(mdpsim_prop)
        if unbound_prop not in rel_prop_set:
            # ignore duplicates
            rel_prop_set.add(unbound_prop)
            rel_props.append(unbound_prop)
    return UnboundAction(schema_name, param_names, rel_props)


def make_bound_action(mdpsim_ground_act: Any) -> BoundAction:
    """Make a BoundAction from a PyGroundAction object.

    Args:
        mdpsim_ground_act (Any): A PyGroundAction object from MDPSim.

    Returns:
        BoundAction: The corresponding BoundAction object.
    """
    lifted_act = make_unbound_action(mdpsim_ground_act.lifted_action)
    arguments = [arg.name for arg in mdpsim_ground_act.arguments]
    return lifted_act.bind(arguments)


def get_domain_meta(domain: Any) -> DomainMeta:
    """Extracts a nice, Pickle-able subset of the information contained in a
    domain so that we can construct the appropriate network weights.

    Args:
        domain (Any): A PyDomain object from MDPSim.

    Returns:
        DomainMeta: DomainMeta information contained in the PyDomain object.
    """
    pred_names: List[str] = [p.name for p in domain.predicates]
    unbound_acts = map(make_unbound_action, domain.lifted_actions)
    return DomainMeta(domain.name, unbound_acts, pred_names)


def get_problem_meta(problem: Any, domain_meta: DomainMeta) -> ProblemMeta:
    """Extracts a nice, Pickle-able subset of the information contained in a
    problem so that we can construct the appropriate network weights.

    Args:
        problem (Any): A PyProblem object from MDPSim.
        domain_meta (DomainMeta): The DomainMeta object corresponding to the
        domain of the problem.

    Returns:
        ProblemMeta: ProblemMeta information contained in the PyProblem object.
    """
    # we get given the real domain, but we also do a double-check to make sure
    # that it matches our problem
    other_domain = get_domain_meta(problem.domain)
    assert other_domain == domain_meta, \
        "%r\n!=\n%r" % (other_domain, domain_meta)

    # use network input orders implied by problem.propositions and
    # problem.ground_actions
    bound_props_ordered: List[BoundProp] = []
    goal_props: List[BoundProp] = []
    for mdpsim_prop in problem.propositions:
        bound_prop = make_bound_prop(mdpsim_prop)
        bound_props_ordered.append(bound_prop)
        if mdpsim_prop.in_goal:
            goal_props.append(bound_prop)
    # must sort these!
    bound_props_ordered.sort()

    prop_set: Set[BoundProp] = set(bound_props_ordered)
    ub_act_set: Set[UnboundAction] = set(domain_meta.unbound_acts)
    bound_acts_ordered: List[BoundAction] = []
    for mdpsim_act in problem.ground_actions:
        bound_act = make_bound_action(mdpsim_act)
        bound_acts_ordered.append(bound_act)

        # sanity  checks
        assert set(bound_act.props) <= prop_set, \
            "bound_act.props (for act %r) not inside prop_set; odd ones: %r" \
            % (bound_act.unique_ident, set(bound_act.props) - prop_set)
        assert bound_act.prototype in ub_act_set, \
            "%r (bound_act.prototype) is not in %r (ub_act_set)" \
            % (bound_act.protype, ub_act_set)
    # again, need to sort lexically
    bound_acts_ordered.sort()

    return ProblemMeta(problem.name, domain_meta, bound_acts_ordered,
                       bound_props_ordered, goal_props)
