# Install ASNets
python3 -m venv venv-asnets && . venv-asnets/bin/activate
pip3 install --upgrade pip
pip3 install wheel cython numpy pkgconfig werkzeug urllib3==1.26.15
pip3 install -e asnets

# Install ssipp
pip3 install -e ssipp-solver

# Install MDPSIM
pip3 install -e mdpsim

# Install PDDL parser
cd pddl-parser && python3 setup.py install && cd ..

# Install PySAT
pip3 install python-sat[pblib,aiger]

# Install Fast Downward
cd downward && ./build.py && cd ..

# export PYTHONPATH=$PYTHONPATH:.

# . venv-asnets/bin/activate