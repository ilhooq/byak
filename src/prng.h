#ifndef PRNG_H
#define PRNG_H
#include "bitboard.h"

/** 
 * Return 64 bit unsigned integer in between [0, 2^64 - 1]
 */
U64 rand64();
void prng_init(int seed);

#endif


 

 

