## ASNets - IPC23

### To Install

1. Download and install [Apptainer](https://apptainer.org/docs/admin/main/installation.html)

2. Clone this project to `CONTENT_ROOT`

3. Open `CONTENT_ROOT` in terminal, and run 

   ```bash
   apptainer build learn.sif apptainer.asnets.learn
   apptainer build plan.sif apptainer.asnets.plan
   ```

   


### To run

1. To run the learning script:

```bash
./learn.sif DK DOMAIN TASK1 TASK2 ...
```

The learner will write its learned domain knowledge (i.e. the policy model) into the “DK” file. 

2. To run the planning script:

```bash
./plan DK DOMAIN TASK PLAN
```

The planner will write the plan into the "PLAN" file.



For more detail about this submission we refer to the [competition's website](https://ipc2023-learning.github.io/#submission). 

