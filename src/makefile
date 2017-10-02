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

CC = gcc
EXE = byak
TEST_EXE = byak_testsuite
CFLAGS =
LDLIBS = -lpthread
LDFLAGS =
WARN = -Wall
OPTI = -O3

ifeq ($(TARGET), win32)
	# Produce WIN 32 bits exe from Linux
	CC = i686-w64-mingw32-gcc
	EXE = byak.exe
	TEST_EXE = testsuite.exe
	LDLIBS =
endif

ifeq ($(TARGET), win64)
	# Produce WIN 64 bits exe from Linux
	CC=x86_64-w64-mingw32-gcc
	EXE = byak.exe
	TEST_EXE = testsuite.exe
	LDLIBS =
endif

ifeq ($(PLATFORM), win32)
	# Produce WIN 32 bits exe from Windows
	CC = mingw32-gcc.exe
	EXE = byak.exe
	TEST_EXE = testsuite.exe
	LDLIBS =
endif

ifeq ($(PLATFORM), win64)
	# Produce WIN 64 bits exe from Windows
	CC = mingw64-gcc.exe
	EXE = byak.exe
	TEST_EXE = testsuite.exe
	LDLIBS =
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

COMMON_SRC  := $(filter-out test.c main.c, $(wildcard *.c))
COMMON_OBJ  := $(COMMON_SRC:%.c=%.o)

.PHONY: clean

all : $(EXE)

testsuite : $(TEST_EXE)

$(EXE) : main.o $(COMMON_OBJ) 
	$(CC) -o $@ $(WARN) $(OPTI) main.o $(COMMON_OBJ) $(LDFLAGS) $(LDLIBS)

$(TEST_EXE) : test.o $(COMMON_OBJ) 
	$(CC) -o $@ $(WARN) $(OPTI) test.o $(COMMON_OBJ) $(LDFLAGS) $(LDLIBS)

main.o :
	$(CC) -c $(WARN) $(OPTI) $(CFLAGS) main.c -o main.o

test.o :
	$(CC) -c $(WARN) $(OPTI) $(CFLAGS) test.c -o test.o

$(COMMON_OBJ) : %.o: %.c
	$(CC) -c $(WARN) $(OPTI) $(CFLAGS) $< -o $@

clean :
	rm -f -v *.o $(EXE) $(TEST_EXE) *.out *.gcov *.gcno *.gcda
