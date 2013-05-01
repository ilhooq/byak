

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
