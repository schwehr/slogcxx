# Cheap hack to get going.
SHELL:=bash

#CXXFLAGS:= -g3
CXX_WFLAGS := -Wall  -Wno-deprecated
CXX_WFLAGS += -Wimplicit 
CXX_WFLAGS += -pedantic 
CXX_WFLAGS += -W 
CXX_WFLAGS += -Wredundant-decls
CXX_WFLAGS += -Werror

CXX_OPT_FLAGS  :=-funroll-loops -fexpensive-optimizations -DNDEBUG -fexpensive-optimizations -O3 -DNDEBUG -ffast-math

slogcxx-test: #slogcxx-test.cpp slogcxx.o
	scons slogcxx-test
	./slogcxx-test 2> test-stderr.log
	doxygen
#	g++ -o $@ $^ ${CXXFLAGS} -I.

slogcxx-nolog-test:
	make clean
	g++ -DNLOG slogcxx-test.cpp -I. -o $@ ${CXX_WFLAGS} ${CXX_OPT_FLAGS}

clean:
	rm -f *.o *.a *.log foo* *-test
#	scons -c

real-clean: clean
	rm -rf html .gdb_history a.out


