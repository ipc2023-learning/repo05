## ASNets - IPC23

### To Build

1. Download and install [Apptainer](https://apptainer.org/docs/admin/main/installation.html)
2. Run `apptainer build learn.sif apptainer.asnets.learn` and 
3. Run `apptainer build plan.sif apptainer.asnets.plan`


### To run

1. To run the learning script:

```bash
./learn.sif DK DOMAIN TASK1 TASK2 ...
```

Where ```DK``` is the name of output policy. To successfully run the script,  at least provide four tasks.

Example:

```bash
./learn.sif policy.dt benchmarks/blocksworld/domain.pddl benchmarks/blocksworld/training/easy/p01.pddl benchmarks/blocksworld/training/easy/p02.pddl benchmarks/blocksworld/training/easy/p03.pddl benchmarks/blocksworld/training/easy/p04.pddl
```

2. To run the planning script:

```bash
./plan.sif DK DOMAIN TASK PLAN
```

Where ```DK``` is the name of learnt policy and `PLAN` is the name of the output plan file. To successfully run the script, only one problem should be given.

Example:

```bash
./plan.sif dk benchmarks/blocksworld/domain.pddl benchmarks/blocksworld/training/easy/p01.pddl
```

For more detail please find on the [competition website](https://ipc2023-learning.github.io/).

