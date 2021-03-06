/**
* Byak, a UCI chess engine.
* Copyright (C) 2013  Sylvain Philip
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
This is our pseudo random number generator (PRNG) used to compute hash keys.

This algorithm comes from Stockfish

George Marsaglia invented the RNG-Kiss-family in the early 90's. This is a
specific version that Heinz van Saanen derived from some public domain code
by Bob Jenkins (http://chessprogramming.wikispaces.com/Bob+Jenkins). 
Following the feature list, as tested by Heinz.

- Quite platform independent
- Passes ALL dieharder tests!
- ~4 times faster than SSE2-version of Mersenne twister
- Average cycle length: ~2^126
- 64 bit seed
- Return doubles with a full 53 bit mantissa
- Thread safe
*/

#include "types.h"

#define rot(x,k) (((x)<<(k))|((x)>>(64-(k))))

static U64 _a;
static U64 _b;
static U64 _c;
static U64 _d;

/** 
 * Return 64 bit unsigned integer in between [0, 2^64 - 1]
 */
U64 rand64()
{
	U64 e = _a - rot(_b, 7);
	_a = _b ^ rot(_c, 13);
	_b = _c + rot(_d, 37);
	_c = _d + e;
	_d = e + _a;
	return _d;
}


void prng_init(int seed) 
{
	int i;
	_a = 0xf1ea5eed;
	_b = _c = _d = 0xd4e12c77;

	for (i = 0; i < seed; i++) { /* Scramble a few rounds */
		rand64();
	}
}
