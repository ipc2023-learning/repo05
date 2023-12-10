# Generalised policy trainer (Action Schema Networks)



## Installation

See [this file](../README.md) for project installation. 

## Running manually

`run_learning.py` is the entry point for the trainer. `python run_learning.py
--help` will explain the options which can be supplied to the trainer:

```sh
# Train on p01-p06; the `{1,2,3,4,5,6}*.pddl` Bash syntax
# is a concise way of including all relevant PDDL files (except domain)
python run_learning.py \
		dk \
		actprop \
   {PATH_TO_BENCHMARS}/domain.pddl \
   {PATH_TO_BENCHMARKS}/p0{1,2,3,4,5,6}*.pddl
```

Here's a quick rundown of the most important options:

- `dk` The name of the output domain knowledge file
- `actprop `Trainer configuration file located in `./experiments/`. It contains most of parameters for the model. 
