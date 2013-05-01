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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "bitboard.h"
#include "magicmoves.h"

/** Algebric notation for each square */
static char bin2alg[64][3];

/** Knight moves mask for each square */
static U64 knight_moves[64];

/** King moves mask for each square */
static U64 king_moves[64];

static int file[64];
static int rank[64];

/** Rank mask for each square */
static U64 rank_mask[64];

/** File mask for each square */
static U64 file_mask[64];

/** NorthEast Diag mask for each square */
static U64 diag_mask_ne[64];

/** NorthWest Diag mask for each square */
static U64 diag_mask_nw[64];

static U64 obstructed_mask[64][64];

/** Private function to init king_moves */
static void gen_king_moves()
{
	int i=0;
	for (i=0; i < 64; i++) {
		U64 offset = C64(1) << i;
		king_moves[i]  = bitboard_nortOne(offset);
		king_moves[i] |= bitboard_westOne(offset);
		king_moves[i] |= bitboard_eastOne(offset);
		king_moves[i] |= bitboard_soutOne(offset);
		king_moves[i] |= bitboard_noEaOne(offset);
		king_moves[i] |= bitboard_noWeOne(offset);
		king_moves[i] |= bitboard_soEaOne(offset);
		king_moves[i] |= bitboard_soWeOne(offset);
	}
}

/** Private function to init knight_moves */
static void gen_knight_moves()
{
	int i=0;
	for (i=0; i < 64; i++) {
		U64 offset = C64(1) << i;
		/* noNoEa */
		knight_moves[i]  = (offset << 17) & ~FILEA;
		/* noEaEa */
		knight_moves[i] |= (offset << 10) & ~FILEA & ~FILEB;
		/* soEaEa */
		knight_moves[i] |= (offset >> 6)  & ~FILEA & ~FILEB;
		/* soSoEa */
		knight_moves[i] |= (offset >> 15) & ~FILEA;
		/* noNoWe */
		knight_moves[i] |= (offset << 15) & ~FILEH;
		/* noWeWe */
		knight_moves[i] |= (offset << 6)  & ~FILEG & ~FILEH;
		/* soWeWe */
		knight_moves[i] |= (offset >> 10) & ~FILEG & ~FILEH;
		/* soSoWe */
		knight_moves[i] |= (offset >> 17) & ~FILEH;
	}
}

static void gen_files()
{
	int count = 0, i=0, j=0;
	for (i=0 ;i < 8; i++) {
		for (j=0; j < 8; j++) {
			file_mask[count] = FILEA << j;
			file[count] = j;
			count++;
		}
	}
}

static void  gen_ranks()
{
	int count = 0, i=0, j=0;
	for (i=0 ;i < 8; i++) {
		for (j=0; j < 8; j++) {
			rank_mask[count] = RANK1 << (8*i);
			rank[count] = j;
			count++;
		}
	}
}

/*
 * Original code from Shatranj
 * Here we use NE instead of NW as the board representation differs from Shatranj
 */
static void gen_diag_ne()
{
	int i=0, j=0;

	/* bottom half of the board */
	for (i=0; i < 8; i++) {
		diag_mask_ne[i] = EMPTY;
		for (j=0 ; j < (8-i); j++) {
			diag_mask_ne[i] |= C64(1) << (i+9*j);
			/*
			int opp = i+9*j;
			printf("square : %i - opposite :%i\n", i, opp );
			*/
		}

		for (j=0; j < (8-i); j++) {
			diag_mask_ne[i+9*j] = diag_mask_ne[i];
		}
	}

	/* top half of the board */
	for (i=56; i < 64; i++) {
		diag_mask_ne[i] = EMPTY;

		for (j=0 ; j < (i-55); j++) {
			diag_mask_ne[i] |= C64(1) << (i-9*j);
			/*
			int opp = i-9*j;
			printf("square : %i - opposite :%i\n", i, opp );
			*/
		}

		for (j=0 ; j < (i-55); j++) {
			diag_mask_ne[i-9*j] = diag_mask_ne[i];
		}
	}
}

/*
 * Original code from Shatranj
 * Here we use NW instead of NE as the board representation differs from Shatranj
 */

static void gen_diag_nw()
{
	int i=0, j=0;
	/* bottom half of the board */
	for (i=0; i < 8; i++) {
		diag_mask_nw[i] = EMPTY;
		for (j=0; j < i+1; j++) {
			diag_mask_nw[i] |= C64(1) << (i+7*j);
			/*
			int opp = i+7*j;
			printf("square : %i - opposite :%i\n", i, opp);
			*/
		}
		for (j=0; j < i+1; j++) {
			diag_mask_nw[i+7*j] = diag_mask_nw[i];
		}
	}

	/* top half of the board */
	for (i=56; i < 64; i++) {
		diag_mask_nw[i] = EMPTY;
		for (j=0; j < 64-i; j++) {
			diag_mask_nw[i] |= C64(1) << (i-7*j);
			/*
			int opp = i-7*j;
			printf("square : %i - opposite :%i\n", i, opp);
			*/
		}
		for (j=0; j < 64-i; j++) {
			diag_mask_nw[i-7*j] = diag_mask_nw[i];
		}
	}
}

static void gen_obstructed() 
{
	/* See http://chessprogramming.wikispaces.com/Square+Attacked+By#toc7 
	 * to see how to calculate in-between set
	 */
	S64 m1 = -1;
	U64 fromSquare = C64(0), toSquare = C64(0), btwn = C64(0);
	int sq1=0, sq2=0;
	for (sq1=0; sq1 < TOTAL_SQUARES; sq1++) {
		fromSquare = C64(1) << sq1;
		for (sq2=0; sq2 < TOTAL_SQUARES; sq2++) {
			toSquare = C64(1) << sq2;
			obstructed_mask[sq1][sq2] = EMPTY;
			 // This mask include at least fromSquare or toSquare...
			//  ( btwn including toSquare and excluding fromSquare )
			btwn  = (m1 << sq1) ^ (m1 << sq2);
			btwn |= fromSquare|toSquare; // ... So Include both...
			btwn ^= fromSquare|toSquare; // ... and exclude both from the mask

			if (file_mask[sq1] & toSquare) {
				obstructed_mask[sq1][sq2] = file_mask[sq1] & btwn;
			}
			else if (rank_mask[sq1] & toSquare) {
				obstructed_mask[sq1][sq2] = rank_mask[sq1] & btwn;
			}
			else if (diag_mask_ne[sq1] & toSquare) {
				obstructed_mask[sq1][sq2] =diag_mask_ne[sq1] & btwn;
			}
			else if (diag_mask_nw[sq1] & toSquare) {
				obstructed_mask[sq1][sq2] = diag_mask_nw[sq1] & btwn;
			}
		}
	}
}

void bitboard_init()
{
	int i=0, j=0, offset=0;
	char letters[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
	for (i=1; i < 9; i++) {
		for (j=0; j < 8; j++) {
			sprintf(bin2alg[offset], "%c%i", letters[j], i);
			offset++;
		}
	}

	gen_king_moves();
	gen_knight_moves();
	gen_files();
	gen_ranks();
	gen_diag_ne();
	gen_diag_nw();
	gen_obstructed();
	/* Init the magic moves generator for the all the application */
	initmagicmoves();
}

char* bitboard_binToAlg(U64 bb)
{
	return bin2alg[bitboard_bitScanForward(bb)];
}

U64 bitboard_algToBin(const char *alg)
{
	U64 rank = C64(0);
	U64 file = C64(0);

	U64 files[8] = {FILEA, FILEB, FILEC, FILED, FILEE, FILEF, FILEG, FILEH};
	U64 ranks[8] = {RANK1, RANK2, RANK3, RANK4, RANK5, RANK6, RANK7, RANK8};
	char x[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
	char y[8] = {'1','2', '3', '4', '5', '6', '7','8'};

	int i=0;

	for (i=0; i < 8; i++) {
		if ( alg[0] == x[i]) {
			file = files[i];
		}
		if ( alg[1] == y[i]) {
			rank = ranks[i];
		}
	}

	return rank & file;
}

#if !(defined __LP64__ || defined __LLP64__) || defined _WIN32 && !defined _WIN64
/* This code works for 32 bits or 64 bits procesors*/
int bitboard_bitScanForward(U64 bb) 
{
	assert(bb);
	/* Returns one plus the index of the 
	 * least significant 1-bit of x, or if x is zero, returns zero */
	return __builtin_ffsll(bb) - 1;
}

int bitboard_bitScanReverse(U64 bb) 
{
	assert(bb);
	/* Returns the number of leading 0-bits in x, starting at the most 
	 * significant bit position. If x is 0, the result is undefined */
	return 63 - __builtin_clzll(bb);
}

#else
/* This code is only for 64 bits procesors */

int bitboard_bitScanForward(U64 bb)
{
	U64 index;
	assert(bb);
	__asm__("bsfq %1, %0": "=r"(index): "rm"(bb) );
	return (int) index;
}


int bitboard_bitScanReverse(U64 bb)
{
	U64 index;
	assert(bb);
	__asm__("bsrq %1, %0": "=r"(index): "rm"(bb) );
	return (int) index;
}

#endif

inline int bitboard_IPopCount(U64 x) {
	int count = 0;
	while (x) {
		count++;
		// x &= x - 1; // reset LS1B
		x = RESET_LS1B(x);
	}
	return count;
};



void bitboard_display(U64 bb)
{
	int rankIndex = 7, 
	fileIndex = 0, 
	offset = 0,
	i=0;


	for (i=1; i < 65; i++) {

		if (fileIndex == 0) {
			printf("\n    +---+---+---+---+---+---+---+---+\n");
			printf("  %i |", rankIndex+1);
		}

		offset = 8*rankIndex + fileIndex;

		if ((bb & C64(1) << offset) != C64(0)) {
			printf(" 1 |");
		} else { 
			printf(" 0 |");
		}

		fileIndex++;

		if (fileIndex > 7) {
			rankIndex--;
			fileIndex = 0;
		}
	}

	printf("\n    +---+---+---+---+---+---+---+---+\n");
	printf("      a   b   c   d   e   f   g   h\n");
}


/**
* Shiffting methods
*/
INLINE U64 bitboard_soutOne(U64 bb)
{
	return bb >> 8;
};

INLINE U64 bitboard_nortOne(U64 bb)
{
	return  bb << 8;
}

INLINE U64 bitboard_eastOne(U64 bb)
{
	/*		(bb << 1) & ~FILEA */
	return (bb << 1) & C64(0xfefefefefefefefe);
}

INLINE U64 bitboard_noEaOne(U64 bb)
{
	/*		(bb << 9) & ~FILEA */
	return (bb << 9) & C64(0xfefefefefefefefe);
}

INLINE U64 bitboard_soEaOne(U64 bb)
{
	/*		(bb >> 7) & ~FILEA */
	return (bb >> 7) & C64(0xfefefefefefefefe);
}

INLINE U64 bitboard_westOne(U64 bb)
{
	/*		(bb >> 1) & ~FILEH */
	return (bb >> 1) & C64(0x7f7f7f7f7f7f7f7f);
}

INLINE U64 bitboard_soWeOne(U64 bb) {
	/*		(bb >> 9) & ~FILEH */
	return (bb >> 9) & C64(0x7f7f7f7f7f7f7f7f);
}

INLINE U64 bitboard_noWeOne(U64 bb)
{
	/*		(bb << 7) & ~FILEH */
	return (bb << 7) & C64(0x7f7f7f7f7f7f7f7f);
}

U64 bitboard_getKingMoves(U64 bb)
{
	assert(bb != EMPTY);
	Square idx = bitboard_bitScanForward(bb);
	return king_moves[idx];
}

U64 bitboard_getKnightMoves(U64 bb)
{
	assert(bb != EMPTY);
	Square idx = bitboard_bitScanForward(bb);
	return knight_moves[idx];
}

U64 bitboard_getFile(U64 bb)
{
	assert(bb != EMPTY);
	Square idx = bitboard_bitScanForward(bb);
	return file_mask[idx];
}

int bitboard_getFileIdx(U64 bb)
{
	Square sq = bitboard_bitScanForward(bb);
	return file[sq];
}

U64 bitboard_getRank(U64 bb)
{
	assert(bb != EMPTY);
	Square idx = bitboard_bitScanForward(bb);
	return rank_mask[idx];
}

U64 bitboard_getDiagNE(U64 bb)
{
	assert(bb != EMPTY);
	Square idx = bitboard_bitScanForward(bb);
	return diag_mask_ne[idx];
}

U64 bitboard_getDiagNW(U64 bb)
{
	assert(bb != EMPTY);
	Square idx = bitboard_bitScanForward(bb);
	return diag_mask_nw[idx];
}

U64 bitboard_getObstructed(U64 from, U64 to)
{
	Square idxFrom = bitboard_bitScanForward(from);
	Square idxTo   = bitboard_bitScanForward(to);
	return obstructed_mask[idxFrom][idxTo];
}


U64 bitboard_fileAttacks(U64 occupancy, U64 fromSquare)
{
	Square idx= bitboard_bitScanForward(fromSquare);
	U64 Rattacks = Rmagic(idx, occupancy);
	return Rattacks & ~rank_mask[idx];
}

U64 bitboard_rankAttacks(U64 occupancy, U64 fromSquare)
{
	Square idx= bitboard_bitScanForward(fromSquare);
	U64 Rattacks = Rmagic(idx, occupancy);
	return Rattacks &  ~file_mask[idx];
}

U64 bitboard_diagonalAttacks(U64 occupancy, U64 fromSquare)
{
	Square idx= bitboard_bitScanForward(fromSquare);
	return Bmagic(idx, occupancy);
}

U64 bitboard_xrayFileAttacks(U64 occupancy, U64 blockers, U64 fromSquare)
{
	U64 attacks = bitboard_fileAttacks(occupancy, fromSquare);
	blockers &= attacks & 0x00FFFFFFFFFFFF00;
	if (blockers == 0) {
		return blockers;
	}
	return attacks ^ bitboard_fileAttacks(occupancy ^ blockers, fromSquare);
}

U64 bitboard_xrayRankAttacks(U64 occupancy, U64 blockers, U64 fromSquare)
{
	U64 attacks = bitboard_rankAttacks(occupancy, fromSquare);
	blockers &= attacks & 0x7E7E7E7E7E7E7E7E;
	if (blockers == 0) {
		return blockers;
	}
	return attacks ^ bitboard_rankAttacks(occupancy ^ blockers, fromSquare);
}

U64 bitboard_xrayDiagonalAttacks(U64 occupancy, U64 blockers, U64 fromSquare)
{
	U64 attacks = bitboard_diagonalAttacks(occupancy, fromSquare);
	blockers &= attacks & 0x007E7E7E7E7E7E00;
	if (blockers == 0) {
		return blockers;
	}
	return attacks ^ bitboard_diagonalAttacks(occupancy ^ blockers, fromSquare);
}
