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

#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>

/*
 * Square mapping used in the app :
 * Little endian rank-file (LERF) mapping
     A    B    C    D    E    F    G    H
   +----+----+----+----+----+----+----+----+
 8 | 56 | 57 | 58 | 59 | 60 | 61 | 62 | 63 |  8th rank
   +----+----+----+----+----+----+----+----+
 7 | 48 | 49 | 50 | 51 | 52 | 53 | 54 | 55 |  7th rank
   +----+----+----+----+----+----+----+----+
 6 | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 |  6th rank
   +----+----+----+----+----+----+----+----+
 5 | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 |  5th rank
   +----+----+----+----+----+----+----+----+
 4 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 |  4th rank
   +----+----+----+----+----+----+----+----+
 3 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 |  3rd rank
   +----+----+----+----+----+----+----+----+
 2 |  8 |  9 | 10 | 11 | 12 | 13 | 14 | 15 |  2nd rank
   +----+----+----+----+----+----+----+----+
 1 |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  1st rank
   +----+----+----+----+----+----+----+----+
     A    B    C    D    E    F    G    H - file(s)
 *
*/

#ifndef DEBUG
	#define USE_INLINING
	/* Deactivate assertions */
	#define NDEBUG
#endif

/* inline is not valid in C ANSI */
#ifdef USE_INLINING
	#ifdef _MSC_VER
		#define INLINE __forceinline
	#elif defined(__GNUC__)
		#define INLINE __inline__ __attribute__((always_inline))
	#else
		#define INLINE inline
	#endif
#else
	#define INLINE
#endif

#define C64(constantU64) __UINT64_C(constantU64)
#define ULL(integer) ((unsigned long long int) integer)

typedef uint64_t U64;
typedef int64_t  S64;
typedef uint32_t  U32;
typedef uint16_t  U16;
typedef uint8_t  U8;

/* This constant is for magicmoves.h */
#define __64_BIT_INTEGER_DEFINED__

typedef enum enumProtocol {
	DEFAULT,
	UCI,
	XBOARD
} Protocol;

extern Protocol protocol;


typedef enum enumSquare {
	a1, b1, c1, d1, e1, f1, g1, h1, /*  0 .. 7  */
	a2, b2, c2, d2, e2, f2, g2, h2, /*  8 .. 15 */
	a3, b3, c3, d3, e3, f3, g3, h3, /* 16 .. 23 */
	a4, b4, c4, d4, e4, f4, g4, h4, /* 24 .. 31 */
	a5, b5, c5, d5, e5, f5, g5, h5, /* 32 .. 39 */
	a6, b6, c6, d6, e6, f6, g6, h6, /* 40 .. 47 */
	a7, b7, c7, d7, e7, f7, g7, h7, /* 48 .. 55 */
	a8, b8, c8, d8, e8, f8, g8, h8,  /* 56 .. 63 */
	TOTAL_SQUARES, /* 64 */
	NONE_SQUARE, /* 64 */
} Square;

typedef enum enumPiece {
P, // Any white pawns
p, // Any black pawns
K, // White king
k, // Black king
Q, // Any white Quuens
q, // Any black Quuens
N, // Any white knights
n, // Any black knights
B, // Any white bishops
b, // Any black bishops
R, // Any white rooks
r,  // Any black rooks
NONE_PIECE
} Piece;

#endif
