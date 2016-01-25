CXX ?= g++
CXXFLAGS ?= -Wall -Wextra -ansi -pedantic -pipe -O3 -march=native -mtune=native -DNDEBUG
LDFLAGS ?= -s -lrt
EXECUTABLE = bench
LINK.o = $(LINK.cc)
SOURCES = \
	evaluator_internal/transition_table.cpp \
	evaluator_internal/jit/common.cpp \
	evaluator_internal/jit/func_templates.cpp \
	evaluator_internal/jit/oper_templates.cpp \
	evaluator_internal/jit/real_templates.cpp \
	benchmark.cpp
OBJECTS = $(SOURCES:.c=.o)

OBJECTS = $(SOURCES:.cpp=.o)

all: $(SOURCES) $(EXECUTABLE)

.PHONY: clean distclean

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)

distclean: clean
	rm -f $(EXECUTABLE)
