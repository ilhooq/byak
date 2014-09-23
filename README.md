# Byak

## A chess UCI engine written in C

This is a work in progress and not all features were implemented but the engine is usable in a UCI compatible GUI like SCID or Arena.

The move generator is prompt and accurate (using Pradyumna Kannan Magic bitboard). On a Core2 duo 2.66GHz, perft runs 100 Mn/s.

To run the perft, the command line arguments are : byak perft \[fenstring\] \[depth\]

For instance :

```
./byak perft "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1" 7
```

### Build instructions on Linux

Open a terminal and type the commands below :
```
$ git clone git@github.com:ilhooq/byak.git
$ cd byak/src
$ make
```

### Build instructions on Windows

Download and install [MinGW](http://www.mingw.org/) then, configure your environment PATH to add the directory where MinGW is installed ([More info](http://www.mingw.org/wiki/Getting_Started))

Download and unzip the sources of Byak : https://github.com/ilhooq/byak/archive/master.zip

Open the command prompt, go in the directory where Byak sources were extracted (using **cd** command) and type the commands below :

```
> cd src
> mingw32-make PLATFORM=win32
```


