.PHONY: all clean

ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean))) ########################

EXE := bin/chim bin/test

CPPSTD := c++23

CFLAGS := -Wall -Wextra -Wpedantic -fmax-errors=3
# CFLAGS := -Og -g # Debug
CFLAGS := -O3 -flto -march=native -DNDEBUG # Release
CFLAGS += -Isrc

LDFLAGS :=
LDLIBS :=

.PRECIOUS: .build/%.o

all: $(EXE)

bin/%: .build/%.o
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

.build/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(CXX) -std=$(CPPSTD) $(CFLAGS) -c $< -o $@

.build/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

bin/chim: .build/Mmapped.o .build/parser.o

.build/Mmapped.o: src/Mmapped.hpp src/Error.hpp src/strings.hpp
.build/parser.o: src/parser.hpp src/Mmapped.hpp src/Error.hpp src/strings.hpp

endif ##########################################################################

clean:
	@rm -frv .build bin
