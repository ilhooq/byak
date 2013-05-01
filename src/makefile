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

CXX  = gcc

# produce WIN 32 bits exe 
# CXX  = i686-w64-mingw32-gcc

# produce WIN 64 bits exe 
# CXX  = x86_64-w64-mingw32-gcc

# LANG = -ansi
LANG = 
WARN = -Wall
OPTI = 
# OPTI = -O1

OPTI = -O3
#OPTI = -O

# Symboles de débuggage
# GNU debogueur
# OPTI += -g
# Gprof
# OPTI += -pg

EXEC = chess_engine
SRC  = $(wildcard *.c)
OBJ  = $(SRC:%.c=bin/%.o)

all : bin/$(EXEC)

# linking
bin/$(EXEC) : $(OBJ)
	$(CXX) -o $@ $(LANG) $(WARN) $(OPTI) $(DIR) $(OBJ) -lpthread 

# compilation
bin/%.o : %.c
	$(CXX) -c -o $@ $< $(LANG) $(WARN) $(OPTI) $(DIR)

clean :
	rm -f -v bin/*.o bin/$(EXEC)
