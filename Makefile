# Byak, a UCI chess engine.
# Copyright (C) 2013  Sylvain Philip
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

CC := gcc
EXE := byak
TEST_EXE := $(EXE)tests
CFLAGS = -std=c11 -I./src
LDLIBS = -lpthread
LDFLAGS =
WARN = -Wall
OPTI = -O3 -flto

ifneq ($(findstring win, $(TARGET) $(PLATFORM)),)
	EXE := $(EXE).exe
	TEST_EXE := $(TEST_EXE).exe
	LDLIBS =
endif

ifeq ($(TARGET), win32)
	# Produce WIN 32 bits exe from Linux
	CC := i686-w64-mingw32-gcc
endif

ifeq ($(TARGET), win64)
	# Produce WIN 64 bits exe from Linux
	CC :=x86_64-w64-mingw32-gcc
endif

ifeq ($(PLATFORM), win32)
	# Produce WIN 32 bits exe from Windows
	CC := mingw32-gcc.exe
endif

ifeq ($(PLATFORM), win64)
	# Produce WIN 64 bits exe from Windows
	CC := mingw64-gcc.exe
endif

ifeq ($(DEBUG), 1)
	# Add GDB symbols
	CFLAGS += -g -DDEBUG -Wno-unused-function
	OPTI =
endif

ifeq ($(DEBUG_PROFILE), 1)
	# Add Gprof support
	CFLAGS += -pg -DDEBUG -Wno-unused-function
	LDFLAGS += -pg
	OPTI =
endif

ifeq ($(DEBUG_COVERAGE), 1)
	# Add code coverage for gcov
	CFLAGS += -fprofile-arcs -ftest-coverage -DDEBUG
	LDFLAGS += -fprofile-arcs -ftest-coverage
	OPTI =
endif

ifeq ($(USE_INLINING), 1)
	# Force inlining (Enabled by default in non-debugging mode)
	CFLAGS += -DUSE_INLINING
endif


COMMON_SRC  := $(filter-out src/main.c test/main.c, $(wildcard src/*.c))
COMMON_OBJ  := $(COMMON_SRC:src/%.c=build/%.o)

.PHONY: clean coverage

all : build/$(EXE)

tests : build/$(TEST_EXE)
	./build/$(TEST_EXE)

build/$(EXE) : build/main.o $(COMMON_OBJ) 
	$(CC) -o $@ $(WARN) $(OPTI) build/main.o $(COMMON_OBJ) $(LDFLAGS) $(LDLIBS)

build/$(TEST_EXE) : build/maintests.o $(COMMON_OBJ) 
	$(CC) -o $@ $(WARN) $(OPTI) build/maintests.o $(COMMON_OBJ) $(LDFLAGS) $(LDLIBS)

build/main.o :
	$(CC) -c $(WARN) $(OPTI) $(CFLAGS) src/main.c -o build/main.o

build/maintests.o :
	$(CC) -c $(WARN) $(OPTI) $(CFLAGS) tests/main.c -o build/maintests.o

$(COMMON_OBJ) : build/%.o: src/%.c
	$(CC) -c $(WARN) $(OPTI) $(CFLAGS) $< -o $@

clean :
	cd build && rm -f -v *.o $(EXE) $(TEST_EXE) *.out *.gcov *.gcno *.gcda

# Before generate coverage info, build byak for debug coverage :
# DEBUG_COVERAGE=1 make
# Then run byak to generate gcov files
coverage :
	lcov -c -d `pwd`/build -o build/cov.info
	genhtml `pwd`/build/cov.info --output-directory build/cov

profile : build/$(EXE) build/callgrind.out

build/callgrind.out :
	valgrind --tool=callgrind --callgrind-out-file=build/callgrind.out build/byak

callgraph : build/callgrind.out
	gprof2dot --format=callgrind build/callgrind.out | dot -Tpng -o build/callgraph.png

