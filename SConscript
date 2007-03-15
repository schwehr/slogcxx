Import('env')

incs = '''
src/slogcxx.h
src/slogcxx-nlog.h
'''

env.Install(env['inc_dir'],Split(incs))

sources = '''
src/slogcxx.cpp
src/slogcxx-test.cpp
'''

objs = env.Object(Split(sources))

Return('objs')
