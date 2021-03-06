CXX ?= g++
CXXFLAGS ?= -Wall -Wextra -ansi -pedantic -pipe -O3 -march=native -mtune=native -DNDEBUG -Wno-long-long
LDFLAGS ?= -Wl,-O1 -s -lrt
EXECUTABLE = bench
LINK.o = $(LINK.cc)
SOURCES = \
	evaluator/evaluator_internal/transition_table.cpp \
	evaluator/evaluator_internal/jit/common.cpp \
	evaluator/evaluator_internal/jit/func_templates.cpp \
	evaluator/evaluator_internal/jit/oper_templates.cpp \
	evaluator/evaluator_internal/jit/real_templates.cpp \
	benchmark.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(SOURCES) $(EXECUTABLE)

.PHONY: clean distclean

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)

distclean: clean
	rm -f $(EXECUTABLE)
