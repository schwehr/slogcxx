# Cheap hack to get going.

CXXFLAGS:= -g3 -Wall  -Wno-deprecated
CXXFLAGS += -Wimplicit 
CXXFLAGS += -pedantic 
CXXFLAGS += -W 
CXXFLAGS += -Wredundant-decls
CXXFLAGS += -Werror

CPPFLAGS := -I../src
LDFLAGS := ../src/slogcxx.o


targets: simplest

clean:
	rm -f *.o *.a *.log foo* *-test

real-clean: clean
	rm -rf html .gdb_history


