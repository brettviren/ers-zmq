#!/usr/bin/env waf
import os.path as osp
from waflib.Tools import waf_unit_test
from subprocess import check_output
from waflib.Utils import subst_vars

VERSION='0.0.0'
APPNAME='ers-zmq'


def import_scanner(task):
    deps = []
    for node in task.inputs:
        cmd = "${MOO} imports %s" % node.abspath()
        cmd = subst_vars(cmd, task.env)
        # out = task.exec_command(cmd)
        out = check_output(cmd.split()).decode()
        deps += out.split("\n")

    deps = [task.generator.bld.path.find_or_declare(d) for d in deps if d]
    print(deps)
    return (deps, [])


def options(opt):
    opt.load('compiler_cxx')
    opt.load('waf_unit_test')
    opt.add_option('--quell-tests', action='store_true', default=False,
                   help='Compile but do not run the tests (default=%default)')
    opt.add_option('--with-ers', default=None,
                   help='Set to ERS install area')
    opt.add_option('--with-libzmq', default=None,
                   help='Set to libzmq install area')
    opt.add_option('--with-nljs', default=None,
                   help='Point nlohmann json install area')


def configure(cfg):
    cfg.load('compiler_cxx')
    cfg.load('waf_unit_test')

    cfg.find_program('moo', var='MOO', mandatory=False)

    cfg.env.CXXFLAGS += ['-std=c++17', '-ggdb3', '-Wall', '-Werror']

    # nlohmann::json
    nljs = getattr(cfg.options, 'with_nljs', None)
    if nljs:
        print("using " + nljs)
        setattr(cfg.env, 'INCLUDES_NLJS', [osp.join(nljs, "include")])

    cfg.check(features='cxx cxxprogram', define_name='HAVE_NLJS',
              header_name='nlohmann/json.hpp',
              use='NLJS', uselib_store='NLJS', mandatory=True)

    # ERS
    ers = getattr(cfg.options, 'with_ers', None)
    if ers:
        setattr(cfg.env, 'RPATH_ERS', [osp.join(ers, 'lib')])
        setattr(cfg.env, 'LIBPATH_ERS', [osp.join(ers, 'lib')])
        setattr(cfg.env, 'INCLUDES_ERS', [osp.join(ers, 'include')])
    cfg.check(features='cxx cxxprogram', define_name='HAVE_ERS',
              header='ers/ers.h', lib=['ers', 'ErsBaseStreams'],
              use='ERS', uselib_store='ERS', mandatory=True)

    # libzmq
    zmq = getattr(cfg.options, "with_libzmq", None)
    if zmq:
        setattr(cfg.env, 'RPATH_LIBZMQ', [osp.join(zmq, 'lib')])
        setattr(cfg.env, 'LIBPATH_LIBZMQ', [osp.join(zmq, 'lib')])
        setattr(cfg.env, 'INCLUDES_LIBZMQ', [osp.join(zmq, 'include')])
    cfg.check_cfg(package='libzmq', uselib_store='ZMQ',
                  mandatory=True, args='--cflags --libs')


def build(bld):

    use = ['ERS', 'ZMQ', 'NLJS']

    if "MOO" in bld.env:
        print("regenerate")
        model = bld.path.find_resource("src/ez-model.jsonnet")
        srcdir = model.parent
        struct = srcdir.make_node("Structs.hpp")
        nljs = srcdir.make_node("Nljs.hpp")
        cmd = "${MOO} render -o ${TGT} ${SRC}"
        bld(rule=cmd + " ostructs.hpp.j2",
            source=model, target=struct,
            scan=import_scanner)
        bld(rule=cmd + " onljs.hpp.j2",
            source=model, target=nljs,
            scan=import_scanner)

    sources = bld.path.ant_glob('src/*.cpp')
    bld.shlib(features='cxx',
              source=sources, target='ers-zmq',
              uselib_store='ERS_ZMQ', use=use)

    bld.add_post_fun(waf_unit_test.summary)

