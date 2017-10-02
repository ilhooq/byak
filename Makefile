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
CFLAGS = -I./src
LDLIBS = -lpthread
LDFLAGS =
WARN = -Wall
OPTI = -O3

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

ifeq ($(DEBUG-PROFILE), 1)
	# Add Gprof support
	CFLAGS += -pg -DDEBUG -Wno-unused-function
	LDFLAGS += -pg
	OPTI =
endif

ifeq ($(DEBUG-GCOV), 1)
	# Add Gcov support
	CFLAGS += -fprofile-arcs -ftest-coverage -DDEBUG
	LDFLAGS += -fprofile-arcs -ftest-coverage
	OPTI =
endif

ifeq ($(USE-INLINING), 1)
	# Force inlining (Enabled by default in non-debugging mode)
	CFLAGS += -DUSE_INLINING
endif

ifeq ($(CLANG), C99)
	# Compile in C99
	CFLAGS = -std=c99
endif

COMMON_SRC  := $(filter-out src/main.c test/main.c, $(wildcard src/*.c))
COMMON_OBJ  := $(COMMON_SRC:src/%.c=build/%.o)

.PHONY: clean

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
