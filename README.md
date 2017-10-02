# Byak

## A chess UCI engine written in C

This is a work in progress and not all features were implemented but the engine is usable in a UCI compatible GUI like SCID or Arena.

The move generator is prompt and accurate (using Pradyumna Kannan Magic bitboard). On a Core2 duo 2.66GHz, perft runs 100 Mn/s.

### Build instructions

#### Build on linux

Standard build:

```bash
git clone https://github.com/ilhooq/byak.git && cd byak
make
```

Cross-platform build targeting win32 or win64:

```bash
# Install mingw before, ex on Ubuntu:
sudo apt-get install mingw-w64

# win32 build
TARGET=win32 make

# win64 build
TARGET=win64 make
```

### Build on Windows

Download and install [MinGW](http://www.mingw.org/) then, configure your environment PATH to add the directory where MinGW is installed ([More info](http://www.mingw.org/wiki/Getting_Started))

Download and unzip the sources of Byak : https://github.com/ilhooq/byak/archive/master.zip

Open the command prompt, go in the directory where Byak sources were extracted (using **cd** command) and type the commands below :

For 32bits system :

```bash
mingw32-make PLATFORM=win32
```

For 64bits system :

```bash
mingw64-make PLATFORM=win64
```

