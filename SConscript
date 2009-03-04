Import('env')
import os

incs = '''
src/slogcxx.h
src/slogcxx-nlog.h
'''

env.Install(os.path.join(env['install'],'include'),Split(incs))

sources = '''
src/slogcxx.cpp
'''
#src/slogcxx-test.cpp

objs = env.Object(Split(sources))

Return('objs')
