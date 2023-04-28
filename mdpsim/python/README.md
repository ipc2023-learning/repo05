# Python bindings for MDPSim

This directory contains the source of some Python bindings for MDPSim. `pip
install ./mdpsim` should be sufficient to install them.

I adapted `setup.py` from pybind11's Python example, so I've included the
relevant license here. I can probably put some other license on the thing if I
just rewrite `setup.py` from scratch. For now, it should not matter (the whole
wrapper is probably bound by the pybind11 terms anyway because it includes the
pybind11 headers).
