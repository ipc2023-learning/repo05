#!/usr/bin/env python3

import argparse
import functools
import glob
import json
import os
import os.path
import shlex
import shutil
import socket
import subprocess
import multiprocessing
import logging
import sys
import sysconfig

DEBUG_OBJS_TO_COMPILE = []
C_COMPILER = None
CPP_COMPILER = None


def runCmd(cmd, change_to_path=None):
    """Run the given cmd and return (retcode, stdout, stderr)

      If change_to_path != None, then change_to_path is passed to subprocess as
      cwd parameter; therefore we change to the given dir before running the
      cmd
    """
    if isinstance(cmd, str):
        cmd = shlex.split(cmd)
    if not isinstance(cmd, list):
        raise NotImplemented

    if change_to_path is None:
        logging.debug('running "%s"' % " ".join(cmd))
    else:
        logging.debug('changing to "%s" and running "%s"' %
                      (change_to_path, " ".join(cmd)))

    p = subprocess.Popen(cmd,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE,
                         cwd=change_to_path)
    stdout, stderr = p.communicate()
    if isinstance(stdout, bytes):
        stdout = stdout.decode('utf8')
    if isinstance(stderr, bytes):
        stderr = stderr.decode('utf8')
    stdout = stdout.rstrip()
    stderr = stderr.rstrip()
    retcode = p.returncode
    # logging.debug('cmd = %s, retcode = %d. stderr = "%s", stdout = "%s"' %
    #               (cmd, retcode, stderr, stdout))
    # with open('compile-log.txt', 'a') as fp:
    #     print(cmd, file=fp)
    return retcode, stdout, stderr


def getModificationTime(file_name):
    try:
        return os.path.getmtime(file_name)
    except OSError:
        return 0


def generateObject(obj, rules, objdir):
    fullpath_obj = "%s/%s" % (objdir, obj)
    fullpath_dir = os.path.dirname(fullpath_obj)
    os.makedirs(fullpath_dir, exist_ok=True)
    for r in rules:
        if r.isApplicable(obj):
            # Returns tuple returned by runCmd
            return r.apply(obj, objdir)
    logging.error("No rule for object '%s'" % obj)
    return False


def phase1Parser(allowed_to_recompile_parser=True):
    if not allowed_to_recompile_parser:
        rv = True
        if not os.path.exists('ext/mgpt/lexer.cc'):
            logging.error(
                'Error! Not allowed to recompile lexer.cc but it does not exist\n'
            )
            rv = False
        if not os.path.exists('ext/mgpt/parser.cc'):
            logging.error(
                'Error! Not allowed to recompile parser.cc but it does not exist\n'
            )
            rv = False
        return rv

    recompile_lexer = False
    if not os.path.exists('ext/mgpt/lexer.l'):
        recompile_lexer = True
    else:
        parser_mtime = getModificationTime('ext/mgpt/lexer.l')
        if parser_mtime > getModificationTime('ext/mgpt/lexer.cc'):
            recompile_lexer = True
    if recompile_lexer:
        retcode, stdout, stderr = runCmd(
            ['lex', '-o', 'ext/mgpt/lexer.cc', 'ext/mgpt/lexer.l'])
        if retcode != 0:
            logging.error('Error recompiling lexer:\n%s' % stderr)
            return False

    recompile_parser = False
    if not os.path.exists('ext/mgpt/parser.cc') or not os.path.exists(
            'ext/mgpt/parser.h'):
        recompile_parser = True
    else:
        parser_mtime = getModificationTime('ext/mgpt/parser.y')
        if (parser_mtime > getModificationTime('ext/mgpt/parser.h')
                or parser_mtime > getModificationTime('ext/mgpt/parser.cc')):
            recompile_parser = True
    if recompile_parser:
        retcode, stdout, stderr = runCmd(['yacc', '-d', 'ext/mgpt/parser.y'])
        if retcode != 0:
            logging.error('Error recompiling parser:\n%s' % stderr)
            return False
        if os.path.exists('ext/mgpt/parser.cc'):
            os.remove('ext/mgpt/parser.cc')
        os.rename('y.tab.c', 'parser.cc')
        shutil.move('parser.cc', 'ext/mgpt/')

        if os.path.exists('ext/mgpt/parser.h'):
            os.remove('ext/mgpt/parser.h')
        os.rename('y.tab.h', 'parser.h')
        shutil.move('parser.h', 'ext/mgpt/')
    return True


def phase2FindObjectsForLinkage(main_obj, deps):
    stack = [main_obj]
    # invariant: every time an item is pushed in the stack, it is also added in
    # the objs_needed_for_linking
    objs_needed_for_linking = [main_obj]
    while stack:
        i = stack.pop()
        # logging.debug('Processing %s' % i)
        if i not in deps:
            # logging.debug('%s has no dependency' % i)
            continue
        for prec in deps[i]:
            p = prec
            assert os.path.exists(p), \
                "%s depends on %s but the latter doesn't exists" % (i, p)
            if prec[-2:] == '.h':
                p = prec[:-2]
                if os.path.exists(p + '.cc') or os.path.exists(p + '.c'):
                    p = p + '.o'
                else:
                    # header only
                    continue
            elif prec[-2:] == '.c':
                p = prec[:-2] + '.o'
            elif prec[-3:] == '.cc':
                p = prec[:-3] + '.o'
            if p not in objs_needed_for_linking:
                # logging.debug("%s depends on %s (and it was added to stack)" % (i, p))
                stack.insert(0, p)
                objs_needed_for_linking.insert(0, p)

            # else:
            #     logging.debug("%s depends on %s (and it was already scheduled)" % (i, p))
    return objs_needed_for_linking


def compileIfNeeded(args):
    obj, deps, rules, objdir = args
    obj_fullpath = "%s/%s" % (objdir, obj)
    need_to_compile = False
    if not os.path.exists(obj_fullpath):
        need_to_compile = True
    else:
        obj_mtime = getModificationTime(obj_fullpath)
        for prec in deps[obj]:
            prec_mtime = getModificationTime(prec)
            if obj_mtime < prec_mtime:
                #                 print "%s is older than %s" % (obj, prec)
                need_to_compile = True

    if need_to_compile:
        logging.info('Compiling %s' % obj)
        rv = generateObject(obj, rules, objdir)
        return (True, obj_fullpath, rv)
    else:
        logging.debug('Reusing %s' % obj)
        return (False, obj_fullpath, None)


def phase3Compiling(objs_needed_for_linking,
                    deps,
                    rules,
                    objdir,
                    n_threads=None):
    compiled_all = True
    compiled_some = False
    stderr = []

    global DEBUG_OBJS_TO_COMPILE
    DEBUG_OBJS_TO_COMPILE = objs_needed_for_linking

    arg_lists = [(x, deps, rules, objdir) for x in objs_needed_for_linking]
    with multiprocessing.Pool(n_threads) as pool:
        for rv in pool.imap_unordered(compileIfNeeded, arg_lists):
            was_compiled, obj_fullpath, gcc_rv = rv
            # logging.debug('callbackCompiling called with {}'.format(rv))
            if was_compiled:
                # This target was compiled
                compiled_some = True
                if not gcc_rv or (isinstance(gcc_rv, tuple)
                                  and gcc_rv[0] != 0):
                    compiled_all = False
                    stderr.append(gcc_rv[2])
                else:
                    # Successfully compiled
                    if len(gcc_rv[1]) > 0:
                        logging.info(
                            'Object "%s" compiled with success. Compiler stdout:\n%s'
                            % (obj_fullpath, gcc_rv[1]))
                    if len(gcc_rv[2]) > 0:
                        logging.info(
                            'Object "%s" compiled with success. Compiler stderr:\n%s'
                            % (obj_fullpath, gcc_rv[2]))
                    if not os.path.exists(obj_fullpath):
                        logging.error(
                            'Obj "%s" was compiled but could not be found... Python bug?'
                            % obj_fullpath)
                        compiled_all = False
                        stderr.append('Not a gcc issue')

    if not compiled_all:
        logging.error(
            "GCC failed to compile one or more files and returned:\n%s" %
            "\n".join(stderr))
        return -1
    elif compiled_some:
        return 1
    else:
        return 0


def phase4LinkTarget(objs_needed_for_linking, result_name, cpp_compiler,
                     linker_flags, objdir):
    link_cmd = [cpp_compiler, '-o', result_name] \
        + ["%s/%s" % (objdir, obj) for obj in objs_needed_for_linking] \
        + linker_flags
    retcode, stdout, stderr = runCmd(link_cmd)
    if retcode != 0:
        logging.error("gcc returned '%d' when linking. Error message:\n%s" %
                      (retcode, stderr))
        logging.debug("objects need to compile: [%s]" %
                      ", ".join(DEBUG_OBJS_TO_COMPILE))
        return False
    return True


def findAllSourceFiles(src_dir):
    all_files = []
    for root, dirnames, filenames in os.walk(src_dir):
        if '.git_ignore_me' in root:
            continue
        if root == '.':
            all_files += [
                f for f in filenames if f[-2:] == '.c' or f[-3:] == '.cc'
            ]
        else:
            assert root[0:2] == './'
            all_files += [
                "%s/%s" % (root[2:], f) for f in filenames
                if f[-2:] == '.c' or f[-3:] == '.cc'
            ]


#    print all_files
    return all_files


def objNameFromSrcName(src_file):
    base = src_file[:-1]  # Remove 'h' and 'c' from .h, .c, and .cc
    if base[-1] == 'c':  # was .cc
        base = base[:-1]
    return base + 'o'


def cleanPath(path_file):
    """ Clean paths such as 'a/b/../c/../d' to 'a/d'

        Useful because gcc -MM generates a lot of weird paths like that
    """
    path_parts = path_file.split('/')
    clean_path = []
    for i in path_parts:
        if i == '.':
            continue
        elif i == '..':
            clean_path.pop()
        else:
            clean_path.append(i)

    # print "cleanPath: %s -> %s" % (path_file, '/'.join(clean_path))
    return '/'.join(clean_path)


def generateDependency(src, flags):
    compiler = C_COMPILER
    if src[-3:] == '.cc':
        compiler = CPP_COMPILER
    cmd = [compiler, '-MM'] + flags + [src]
    retcode, make_rule, stderr = runCmd(cmd)
    if retcode != 0:
        return (False, (retcode, make_rule, stderr))

    # logging.debug('Makefile rule for "%s": %s' % (src, make_rule))
    parts = make_rule.split()
    deps = set()
    # Ignoring the head of the rule, i.e., the .o file
    for i in parts[1:]:
        if i == '\\':
            continue
        deps.add(cleanPath(i))
    return (True, list(deps))


def generateDependencyWrapper(src, obj, flags):
    success, remaning_rv = generateDependency(src, flags)
    return (success, obj, remaning_rv)


def compileAndLinkWithAutoDependencies(main_obj,
                                       result_name,
                                       rules,
                                       deps_file,
                                       cpp_compiler,
                                       extra_cflags,
                                       linker_flags,
                                       objdir='.',
                                       n_threads=None,
                                       allowed_to_recompile_parser=True):
    # FWT: compiled_all cannot be a basic variable because it will loose scope.
    # a reference should be used instead
    status = {'deps_changed': False, 'found_error': False, 'stderr': []}
    deps = {}

    logging.info('Finding the dependencies for "%s"' % main_obj)
    deps_mtime = 0
    if os.path.exists(deps_file):
        deps_mtime = getModificationTime(deps_file)
        with open(deps_file) as data_file:
            deps = json.load(data_file)

    all_srcs = findAllSourceFiles('.')
    pool_jobs = []
    for src in all_srcs:
        obj = objNameFromSrcName(src)
        if obj not in deps or getModificationTime(src) > deps_mtime:
            deps[obj] = None
            pool_jobs.append((
                src,
                obj,
                extra_cflags,
            ))

    with multiprocessing.Pool(n_threads) as pool:
        for rv in pool.starmap(generateDependencyWrapper, pool_jobs):
            success, obj, dep_or_error_msg = rv
            if success:
                # This target was compiled
                deps[obj] = dep_or_error_msg
                logging.debug('New dependency for "%s": [%s]' %
                              (obj, ", ".join(dep_or_error_msg)))
                status['deps_changed'] = True
            else:
                status['found_error'] = True
                status['stderr'].append(dep_or_error_msg[2])
                pool.close()
                pool.terminate()
                break

    if status['found_error']:
        logging.error(
            'gcc found error while pre-processing for auto-dep:\n%s' %
            "\n".join(status['stderr']))
        logging.error('Giving up')
        return -1

    if status['deps_changed']:
        with open(deps_file, 'w') as data_file:
            json.dump(deps, data_file, separators=(',', ': '), indent=2)

    logging.info('Dependency generation done.')
    return compileAndLinkWithDependencies(main_obj, result_name, deps, rules,
                                          cpp_compiler, linker_flags, objdir,
                                          n_threads,
                                          allowed_to_recompile_parser)


# Return values:
#  -1: Error during compilation or linking
#   0: Nothing changed in the target, i.e., no compilation neither linking was
#      performed
#   1: Something changed and the target was compiled and linked with SUCCESS
def compileAndLinkWithDependencies(main_obj,
                                   result_name,
                                   deps,
                                   rules,
                                   cpp_compiler,
                                   linker_flags,
                                   objdir='.',
                                   n_threads=1,
                                   allowed_to_recompile_parser=True):
    # Phase 1: parser and lexer force for all binaries :(
    if not phase1Parser(allowed_to_recompile_parser):
        logging.error('Failed while generating the parser/lexer. Giving up')
        return -1

    # Phase 2: generating list of files that need to be compiled
    objs_needed_for_linking = phase2FindObjectsForLinkage(main_obj, deps)
    # TODO: HACK:
    if "ext/mgpt/problems.o" in objs_needed_for_linking:
        for forced_dep in ['ext/mgpt/parser.o', 'ext/mgpt/lexer.o']:
            if forced_dep not in objs_needed_for_linking:
                objs_needed_for_linking.append(forced_dep)

    logging.info("Total objects needed for the target: %d" %
                 len(objs_needed_for_linking))
    logging.debug('Objects needed: %s' %
                  " ".join(sorted(objs_needed_for_linking)))

    # Phase 3: Compiling
    phase3_rv = phase3Compiling(objs_needed_for_linking, deps, rules, objdir,
                                n_threads)
    if phase3_rv == -1:
        logging.error("Failed to compile one or more objects. Giving up")
        return -1
    elif phase3_rv == 0:
        logging.info(
            "Nothing was compiled for this target. Skipping linking phase")
        return 0

    # Phase 4: link together the necessary files
    logging.info('Linking binary for the giving target')
    if not phase4LinkTarget(objs_needed_for_linking, result_name, cpp_compiler,
                            linker_flags, objdir):
        logging.error("Fail linking '%s'. Giving Up" % result_name)
        return -1

    logging.info('Done linking binary')
    return 1


class PyBindTarget:
    def __init__(self, rules, deps_file, cpp_compiler, extra_cflags,
                 orig_linker_flags, objdir, n_threads,
                 allowed_to_recompile_parser):
        linker_flags = ['-shared'] + orig_linker_flags
        self.build_args = (rules, deps_file, cpp_compiler, extra_cflags,
                           linker_flags, objdir, n_threads,
                           allowed_to_recompile_parser)

    def __call__(self):
        # suffix code copied from python-config
        ext_suffix = sysconfig.get_config_var('EXT_SUFFIX')
        if ext_suffix is None:
            ext_suffix = sysconfig.get_config_var('SO')
        pybind_obj = 'python/pyssipp.o'
        pybind_so = 'ssipp' + ext_suffix
        # this is useful for setup.py script
        print('Extension: %s' % os.path.join(os.getcwd(), pybind_so))
        return compileAndLinkWithAutoDependencies(pybind_obj, pybind_so,
                                                  *self.build_args)


def clean(objdir):
    if os.path.exists(objdir):
        shutil.rmtree(objdir)
        os.makedirs(objdir, exist_ok=True)
    return 1


if not hasattr(__builtins__, 'apply'):

    def apply(func, *args, **kwargs):
        """Partial application for Python 3"""

        @functools.wraps(func)
        def wrapper(*new_args, **new_kwargs):
            all_args = args + new_args
            all_kwargs = dict(kwargs)
            all_kwargs.update(new_kwargs)
            return func(*all_args, **all_kwargs)

        return wrapper


# python_{cflags,ldflags} code taken from python-config
def python_cflags(debug=False):
    flags = [
        '-I' + sysconfig.get_path('include'),
        '-I' + sysconfig.get_path('platinclude')
    ]
    flags.extend(sysconfig.get_config_var('CFLAGS').split())
    # (1) -Wstrict-prototypes does not help for C++
    # (2) -O{0,g,1,2} may conflict with our own optimisations (if included
    #     after our own flags)
    disallowed = ['-Wstrict-prototypes', '-O2', '-O1', '-O0', '-Og']
    if debug:
        # -DNDEBUG conflicts with our local usage of NDEBUG
        disallowed += ['-DNDEBUG']
    for bad_flag in disallowed:
        if bad_flag in flags:
            flags.remove(bad_flag)
    return flags


def python_ldflags():
    abiflags = getattr(sys, 'abiflags', '')
    libs = ['-lpython' + sysconfig.get_config_var('VERSION') + abiflags]
    libs += sysconfig.get_config_var('LIBS').split()
    libs += sysconfig.get_config_var('SYSLIBS').split()
    if not sysconfig.get_config_var('Py_ENABLE_SHARED'):
        libs.insert(0, '-L' + sysconfig.get_config_var('LIBPL'))
    if not sysconfig.get_config_var('PYTHONFRAMEWORK'):
        libs.extend(sysconfig.get_config_var('LINKFORSHARED').split())
    return libs


class Rule(object):
    def __init__(self):
        pass

    def isApplicable(self, obj):
        return None

    def apply(self, obj, objdir):
        return None


class GenericRule(Rule):
    def __init__(self, compiler, source_extension, flags):
        self.compiler = compiler
        self.source_extension = source_extension
        self.flags = flags

    def isApplicable(self, obj):
        root = obj[:-2]
        if obj[-2:] == '.o' and os.path.exists(root + self.source_extension):
            return True
        return False

    def apply(self, obj, objdir):
        source = "%s%s" % (obj[:-2], self.source_extension)
        cmd = [self.compiler] + self.flags + [
            '-c', source, '-o', "%s/%s" % (objdir, obj)
        ]
        return runCmd(cmd)


class FileSpecificRule(Rule):
    def __init__(self, target, source, compiler, flags):
        self.target = target
        self.source = source
        self.compiler = compiler
        self.flags = flags

    def isApplicable(self, obj):
        return self.target == obj and os.path.exists(self.source)

    def apply(self, obj, objdir):
        cmd = [self.compiler] + self.flags + [
            '-c', self.source, '-o',
            "%s/%s" % (objdir, obj)
        ]
        return runCmd(cmd)


class AddGitFlag(Rule):
    def __init__(self, rule):
        self.rule = rule

    def isApplicable(self, obj):
        return self.rule.isApplicable(obj)

    def apply(self, obj, objdir):
        git_hash = getClosestHashRef()
        self.rule.flags.append('-DGIT_HASH="%s"' % git_hash)
        return self.rule.apply(obj, objdir)


def getClosestHashRef():
    def runGitCmdWrapper(cmd):
        rv, stdout, stderr = runCmd(cmd)
        if rv != 0:
            logging.warning('%s returned "%d":\n%s' % (cmd, rv, stderr))
            return None
        return stdout

    logging.info('Finding closest git sha1 to current code')
    this_branch_name = runGitCmdWrapper("git rev-parse --abbrev-ref HEAD")
    if this_branch_name is None:
        return "UNKNOWN"

    rv, stdout, stderr = runCmd('git rev-parse "refs/wip/%s"' %
                                this_branch_name)
    last_git_wip_name = stdout
    if rv == 0:
        timestamp_this_branch = runGitCmdWrapper("git show -s --format=%ct " +
                                                 this_branch_name)
        if timestamp_this_branch is None:
            return "UNKNOWN"
        else:
            timestamp_this_branch = int(timestamp_this_branch)

        timestamp_wip = runGitCmdWrapper("git show -s --format=%ct " +
                                         last_git_wip_name)
        if timestamp_wip is None:
            return "UNKNOWN"
        else:
            timestamp_wip = int(timestamp_this_branch)

        if timestamp_this_branch < timestamp_wip:
            wip_sha1 = runGitCmdWrapper("git rev-parse %s" % last_git_wip_name)
            if wip_sha1 is None:
                return "UNKNOWN"
            else:
                return wip_sha1

    head_sha1 = runGitCmdWrapper("git rev-parse %s" % this_branch_name)
    if head_sha1 is None:
        return "UNKNOWN"
    else:
        return head_sha1


def main():
    parser = argparse.ArgumentParser(
        description='Build the code (Makefile substitute)')
    parser.add_argument(
        '--keep_parser',
        '-k',
        action='store_true',
        help='keep the current parser, i.e., don\'t call lex and yacc')
    parser.add_argument('--no-opt',
                        action='store_false',
                        dest='opt',
                        help='turn off optimization flags')
    parser.add_argument('--debug',
                        action='store_true',
                        help='turn on debug flags')
    parser.add_argument(
        # none = let Python decide
        '--jobs',
        '-j',
        type=int,
        default=None,
        help='number of parallel jobs')
    parser.add_argument(
        '-l',
        '--log',
        dest='logLevel',
        choices=['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'],
        help="Set the logging level")
    parser.add_argument('-v',
                        '--verbose',
                        help="Print debugging statements",
                        action="store_const",
                        dest="loglevel",
                        const=logging.DEBUG,
                        default=logging.INFO)
    parser.add_argument('-c',
                        '--c-compiler',
                        help="C compiler",
                        type=str,
                        default='gcc',
                        dest='c_compiler')
    parser.add_argument('-p',
                        '--cpp-compiler',
                        help="C++ compiler",
                        type=str,
                        default='g++',
                        dest='cpp_compiler')
    parser.add_argument(
        '-f',
        '--flag',
        help='Add the following flag to the C++ compiler. Example: '
        '-f="--pedantic -DMacroVal=3"',
        type=str,
        default=None,
        dest='cpp_flags')
    parser.add_argument(
        '-i',
        '--incremental',
        action='store_true',
        default=False,
        help="keep temp build directory around instead of deleting; good for "
        "incremental re-compilation, but can lead to confusing results when"
        "building targets with different CFLAGS")

    parser.add_argument('targets',
                        type=str,
                        nargs='+',
                        help='target to be processed linearly')
    args = parser.parse_args()

    if args.logLevel:
        log_format = '[%(levelname)s %(funcName)s::%(lineno)d] %(message)s'
        logging.basicConfig(level=getattr(logging, args.logLevel),
                            format=log_format)
    else:
        log_format = '[%(levelname)s] %(message)s'
        logging.basicConfig(level=logging.INFO, format=log_format)

    n_threads = args.jobs

    hostname = socket.gethostname()

    global C_COMPILER
    C_COMPILER = args.c_compiler
    c_flags = ['-Wall', '-march=native', '-fno-lto']

    global CPP_COMPILER
    CPP_COMPILER = args.cpp_compiler

    cpp_flags = [
        '-Wall', '-DATOM_STATES', '-march=native', '-std=c++14', '-I./',
        '-fdiagnostics-color=always', '-fno-lto'
    ]
    if args.cpp_flags is not None:
        cpp_cli_flags = args.cpp_flags.strip().split()
        logging.info("Adding the following C++ Flags: %s" % str(cpp_cli_flags))
        cpp_flags += cpp_cli_flags

    if args.debug:
        debug_flags = ['-ggdb', '-DDIE_WITH_ASSERT']
        cpp_flags += debug_flags
        c_flags += debug_flags
    else:
        no_debug_flags = ['-DNDEBUG']
        cpp_flags += no_debug_flags
        c_flags += no_debug_flags

    if args.opt:
        opt_flags = ['-ffast-math', '-O3']
        cpp_flags += opt_flags
        c_flags += opt_flags

    allowed_to_recompile_parser = True
    if args.keep_parser:
        allowed_to_recompile_parser = False

    # Host personalization
    if 'cs.cmu.edu' in hostname:
        logging.info("Host personalization for CMU (hostname = %s)" % hostname)
        allowed_to_recompile_parser = False
    elif hostname == 'cluster2.it.nicta.com.au)':
        logging.info("Host personalizations for NICTA (hostname = %s)" %
                     hostname)
        allowed_to_recompile_parser = False
        idx = c_flags.index('-march=native')
        c_flags[idx] = '-m64'
        logging.info(" - Replacing '-march=native' with '-m64'")
    elif 'helix' in hostname:
        logging.info("Host Personalization for helix:")
        if C_COMPILER == 'gcc':
            logging.info(' - Using gcc-4.9')
            C_COMPILER = 'gcc-4.9'
        if CPP_COMPILER == 'g++':
            logging.info(' - Using g++-4.9')
            CPP_COMPILER = 'g++-4.9'
        if n_threads == 0:
            logging.info(' - Using n_threads = 3')
            n_threads = 3
    else:
        logging.debug("No host personalization")

    # Sam: -fno-lto possibly gets around a GCC7 bug, per this Github post:
    # https://github.com/halide/Halide/issues/2713
    # (not sure whether this happens on newer GCC)
    linker_flags = ['-pthread', '-lm', '-fno-lto']

    logging.info('Using cpp_flags = %s' % str(cpp_flags))
    logging.info('Using linker_flags = %s' % str(linker_flags))
    used_cpp_flags = ' '.join(cpp_flags)

    solver_cflags = [
        '-DCFLAGS_USED="%s"' % used_cpp_flags,
        '-DHOSTNAME="%s"' % hostname
    ] + cpp_flags
    solver_rules = [
        AddGitFlag(
            FileSpecificRule('solver_ssp.o', 'solver_ssp.cc', CPP_COMPILER,
                             solver_cflags)),
        GenericRule(CPP_COMPILER, '.cc', cpp_flags),
    ]

    tmpdir = "/tmp/ssipp_build_tmp/"
    os.makedirs(tmpdir, exist_ok=True)
    objdir = "%s/objs" % tmpdir
    deps_file = "%s/deps.json" % tmpdir

    def clean_target():
        rv = clean(tmpdir)
        artefact_globs = [
            'solver_ssp', 'heur_eval', 'ssipp*.so', 'build', 'ssipp.egg-info',
            tmpdir
        ]
        for artefact_glob in artefact_globs:
            # should follow EAFP in theory, but this is easier than looking at
            # exception cases
            matches = glob.glob(artefact_glob)
            if len(matches) > 1:
                # not expecting this, but maybe it doesn't matter
                raise IOError(
                    "expected only one match for '%s', got matches %s" %
                    (artefact_glob, matches))
            if not matches:
                continue
            artefact, = matches
            if os.path.exists(artefact):
                if os.path.isdir(artefact):
                    shutil.rmtree(artefact)
                else:
                    os.unlink(artefact)
        return rv

    targets = {
        'solver_ssp':
        apply(compileAndLinkWithAutoDependencies, 'solver_ssp.o', 'solver_ssp',
              solver_rules, deps_file, CPP_COMPILER, solver_cflags,
              linker_flags, objdir, n_threads, allowed_to_recompile_parser),
        'heur_eval':
        apply(compileAndLinkWithAutoDependencies, 'heur_eval.o', 'heur_eval',
              solver_rules, deps_file, CPP_COMPILER, solver_cflags,
              linker_flags, objdir, n_threads, allowed_to_recompile_parser),
        'clean':
        clean_target,
    }

    this_dir = os.path.dirname(os.path.abspath(__file__))
    pybind11_path = os.path.join(this_dir, 'vendor/pybind11-2.2.4/include/')
    pybind_cflags \
        = ['-fPIC', '-I' + pybind11_path, '-DSSIPP_NO_SIGNAL_MANAGER'] \
        + python_cflags(debug=args.debug) + cpp_flags
    pybind_rules = [GenericRule(CPP_COMPILER, '.cc', pybind_cflags)]
    targets['pybind'] = PyBindTarget(
        pybind_rules, deps_file, CPP_COMPILER, pybind_cflags,
        python_ldflags() + ['-shared'] + linker_flags, objdir, n_threads,
        allowed_to_recompile_parser)

    # Aliases
    targets['ssp'] = targets['solver_ssp']

    for t in args.targets:
        logging.info('Processing target "%s"' % t)
        if t not in targets:
            logging.error('Unknown target "%s"' % t)
            break
        target_rule_rv = True
        if isinstance(targets[t], list) or isinstance(targets[t], tuple):
            for sub_rule in targets[t]:
                target_rule_rv = sub_rule()
                if target_rule_rv < 0:
                    break
        else:
            target_rule_rv = targets[t]()

        if target_rule_rv < 0:
            logging.error('Failed at target "%s"' % t)
            sys.exit(1)
        logging.info('Target "%s" built successfully' % t)

    if not args.incremental:
        logging.info("Removing temporary directory at '%s'" % tmpdir)
        shutil.rmtree(tmpdir, ignore_errors=True)


if __name__ == "__main__":
    main()
