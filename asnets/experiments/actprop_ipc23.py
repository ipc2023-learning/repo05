"""A two-layer configuration for the action/proposition network w/ Fast
Downward (FD) teacher."""

# use defaults from actprop_1l
"""One-layer configuration for action-proposition network"""

# train supervised or RL? (only supervised supported at the moment)
SUPERVISED = True

# learning rate
SUPERVISED_LEARNING_RATE = 1e-3  # EXPERIMENTAL
# can also specify some steps to jump down from initial rate (e.g [(10, 1e-3),
# (20, 1e-4)] jumps down to 1e-3 after 10 epochs, and down to 1e-4 after 20
# epochs)
LEARNING_RATE_STEPS = [(500, 1e-4), (1000, 1e-5)]  # EXPERIMENTAL
# batch size
SUPERVISED_BATCH_SIZE = 64  # EXPERIMENTAL
MAX_OPT_EPOCHS = 100
# number of batches of optimisation per epoch
OPT_BATCH_PER_EPOCH = 100  # EXPERIMENTAL
# num of epochs after which to do early stopping if success rate is high but
# doesn't increase (0 disables)
SUPERVISED_EARLY_STOP = 20
# save model every N epochs, in addition to normal saving behaviour (on success
# rate increase & at end of training); 0 disables additional saves
SAVE_EVERY_N_EPOCHS = 1
# heuristic for supervised teacher (if FD)
SSIPP_TEACHER_HEURISTIC = 'lm-cut'
FD_TEACHER_HEURISTIC = 'lama-first'
# type of planner to use as teacher
TEACHER_PLANNER = 'fd'
# controls strategy used to teacher the planner; try ANY_GOOD_ACTION if you
# want the ASNet to choose evenly between all actions that have minimal teacher
# Q-value, or THERE_CAN_ONLY_BE_ONE to imitate the single action that the
# planner would return if you just ran it on the current state
TRAINING_STRATEGY = 'THERE_CAN_ONLY_BE_ONE'
# use 'ROLLOUT' to only accumulate optimal policy rollouts, or 'ENVELOPE' to
# accumulate entire optimal policy envelopes
TEACHER_EXPERIENCE_MODE = 'ROLLOUT'
# regularisers; SOME regularisation is needed so that objective is bounded
# below & l2 seems like reasonable default
L2_REG = 2e-4  # EXPERIMENTAL
L1_REG = 0.0  # EXPERIMENTAL
# can be used to turn on dropout (at training time only)
DROPOUT = 0.1   # EXPERIMENTAL
# target number of ASNet rollouts to add to the replay buffer at the beginning
# of each epoch (higher = more data, but more planning cost)
TARGET_ROLLOUTS_PER_EPOCH = 70  # EXPERIMENTAL (moved 90 -> 70 to help PBW)
LIMIT_TRAIN_OBS_SIZE = 1000
# model flags
NUM_LAYERS = 2
HIDDEN_SIZE = 16  # EXPERIMENTAL
# include skip connections
SKIP = True
USE_LMCUT_FEATURES = True

# train + eval settings
# For Joerg:
# TIME_LIMIT_SECONDS = int(60 * 60 * 8)  # XXX this is probably too much
# EVAL_ROUNDS = 2  # XXX det only really
# Normal settings:
TIME_LIMIT_SECONDS = int(60 * 60 * 72)
# optionally, we can have lower limit for rollouts in case we have HUGE test
# problems
EVAL_TIME_LIMIT_SECONDS = int(60 * 30)
ROUND_TURN_LIMIT = 300

# deterministic eval, so only need one round
EVAL_ROUNDS = 1
DET_EVAL = True
# these probably help with GM, and might help with other things, too
USE_ACT_HISTORY_FEATURES = True
