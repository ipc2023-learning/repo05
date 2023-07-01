from asnets.ops.asnet_ops import multi_gather_concat, multi_pool_concat
from asnets.utils.prof_utils import can_profile
from asnets.utils.tf_utils import masked_softmax
import joblib
import numpy as np
import tensorflow as tf

USE_CUSTOM_MULTI_GATHER_CONCAT = True
USE_CUSTOM_MULTI_POOL_CONCAT = True
NONLINEARITY = 'elu'


class WeightData:
    def __init__(self, shape, name, value=None):
        self.shape = shape
        self.name = name
        self.value = value


class PropNetworkWeights:
    """Manages weights for a domain-specific problem network. Those weights can
    then be used in problem-specific networks."""

    # WARNING: you need to change __{get,set}state__ if you change __init__ or
    # _make_weights()!

    def __init__(self, dom_meta, hidden_sizes, extra_dim, skip):
        # note that hidden_sizes is list of (act layer size, prop layer size)
        # pairs.
        # extra_input is just the number of extra items included in the input
        # vector for each action
        self.dom_meta = dom_meta
        self.hidden_sizes = list(hidden_sizes)
        self.extra_dim = extra_dim
        self.skip = skip
        self._make_weights()

    def __getstate__(self):
        """Pickle weights ourselves, since TF stuff is hard to pickle."""
        prop_weights_np = self._serialise_weight_list(self.prop_weights)
        act_weights_np = self._serialise_weight_list(self.act_weights)
        return {
            'dom_meta': self.dom_meta,
            'hidden_sizes': self.hidden_sizes,
            'prop_weights_np': prop_weights_np,
            'act_weights_np': act_weights_np,
            'extra_dim': self.extra_dim,
            'skip': self.skip,
        }

    def __setstate__(self, state):
        """Unpickle weights"""
        self.dom_meta = state['dom_meta']
        self.hidden_sizes = state['hidden_sizes']
        self.extra_dim = state['extra_dim']
        # old network snapshots always had skip connections turned on
        self.skip = state.get('skip', True)
        self._make_weights(state['prop_weights_np'], state['act_weights_np'])

    @staticmethod
    def _serialise_weight_list(weight_list):
        # serialises a list of dicts, each mapping str -> (tensorflow weights,
        # ...)
        sess = tf.compat.v1.get_default_session()
        rv = []
        for d in weight_list:
            new_d = {}
            for k, v in d.items():
                new_d[k] = v
            rv.append(new_d)
        return rv

    @can_profile
    def _make_weights(self, old_prop_weights=None, old_act_weights=None):
        # prop_weights[i] is a dictionary mapping predicate names to weights
        # for modules in the i-th proposition layer
        self.prop_weights = []
        self.act_weights = []
        self.all_weights = []

        # TODO: constructing weights separately like this (and having
        # to restore with tf.const, etc.) is silly. Should store
        # parameters *purely* by name, and have code responsible for
        # automatically re-instantiating old weights (after network
        # construction) if they exist. TF offers several ways of doing
        # exactly that.

        for hid_idx, hid_sizes in enumerate(self.hidden_sizes):
            act_size, prop_size = hid_sizes

            # make action layer weights
            act_dict = {}
            for unbound_act in self.dom_meta.unbound_acts:
                preds = self.dom_meta.rel_pred_names(unbound_act)
                if not hid_idx:
                    # first layer, so our input is actually a binary vector
                    # giving a truth value for each proposition
                    in_size = len(preds) * 2 + self.extra_dim
                else:
                    # prop inputs + skip input from previous action layer
                    in_size = len(preds) * self.hidden_sizes[hid_idx - 1][1]
                    if self.skip:
                        in_size = in_size + self.hidden_sizes[hid_idx - 1][0]

                name_pfx = 'hid_%d_act_%s' % (hid_idx, unbound_act.schema_name)
                if old_act_weights is not None:
                    act_W = tf.Variable(
                        tf.constant_initializer(value=old_act_weights[hid_idx][unbound_act][0].numpy())(
                            shape=(in_size, act_size)),
                        shape=(in_size, act_size),
                        name=name_pfx + '/W',
                        trainable=True)

                    act_b = tf.Variable(
                        tf.constant_initializer(
                            value=old_act_weights[hid_idx][unbound_act][1].numpy())(shape=(act_size, )),
                        shape=(act_size, ),
                        name=name_pfx + '/b',
                        trainable=True)

                else:
                    act_W = tf.Variable(
                        tf.keras.initializers.VarianceScaling(
                            scale=1.0,
                            mode="fan_avg",
                            distribution="uniform")(shape=(in_size, act_size)),
                        name=name_pfx + '/W',
                        trainable=True
                    )

                    act_b = tf.Variable(
                        tf.zeros_initializer()(shape=(act_size, )),
                        name=name_pfx + '/b',
                        trainable=True
                    )

                act_dict[unbound_act] = (act_W, act_b)
                self.all_weights.extend([act_W, act_b])

            self.act_weights.append(act_dict)

            # make hidden proposition layer weights
            pred_dict = {}
            for pred_name in self.dom_meta.pred_names:
                rel_act_slots = self.dom_meta.rel_act_slots(pred_name)
                # We should never end up with NO relevant actions & slots for a
                # predicate, else there's probably issue with domain.
                assert len(rel_act_slots) > 0, \
                    "no relevant actions for proposition %s" % pred_name

                in_size = len(rel_act_slots) * act_size
                if hid_idx and self.skip:
                    # skip connection from previous prop layer
                    in_size = in_size + self.hidden_sizes[hid_idx - 1][1]
                name_pfx = 'hid_%d_prop_%s' % (hid_idx, pred_name)
                if old_prop_weights is not None:
                    prop_W = tf.Variable(
                        tf.constant_initializer(value=old_prop_weights[hid_idx][pred_name][0].numpy())(
                            shape=(in_size, prop_size)),
                        shape=(in_size, act_size),
                        name=name_pfx + '/W',
                        trainable=True)

                    prop_b = tf.Variable(
                        tf.constant_initializer(value=old_prop_weights[hid_idx][pred_name][1].numpy())(
                            shape=(prop_size, )),
                        shape=(act_size, ),
                        name=name_pfx + '/b',
                        trainable=True)
                    # prop_W = WeightData(shape=(in_size, prop_size),
                    #                    name=name_pfx + '/W',
                    #                    value=old_prop_weights[hid_idx][pred_name][0])
                    # prop_b = WeightData(shape=(prop_size, ),
                    #                    name=name_pfx + '/b',
                    #                    value=old_prop_weights[hid_idx][pred_name][1])
                else:
                    prop_W = tf.Variable(
                        tf.keras.initializers.VarianceScaling(
                            scale=1.0,
                            mode="fan_avg",
                            distribution="uniform")(shape=(in_size, prop_size)),
                        name=name_pfx + '/W',
                        trainable=True
                    )

                    prop_b = tf.Variable(
                        tf.zeros_initializer()(shape=(prop_size, )),
                        name=name_pfx + '/b',
                        trainable=True
                    )
                pred_dict[pred_name] = (prop_W, prop_b)
                self.all_weights.extend([prop_W, prop_b])

            self.prop_weights.append(pred_dict)

        # make final layer weights (action)
        final_act_dict = {}
        for unbound_act in self.dom_meta.unbound_acts:
            preds = self.dom_meta.rel_pred_names(unbound_act)
            if not self.hidden_sizes:
                in_size = len(preds) * 2 + self.extra_dim
            else:
                in_size = len(preds) * self.hidden_sizes[-1][1]
                if self.skip:
                    in_size = in_size + self.hidden_sizes[-1][0]

            name_pfx = 'final_act_%s' % unbound_act.schema_name
            if old_act_weights is not None:
                final_act_W = tf.Variable(
                    tf.constant_initializer(
                        value=old_act_weights[-1][unbound_act][0].numpy())(shape=(in_size, 1)),
                    shape=(in_size, 1),
                    name=name_pfx + '/W',
                    trainable=True)

                final_act_b = tf.Variable(
                    tf.constant_initializer(
                        value=old_act_weights[-1][unbound_act][1].numpy())(shape=(1, )),
                    shape=(1, ),
                    name=name_pfx + '/b',
                    trainable=True)
            else:
                final_act_W = tf.Variable(
                    tf.keras.initializers.VarianceScaling(
                        scale=1.0,
                        mode="fan_avg",
                        distribution="uniform")(shape=(in_size, 1)),
                    name=name_pfx + '/W',
                    trainable=True
                )

                final_act_b = tf.Variable(
                    tf.zeros_initializer()(shape=(1, )),
                    name=name_pfx + '/b',
                    trainable=True
                )
            final_act_dict[unbound_act] = (final_act_W, final_act_b)
            self.all_weights.extend([final_act_W, final_act_b])

        self.act_weights.append(final_act_dict)

    @can_profile
    def save(self, path):
        """Save a snapshot of the current network weights to the given path."""
        joblib.dump(self, path, compress=True)


class PropNetwork(tf.keras.layers.Layer):

    def __init__(self, weight_manager, problem_meta,
                 dropout=0.0, debug=False,
                 trainable=True, name=None,
                 dtype=None, dynamic=False,
                 **kwargs):
        super().__init__(trainable, name, dtype, dynamic, **kwargs)

        self._weight_manager = weight_manager
        self._prob_meta = problem_meta
        self._debug = debug
        # I tried ReLU, tanh, softplus, & leaky ReLU before settling on ELU for
        # best combination of numeric stability + sample efficiency
        self.nonlinearity = getattr(tf.nn, NONLINEARITY)
        # should we include skip connections?
        self.skip = self._weight_manager.skip

        self.dropout = dropout

        hidden_sizes = self._weight_manager.hidden_sizes
        dom_meta = self._weight_manager.dom_meta

        # this is useful for getting values from ALL action/proposition layers
        self.act_layers = []
        self.prop_layers = []
        self.action_layer_input = {}
        self.weights_collection = []
        self.bias_collection = []

        # is this the input layer? (gets set to False on first loop)
        is_input = True

        # hidden layers
        for hid_idx, hid_sizes in enumerate(hidden_sizes):
            act_dict = {}
            for unbound_act in dom_meta.unbound_acts:

                weight, bias = self._weight_manager.act_weights[hid_idx][unbound_act]

                act_dict[unbound_act] = ActionPropModule(
                    weight_manager=self._weight_manager,
                    prop_or_act="act",
                    problem_meta=self._prob_meta,
                    unbound_clause=unbound_act,
                    layer_num=hid_idx,
                    weight=weight,
                    bias=bias,
                    nonlinearity=self.nonlinearity,
                    dropout=self.dropout,
                    save_input=is_input)

                self.weights_collection.append(weight)
                self.bias_collection.append(bias)

            is_input = False
            self.act_layers.append(act_dict)

            pred_dict = {}
            for pred_name in dom_meta.pred_names:

                weight, bias = self._weight_manager.prop_weights[hid_idx][pred_name]

                pred_dict[pred_name] = ActionPropModule(
                    weight_manager=self._weight_manager,
                    prop_or_act="prop",
                    problem_meta=self._prob_meta,
                    unbound_clause=pred_name,
                    layer_num=hid_idx,
                    weight=weight,
                    bias=bias,
                    nonlinearity=self.nonlinearity,
                    dropout=self.dropout)
                self.weights_collection.append(weight)
                self.bias_collection.append(bias)
            self.prop_layers.append(pred_dict)

        # final (action) layer
        finals = {}
        for unbound_act in dom_meta.unbound_acts:

            weight, bias = self._weight_manager.act_weights[len(
                hidden_sizes)][unbound_act]

            finals[unbound_act] = ActionPropModule(weight_manager=self._weight_manager,
                                                   prop_or_act="act",
                                                   problem_meta=self._prob_meta,
                                                   unbound_clause=unbound_act,
                                                   layer_num=len(hidden_sizes),
                                                   weight=weight,
                                                   bias=bias,
                                                   nonlinearity=tf.identity,
                                                   dropout=0.0)
            self.weights_collection.append(weight)
            self.bias_collection.append(bias)
        self.act_layers.append(finals)

    def _split_input(self, obs_layer):
        """Splits an observation layer up into appropriate proposition
        layers."""
        # TODO: comment below about _merge_finals ugliness also applies here
        prob_meta = self._prob_meta
        prop_to_flat_input_idx = {
            prop: idx
            for idx, prop in enumerate(prob_meta.bound_props_ordered)
        }
        rv = {}
        # This is a hack!! Need to fix this later on.
        to_change = []
        zero_like_shape = []
        for pred_name in prob_meta.domain.pred_names:
            sub_props = prob_meta.pred_to_props(pred_name)
            gather_inds = []
            for sub_prop in sub_props:
                to_look_up = prop_to_flat_input_idx[sub_prop]
                gather_inds.append(to_look_up)

            if len(gather_inds) != 0:

                rv[pred_name] = tf.gather(obs_layer,
                                      gather_inds,
                                      axis=1,
                                      name='split_input/' + pred_name)
                zero_like_shape = tf.shape(rv[pred_name])

            else: 
                to_change.append(pred_name)
        
        for pred_name in to_change:
            rv[pred_name] = tf.zeros(zero_like_shape)
        return rv

    def _split_extra(self, extra_data):
        """Sometimes we also have input data which goes straight to the
        network. We need to split this up into an unbound action->tensor
        dictionary just like the rest."""
        prob_meta = self._prob_meta
        out_dict = {}
        # This is a hack!! Need to fix this later on.
        to_change = []
        zero_like_shape = []
        for unbound_act in prob_meta.domain.unbound_acts:
            ground_acts = prob_meta.schema_to_acts(unbound_act)
            sorted_acts = sorted(ground_acts,
                                 key=prob_meta.act_to_schema_subtensor_ind)
            if len(sorted_acts) == 0:
                # FIXME: make this message scarier
                print("no actions for schema %s?" % unbound_act.schema_name)
            # these are the indices which we must read and concatenate
            tensor_inds = [
                # TODO: make this linear-time (or linearithmic) by using a dict
                prob_meta.bound_acts_ordered.index(act) for act in sorted_acts
            ]

            if len(tensor_inds) != 0:
                out_dict[unbound_act] = tf.gather(extra_data,
                                              tensor_inds,
                                              axis=1,
                                              name='split_extra/' +
                                              unbound_act.schema_name)
                zero_like_shape = tf.shape(out_dict[unbound_act])

            else: 
                to_change.append(unbound_act)
        
        for unbound_act in to_change:
            out_dict[unbound_act] = tf.zeros(zero_like_shape)
        return out_dict

    def call(self, inputs, *args, **kwargs):
        super().call(inputs, *args, **kwargs)

        hidden_sizes = self._weight_manager.hidden_sizes
        dom_meta = self._weight_manager.dom_meta
        prob_meta = self._prob_meta

        # input vector spec:
        #
        # |<--num_acts-->|<--k*num_acts-->|<--num_props-->|
        # | action mask  |  action data   | propositions  |
        #
        # 1) `action_mask` tells us whether actions are enabled
        # 2) `action_data` is passed straight to action modules
        # 3) `propositions` tells us what is and isn't true
        #
        # Reminder: this convoluted input shape is required solely because of
        # rllab's inflexible input conventions (it can only take a single
        # vector per state).
        #
        # FIXME: now that I'm not using RLLab any more, how about I change this
        # input convention around so that it actually makes sense? Should
        # collect inputs for each action module in Numpy vectors, then pass
        # them directly to the network as different k/v pairs in a feed_dict.

        mask_size = prob_meta.num_acts
        extra_data_dim = self._weight_manager.extra_dim
        extra_size = extra_data_dim * prob_meta.num_acts
        prop_size = prob_meta.num_props
        # in_dim = mask_size + extra_size + prop_size
        # assert in_dim == inputs.shape[1], f"Inconsistent input shape {inputs.shape}"

        def act_extra_inner(in_vec):
            act_vecs = in_vec[:, mask_size:mask_size + extra_size]
            out_shape = (-1, prob_meta.num_acts, extra_data_dim)
            return tf.reshape(act_vecs, out_shape)

        def obs_inner(in_vec):
            # print(in_vec)
            prop_truth = in_vec[:, mask_size + extra_size:, None]
            # FIXME: it doesn't make sense to mess with goal vectors here; that
            # should be ActionDataGenerator's job, or whatever. Should be
            # passed in as part of network input, not fixed as  TF constant!
            goal_vec = [
                float(prop in prob_meta.goal_props)
                for prop in prob_meta.bound_props_ordered
            ]

            assert sum(goal_vec) == len(prob_meta.goal_props)
            assert any(goal_vec), 'there are no goals?!'
            assert not all(goal_vec), 'there are no goals?!'

            # apparently this broadcasts (hooray!)
            tf_goals = tf.constant(goal_vec)[None, :, None]
            batch_size = tf.shape(input=prop_truth)[0]
            tf_goals_broad = tf.tile(tf_goals, (batch_size, 1, 1))
            l_obs = tf.concat([prop_truth, tf_goals_broad], axis=2)
            return l_obs
        # [None, num_props, 2]
        l_obs = obs_inner(inputs)
        # {predicate: [None, props_of_this_pred, 2]}
        pred_dict = self._split_input(l_obs)
        if extra_data_dim > 0:
            # [None, num_actions, extra_dimension]
            l_act_extra = act_extra_inner(inputs)
            extra_dict = self._split_extra(l_act_extra)
        else:
            extra_dict = None

        # this is useful for getting values from ALL action/proposition layers
        self.act_layers_outcome = []
        self.prop_layers_outcome = []

        # Input + hidden layers
        prev_act_dict = {}
        prev_pred_dict = {}
        for hid_idx, hid_sizes in enumerate(hidden_sizes):

            act_outcome_dict = {}
            act_module_dict = self.act_layers[hid_idx]
            for unbound_act in dom_meta.unbound_acts:
                act_outcome_dict[unbound_act] = act_module_dict[unbound_act].forward(
                    prev_dict=pred_dict,
                    extra_dict=extra_dict,
                    prev_layer=prev_act_dict.get(unbound_act, None))

            self.act_layers_outcome.append(act_outcome_dict)
            prev_act_dict = act_outcome_dict

            pred_dict = {}
            pred_module_dict = self.prop_layers[hid_idx]
            for pred_name in dom_meta.pred_names:
                pred_dict[pred_name] = pred_module_dict[pred_name].forward(
                                    prev_dict=act_outcome_dict,
                                    extra_dict=extra_dict,
                                    prev_layer=prev_pred_dict.get(pred_name, None))

            self.prop_layers_outcome.append(pred_dict)
            prev_pred_dict = pred_dict

        # final (action) layer
        finals = {}
        act_module_dict = self.act_layers[len(hidden_sizes)]
        for unbound_act in dom_meta.unbound_acts:
            finals[unbound_act] = act_module_dict[unbound_act].forward(
                prev_dict=pred_dict,
                extra_dict=extra_dict,
                prev_layer=prev_act_dict.get(unbound_act, None))

        l_pre_softmax = _merge_finals(prob_meta, finals)
        l_mask = inputs[:, :mask_size]
        # voila!
        return masked_softmax(l_pre_softmax, l_mask)


class ActionPropModule:

    def __init__(self,
                 weight_manager,
                 weight,
                 bias,
                 prop_or_act,
                 unbound_clause,
                 layer_num,
                 problem_meta,
                 nonlinearity=None,
                 dropout=0.0,
                 save_input=False):

        self.weight_manager = weight_manager
        self.unbound_clause = unbound_clause
        self.layer_num = layer_num
        self.nonlinearity = nonlinearity
        self.dropout = dropout
        self.save_input = save_input
        self.prop_or_act = prop_or_act
        self.prob_meta = problem_meta
        self.w = weight
        self.b = bias
        self.skip = weight_manager.skip

        if self.prop_or_act == "act":
            self.name_pfx = '%s_mod_%s_%d' % (
                self.prop_or_act, unbound_clause.schema_name, layer_num)
        else:
            self.name_pfx = '%s_mod_%s_%d' % (
                self.prop_or_act, unbound_clause, layer_num)

    def forward(self, prev_dict, extra_dict, prev_layer):

        if self.prop_or_act == "act":
            # sort input layers so we can index into them properly
            pred_to_tensor_idx, prev_inputs = _sort_inputs(prev_dict)
            dom_meta = self.weight_manager.dom_meta
            # hack: if not grounded, create a dummy input
            if len(self.prob_meta.schema_to_acts(self.unbound_clause)) == 0:
                batch_size = tf.shape(prev_inputs[0])[0]
                conv_input = tf.zeros([batch_size, 1, tf.shape(self.w)[0]])
            else:
                # this tells us how many channels our input will need
                # [index of the pred in prev_dict,
                #   [prop index of the pred for each ground action,
                #   len = num of ground actions]
                # ] len=num of rel pred.
                index_spec = []
                dom_rel_preds = dom_meta.rel_pred_names(self.unbound_clause)
                for act_pred_idx, arg_pred in enumerate(dom_rel_preds):
                    pools = []
                    for ground_act in self.prob_meta.schema_to_acts(self.unbound_clause):
                        # we're looking at the act_pred_idx-th relevant proposition
                        bound_prop = self.prob_meta.rel_props(ground_act)[
                            act_pred_idx]
                        prop_idx = self.prob_meta.prop_to_pred_subtensor_ind(
                            bound_prop)
                        # we're only "pooling" over one element (the proposition
                        # features)
                        pools.append([prop_idx])

                    # which tensor do we need to pick this out of?
                    tensor_idx = pred_to_tensor_idx[arg_pred]
                    index_spec.append((tensor_idx, pools))

                extra_chans = []
                if self.layer_num == 0 and extra_dict is not None:
                    # first action layer, so add in extra data
                    act_data = extra_dict[self.unbound_clause]
                    extra_chans.append(act_data)
                if self.layer_num > 0 and self.skip:
                    assert prev_layer is not None, \
                        "act mod in L%d not supplied previous acts for skip conn" \
                        % self.layer_num
                    extra_chans.append(prev_layer)
                elif self.layer_num == 0 and self.skip:
                    assert prev_layer is None, "ugh this shouldn't happen in layer 0"
                if USE_CUSTOM_MULTI_GATHER_CONCAT:
                    with tf.name_scope(self.name_pfx + '/mgc'):
                        mgc_inputs = []
                        mgc_elem_indices = []
                        for tensor_idx, pools in index_spec:
                            # which pred tensor
                            mgc_inputs.append(prev_inputs[tensor_idx])
                            elem_inds = [p for p, in pools]
                            mgc_elem_indices.append(
                                tf.constant(elem_inds, dtype=tf.int64))  # which column of the pred tensor
                        for extra_chan in extra_chans:
                            mgc_inputs.append(extra_chan)
                            extra_chan_width = tf.cast(
                                tf.shape(input=extra_chan)[1], tf.int64)
                            mgc_elem_indices.append(
                                tf.range(extra_chan_width, dtype=tf.int64))
                            # helps out shape inference if extra_chan.shape[1] is known
                            mgc_elem_indices[-1].set_shape(extra_chan.shape[1])
                        # shape [None, num_of_actions, len_input(len(props)+extra)]
                        conv_input = multi_gather_concat(
                            mgc_inputs, mgc_elem_indices)
                else:
                    assert False, "Have to set USE_CUSTOM_MULTI_GATHER_CONCAT True"
            # if self.save_input:
                # hack so that I can get at input to the convolution when
                # displaying layer-by-layer network activations (it's easier to do
                # it this way)
                # self.action_layer_input[self.unbound_clause] = conv_input

        else:
            act_to_tensor_idx, prev_inputs = _sort_inputs(prev_dict)
            dom_meta = self.weight_manager.dom_meta
            if not self.prob_meta.pred_to_props(self.unbound_clause):
                batch_size = tf.shape(prev_inputs[0])[0]
                conv_input = tf.zeros([batch_size, 1, tf.shape(self.w)[0]])
            else:
                index_spec = []
                pred_rela_slots = dom_meta.rel_act_slots(self.unbound_clause)
                for rel_unbound_act_slot in pred_rela_slots:
                    pools = []
                    for prop in self.prob_meta.pred_to_props(self.unbound_clause):
                        # we're looking at the act_pred_idx-th relevant proposition
                        rel_slots = self.prob_meta.rel_act_slots(prop)
                        ground_acts = [
                            ground_act for unbound_act, slot, ground_acts in rel_slots
                            for ground_act in ground_acts
                            if (unbound_act, slot) == rel_unbound_act_slot
                        ]
                        act_inds = [
                            self.prob_meta.act_to_schema_subtensor_ind(ground_act)
                            for ground_act in ground_acts
                        ]
                        pools.append(act_inds)

                    tensor_idx = act_to_tensor_idx[rel_unbound_act_slot[0]]
                    index_spec.append((tensor_idx, pools))

                extra_chans = []
                if self.layer_num > 0 and self.skip:
                    assert prev_layer is not None, \
                        "pred mod in L%d not supplied previous acts for skip conn" \
                        % self.layer_num
                    extra_chans.append(prev_layer)
                elif self.layer_num == 0 and self.skip:
                    assert prev_layer is None, "ugh this shouldn't happen in layer 0"
                if USE_CUSTOM_MULTI_POOL_CONCAT:
                    # use a custom fused op to create input to prop module
                    with tf.name_scope(self.name_pfx + '/mpc'):
                        mpc_inputs = []
                        mpc_ragged_pools = []
                        for tensor_idx, py_pools in index_spec:
                            mpc_inputs.append(prev_inputs[tensor_idx])
                            flat_pools = sum(py_pools, [])
                            pool_lens = [len(p) for p in py_pools]
                            ragged_pool = tf.cast(
                                tf.RaggedTensor.from_row_lengths(
                                    flat_pools, pool_lens), tf.int64)
                            mpc_ragged_pools.append(ragged_pool)
                        assert NONLINEARITY == 'elu', \
                            'minimum value of -1 is dependent on using elu'
                        min_value = -1.0
                        conv_input = multi_pool_concat(mpc_inputs, mpc_ragged_pools,
                                                    min_value)
                        if extra_chans:
                            # TODO: also test adding this directly to
                            # multi_pool_concat; is it any slower?
                            conv_input = tf.concat(
                                [conv_input, *extra_chans], axis=2)
                else:
                    assert False, "Have to set USE_CUSTOM_MULTI_POOL_CONCAT True"

        with tf.name_scope(self.name_pfx + '/conv'):
            conv_result = _apply_conv_matmul(conv_input, self.w)
            rv = self.nonlinearity(conv_result + self.b[None, :])
            # FIXME: is this really necessary after every conv module?
            # if self._debug:
            #     rv = self._finite_check(rv)

        if self.dropout > 0:
            rv = tf.nn.dropout(rv, self.dropout, name=self.name_pfx + '/drop')

        return rv


def _sort_inputs(prev_dict):
    # Sort order is kind of arbitrary. Main thing is that order implied by
    # pred_to_tensor_idx must exactly match that of prev_inputs, as
    # pred_to_tensor_idx is used to index into prev_inputs.
    input_items_sorted = sorted(prev_dict.items(), key=lambda p: p[0])
    pred_to_tensor_idx = {
        p[0]: idx
        for idx, p in enumerate(input_items_sorted)
    }
    prev_inputs = [tensor for _, tensor in input_items_sorted]
    return pred_to_tensor_idx, prev_inputs


def _apply_conv_matmul(conv_input, W):
    reshaped = tf.reshape(conv_input, (-1, conv_input.shape[2]))
    conv_result_reshaped = tf.matmul(reshaped, W)
    conv_shape = tf.shape(input=conv_input)
    batch_size = conv_shape[0]
    # HACK: if conv_input.shape[1] is not Dimension(None) (i.e. if it's
    # known) then I want to keep that b/c it will help shape inference;
    # otherwise I want to use conv_shape[1], which will make the reshape
    # succeed. It turns out the best way I can see to compare
    # conv_input.shape[1] to Dimension(None) is to abuse comparison by
    # checking whether conv_input.shape[1] >= 0 returns None or True (!!).
    # This is stupid, but I can't see a better way of doing it.
    width = conv_shape[1] if (conv_input.shape[1] >= 0) is None \
        else conv_input.shape[1]
    out_shape = (batch_size, width, W.shape[1])
    conv_result = tf.reshape(conv_result_reshaped, out_shape)
    return conv_result


def _merge_finals(prob_meta, final_acts):
    # we make a huge tensor of actions that we'll have to reorder
    sorted_final_acts = sorted(final_acts.items(), key=lambda t: t[0])
    # also get some metadata about which positions in tensor correspond to
    # which schemas
    unbound_to_super_ind = {
        t[0]: idx
        for idx, t in enumerate(sorted_final_acts)
    }
    # indiv_sizes[i] is the number of bound acts associated with the i-th
    # schema
    indiv_sizes = [
        len(prob_meta.schema_to_acts(ub)) for ub, _ in sorted_final_acts
    ]
    # cumul_sizes[i] is the sum of the number of ground actions associated
    # with each action schema *before* the i-th schema
    cumul_sizes = np.cumsum([0] + indiv_sizes)
    # this stores indices that we have to look up
    gather_list = []
    for ground_act in prob_meta.bound_acts_ordered:
        subact_ind = prob_meta.act_to_schema_subtensor_ind(ground_act)
        superact_ind = unbound_to_super_ind[ground_act.prototype]
        actual_ind = cumul_sizes[superact_ind] + subact_ind
        assert 0 <= actual_ind < prob_meta.num_acts, \
            "action index %d for %r out of range [0, %d)" \
            % (actual_ind, ground_act, prob_meta.num_acts)
        gather_list.append(actual_ind)

    # now let's actually build and reorder our huge tensor of action
    # selection probs
    cat_super_acts = tf.concat([t[1] for t in sorted_final_acts],
                               axis=1,
                               name='merge_finals/cat')
    rv = tf.gather(cat_super_acts[:, :, 0],
                   np.array(gather_list),
                   axis=1,
                   name='merge_finals/reorder')

    return rv
