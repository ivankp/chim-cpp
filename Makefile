.PHONY: all clean

ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean))) ########################

EXE := chim test

CPPSTD := c++23

CFLAGS := -Wall -Wextra -Wpedantic -fmax-errors=3
# CFLAGS := -Og -g # Debug
CFLAGS := -O3 -flto -march=native -DNDEBUG # Release
CFLAGS += -Isrc

LDFLAGS :=
LDLIBS :=

.PRECIOUS: .build/%.o

EXE := $(addprefix bin/,$(EXE))

all: $(EXE)

bin/%: .build/%.o
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

.build/%.o: src/%.cc
	@mkdir -p $(@D)
	$(CXX) -std=$(CPPSTD) $(CFLAGS) -c $< -o $@

.build/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

bin/chim: .build/Mmapped.o .build/PosixFile.o .build/parser.o .build/config.o

.build/Mmapped.o: src/Mmapped.hh src/Error.hh src/strings.hh
.build/PosixFile.o: src/strings.hh
.build/parser.o: src/parser.hh src/Mmapped.hh src/Error.hh src/strings.hh \
    src/config.hh src/PosixFile.hh
.build/config.o: src/config.hh src/PosixFile.hh

endif ##########################################################################

clean:
	@rm -frv .build bin
