SSiPP
=====

This code implements SSiPP and Labeled SSiPP. For the description of these
planners, see the paper ["Depth-based Short-sighted Stochastic Shortest Path
Problems" AIJ'14](http://felipe.trevizan.org/papers/trevizan14:depth.pdf)


Compiling
---------

To compile, run

```bash
build.py --opt solver_ssp
```

For more options (e.g., compiler path and flags) run `build.py -h`

### Requirements
* C++11
* gcc/g++ >= 4.7 (tested with versions 4.7 and 4.9)
* yacc and lex if the PPDDL parser is recompiled (use option -k on build.py to
  avoid recompiling the parser)

Running
-------

To see the options, run `solver_ssp`

### Examples

* Run Labeled SSiPP using depth-based short-sighted SSPs with depth 4 until the
initial state is labeled as solved (enforcing a maximum CPU-time of 600s):

```bash
solver_ssp -h lm-cut -p labeledssipp:lrtdp:depth:4 --conv_s0 --max_cpu_time_sec 600 exbw/exbw_p08-n8-N10-s8.pddl
```

* Run SSiPP from initial state to the goal 50 times (i.e., 50 rounds):

```bash
solver_ssp -h h-add -p ssipp:lrtdp:depth:4 -R 50 exbw/exbw_p08-n8-N10-s8.pddl
```

![logo](http://felipe.trevizan.org/ssipp_logo.png)
