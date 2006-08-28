Import('env')

incs = '''
src/slogcxx.h
src/slogcxx-nlog.h
'''

for i in Split(incs):
        env.Install('#include',i)

sources = '''
src/slogcxx.cpp
src/slogcxx-test.cpp
'''

objs = []

for s in Split(sources):
        objs.append(env.Object(s))

Return('objs')
