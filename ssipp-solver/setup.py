from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import shutil
from subprocess import run, PIPE
import os
import sys

__version__ = '0.0.1'

ext_modules = [
    # this will be built externally
    Extension('ssipp', [])
]


class SSiPPBuildError(Exception):
    pass


class BuildExt(build_ext):
    def build_extension(self, ext):
        # For example of prebuilt extension, see
        # https://github.com/frida/frida-python/blob/6084f2fef4ac9a28b89c0775b4de2f326630fb21/src/setup.py
        # TODO: try supporting build_ext.* flags (e.g. debug, parallel,
        # whatever)
        rv = run(['python', 'build.py', 'pybind'],
                 stdout=PIPE,
                 stderr=PIPE,
                 universal_newlines=True,
                 check=False)
        if rv.returncode != 0:
            print('SSiPP build failed. stdout:', file=sys.stderr)
            print(rv.stdout, file=sys.stderr)
            print('\n\nstderr:', file=sys.stderr)
            print(rv.stderr, file=sys.stderr)
            print('\n', file=sys.stderr)
            raise SSiPPBuildError('SSiPP returned code %d' % rv.returncode)
        lines = rv.stdout.splitlines()
        prefix = 'Extension: '
        ext_lines = [
            l[len(prefix):] for l in lines if l.startswith('Extension: ')
        ]
        ext_name = ext_lines[0]
        dest = self.get_ext_fullpath(ext.name)
        print("Copying to %s" % dest)
        targ_dir = os.path.dirname(dest)
        os.makedirs(targ_dir, exist_ok=True)
        shutil.copy(ext_name, dest)


with open('README.md') as fp:
    long_description = fp.read()

setup(
    name='ssipp',
    version=__version__,
    author='Sam Toyer (wrapping planner by Felipe Trevizan)',
    author_email='sam@qxcv.net',
    url='https://gitlab.com/qxcv/ssipp',
    description='Python wrapper for SSiPP',
    long_description=long_description,
    ext_modules=ext_modules,
    setup_requires=['pybind11>=2.2.4,<3.0'],
    install_requires=['pybind11>=2.2.4,<3.0'],
    cmdclass=dict(build_ext=BuildExt),
    zip_safe=False)
