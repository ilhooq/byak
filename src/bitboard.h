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

#ifndef BITBOARD_H
#define BITBOARD_H

#include <assert.h>
#include "types.h"

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



#define EMPTY C64(0x0000000000000000)
#define FULL C64(0xFFFFFFFFFFFFFFFF)

#define RANK1 C64(0X00000000000000FF)
#define RANK2 C64(0X000000000000FF00)
#define RANK3 C64(0X0000000000FF0000)
#define RANK4 C64(0X00000000FF000000)
#define RANK5 C64(0X000000FF00000000)
#define RANK6 C64(0X0000FF0000000000)
#define RANK7 C64(0X00FF000000000000)
#define RANK8 C64(0XFF00000000000000)

#define FILEA C64(0X0101010101010101)
#define FILEB C64(0X0202020202020202)
#define FILEC C64(0X0404040404040404)
#define FILED C64(0X0808080808080808)
#define FILEE C64(0X1010101010101010)
#define FILEF C64(0X2020202020202020)
#define FILEG C64(0X4040404040404040)
#define FILEH C64(0X8080808080808080)



/* Get the Less Signifiant one bit of a bitboard */
#define LS1B(constantU64) ((constantU64) & -(constantU64))

/* Reset the Less Signifiant one bit of a bitboard */
#define RESET_LS1B(constantU64) ((constantU64) & ((constantU64)-1))

/* Convert a square to bitboard */
#define SQ64(constantINT) ((C64(1)) << (constantINT))

#define NONE_SQUARE 0

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
} Square;

/** Algebric notation for each square */
extern char bin2alg[64][3];

/** Knight moves mask for each square */
extern U64 knight_moves[64];

/** King moves mask for each square */
extern U64 king_moves[64];

extern int file[64];
extern int rank[64];

/** Rank mask for each square */
extern U64 rank_mask[64];

/** File mask for each square */
extern U64 file_mask[64];

/** NorthEast Diag mask for each square */
extern U64 diag_mask_ne[64];

/** NorthWest Diag mask for each square */
extern U64 diag_mask_nw[64];

extern U64 obstructed_mask[64][64];


int bitboard_IPopCount(U64 x);

/**
* Shiffting methods
*/
U64 static INLINE bitboard_soutOne(U64 bb)
{
	return bb >> 8;
}

U64 static INLINE bitboard_nortOne(U64 bb)
{
	return  bb << 8;
}

U64 static INLINE bitboard_eastOne(U64 bb)
{
	/*		(bb << 1) & ~FILEA */
	return (bb << 1) & C64(0xfefefefefefefefe);
}

U64 static INLINE bitboard_noEaOne(U64 bb)
{
	/*		(bb << 9) & ~FILEA */
	return (bb << 9) & C64(0xfefefefefefefefe);
}

U64 static INLINE bitboard_soEaOne(U64 bb)
{
	/*		(bb >> 7) & ~FILEA */
	return (bb >> 7) & C64(0xfefefefefefefefe);
}

U64 static INLINE bitboard_westOne(U64 bb)
{
	/*		(bb >> 1) & ~FILEH */
	return (bb >> 1) & C64(0x7f7f7f7f7f7f7f7f);
}

U64 static INLINE bitboard_soWeOne(U64 bb) {
	/*		(bb >> 9) & ~FILEH */
	return (bb >> 9) & C64(0x7f7f7f7f7f7f7f7f);
}

U64 static INLINE bitboard_noWeOne(U64 bb)
{
	/*		(bb << 7) & ~FILEH */
	return (bb << 7) & C64(0x7f7f7f7f7f7f7f7f);
}


void bitboard_init();

char* bitboard_binToAlg(U64 bb);

U64 bitboard_algToBin(const char *alg);

#if !(defined __LP64__ || defined __LLP64__) || defined _WIN32 && !defined _WIN64
/* This code works for 32 bits or 64 bits procesors*/
int static INLINE bitboard_bitScanForward(U64 bb) 
{
	assert(bb);
	/* Returns one plus the index of the 
	 * least significant 1-bit of x, or if x is zero, returns zero */
	return __builtin_ffsll(bb) - 1;
}

int static INLINE bitboard_bitScanReverse(U64 bb) 
{
	assert(bb);
	/* Returns the number of leading 0-bits in x, starting at the most 
	 * significant bit position. If x is 0, the result is undefined */
	return 63 - __builtin_clzll(bb);
}

#else
/* This code is only for 64 bits procesors */
/**
 * bitboard_bitScanForward
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */
int static INLINE bitboard_bitScanForward(U64 bb) 
{
	assert(bb);
	// U64 index;
	__asm__("bsfq %0, %0" : "=r" (bb) : "0" (bb));
	return (int) bb;
}

/**
 * bitboard_bitScanReverse
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of most significant one bit
 */
int static INLINE bitboard_bitScanReverse(U64 bb) 
{
	assert(bb);
	U64 index;
	__asm__("bsrq %0, %0": "=r" (index) : "mr" (bb));
	return (int) index;
}

#endif

/**
 * bitboard_display
 * Display a bitboard in a 8x8 board 
 * @param U64 bitboard
 * @return void
 */
void bitboard_display(U64 bb);

U64 static INLINE bitboard_getKingMoves(U64 bb)
{
	assert(bb != EMPTY);
	Square idx = bitboard_bitScanForward(bb);
	return king_moves[idx];
}

U64 static INLINE bitboard_getKnightMoves(U64 bb)
{
	assert(bb != EMPTY);
	Square idx = bitboard_bitScanForward(bb);
	return knight_moves[idx];
}

U64 bitboard_getFile(U64 bb);

int bitboard_getFileIdx(U64 bb);

U64 bitboard_getRank(U64 bb);

U64 bitboard_getDiagNE(U64 bb);

U64 bitboard_getDiagNW(U64 bb);

U64 bitboard_getObstructed(U64 from, U64 to);

U64 bitboard_fileAttacks(U64 occupancy, U64 fromSquare);

U64 bitboard_rankAttacks(U64 occupancy, U64 fromSquare);

U64 bitboard_diagonalAttacks(U64 occupancy, U64 fromSquare);

U64 bitboard_xrayFileAttacks(U64 occupancy, U64 blockers, U64 fromSquare);

U64 bitboard_xrayRankAttacks(U64 occupancy, U64 blockers, U64 fromSquare);

U64 bitboard_xrayDiagonalAttacks(U64 occupancy, U64 blockers, U64 fromSquare);

#endif
