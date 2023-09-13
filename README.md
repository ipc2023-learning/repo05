## ASNets - IPC23

### To Install

1. Download and install *Docker* and *VSCode*
2. Clone and open this project in *VSCode*
3. Install all the Microsoft "remote" extensions
4. Open control palette, and select "Dev Containers: Reopen in Container"


### To run

1. Open up a new terminal and run:

```source venv-asnets/bin/activate```

2. To run the learning script:

```bash
python asnets/run_learn experiments.actprop_ipc23 {dk} {domain_file} {problem_files_1}...{problem_files_n} 
```

Where ```experiments.actprop_ipc23``` contains all hyperparameters, and ```{dk}``` is the name of learnt policy. To successfully run the script, you need to pass at least four problems.

Example:
```bash
python asnets/run_learn experiments.actprop_ipc23 dk benchmarks/blocksworld/domain.pddl benchmarks/blocksworld/training/easy/p01.pddl benchmarks/blocksworld/training/easy/p02.pddl benchmarks/blocksworld/training/easy/p03.pddl benchmarks/blocksworld/training/easy/p04.pddl
```

3. To run the planning script:
```bash
python asnets/run_plan experiments.actprop_ipc23 {dk} {domain_file} {problem_file} 
```

Where ```experiments.actprop_ipc23``` contains all hyperparameters, and ```{dk}``` is the name of learnt policy. To successfully run the script, only one problem should be passed.

Example:
```bash
python asnets/run_plan experiments.actprop_ipc23 dk benchmarks/blocksworld/domain.pddl benchmarks/blocksworld/training/easy/p01.pddl
```