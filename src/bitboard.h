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

#define RANK18 C64(0xFF000000000000FF)

/* Get the Less Signifiant one bit of a bitboard */
#define LS1B(constantU64) ((constantU64) & -(constantU64))

/* Reset the Less Signifiant one bit of a bitboard */
#define RESET_LS1B(constantU64) ((constantU64) & ((constantU64)-1))

/* Convert a square to bitboard */
#define SQ64(constantINT) ((C64(1)) << (constantINT))

/* --- Shiffting macros --- */

#define bitboard_soutOne(constantU64) ((constantU64) >> 8)

#define bitboard_nortOne(constantU64) ((constantU64) << 8)

/* (bb << 1) & ~FILEA */
#define bitboard_eastOne(constantU64) (((constantU64) << 1) & C64(0xfefefefefefefefe))

/* (bb << 9) & ~FILEA */
#define bitboard_noEaOne(constantU64) (((constantU64) << 9) & C64(0xfefefefefefefefe))

/* (bb >> 7) & ~FILEA */
#define bitboard_soEaOne(constantU64) (((constantU64) >> 7) & C64(0xfefefefefefefefe))

/* (bb >> 1) & ~FILEH */
#define bitboard_westOne(constantU64) (((constantU64) >> 1) & C64(0x7f7f7f7f7f7f7f7f))

/* (bb >> 9) & ~FILEH */
#define bitboard_soWeOne(constantU64) (((constantU64) >> 9) & C64(0x7f7f7f7f7f7f7f7f))

/* (bb << 7) & ~FILEH */
#define bitboard_noWeOne(constantU64) (((constantU64) << 7) & C64(0x7f7f7f7f7f7f7f7f))

static INLINE int bitboard_IPopCount(U64 x) {
	int count = 0;
	while (x) {
		count++;
		// x &= x - 1; // reset LS1B
		x = RESET_LS1B(x);
	}
	return count;
}

#if !(defined __LP64__ || defined __LLP64__) || defined _WIN32 && !defined _WIN64
/* This code works for 32 bits or 64 bits procesors*/
U8 static INLINE bitboard_bitScanForward(U64 bb) 
{
	assert(bb);
	/* Returns one plus the index of the 
	 * least significant 1-bit of x, or if x is zero, returns zero */
	return __builtin_ffsll(bb) - 1;
}

U8 static INLINE bitboard_bitScanReverse(U64 bb) 
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
U8 static INLINE bitboard_bitScanForward(U64 bb) 
{
	assert(bb);
	// U64 index;
	__asm__("bsfq %0, %0" : "=r" (bb) : "0" (bb));
	return (U8) bb;
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

void bitboard_init();

char* bitboard_binToAlg(U64 bb);

U64 bitboard_algToBin(const char *alg);

/**
 * bitboard_display
 * Display a bitboard in a 8x8 board 
 * @param U64 bitboard
 * @return void
 */
void bitboard_display(U64 bb);



U64 bitboard_getKingMoves(U64 bb);

U64 bitboard_getKnightMoves(U64 bb);

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
