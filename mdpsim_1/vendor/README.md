This directory is for bundled third-party libraries. Library descriptions:

- `pybind11`: headers for `pybind11` v2.2.4. I decided to vendor this because
  including `pybind11` in `setup_requires` (in `setup.py`) [doesn't actually do
  anything yet](https://github.com/pybind/pybind11/issues/1067), so it's
  impossible to reliably package in the PyPI version of `pybind11` when using
  setuptools.
