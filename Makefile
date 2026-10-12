.PHONY: all clean

ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean))) ########################

EXE := chim test

CPPSTD := c++23

CFLAGS := -Wall -Wextra -Wpedantic -fmax-errors=3
# CFLAGS := -Og -g # Debug
CFLAGS += -O3 -flto -march=native -DNDEBUG # Release
CFLAGS += -Isrc

DEPFLAGS := -MMD -MP

LDFLAGS :=
LDLIBS :=

.PRECIOUS: .build/%.o

all: $(addprefix bin/,$(EXE))

bin/%: .build/%.o | bin
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

.build/%.o: src/%.cc | .build
	$(CXX) -std=$(CPPSTD) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

.build/%.o: src/%.c | .build
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

bin .build:
	mkdir -p $@

-include $(wildcard .build/*.d)

bin/chim: .build/file_posix.o .build/config.o .build/parser.o

endif ##########################################################################

clean:
	@rm -frv .build bin
