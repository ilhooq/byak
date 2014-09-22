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

/** Init king_moves */
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

/** Init knight_moves */
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

/** Init files */
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

/** Init ranks */
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
			diag_mask_ne[i] |= SQ64(i+9*j);
		}

		for (j=0; j < (8-i); j++) {
			diag_mask_ne[i+9*j] = diag_mask_ne[i];
		}
	}

	/* top half of the board */
	for (i=56; i < 64; i++) {
		diag_mask_ne[i] = EMPTY;

		for (j=0 ; j < (i-55); j++) {
			diag_mask_ne[i] |= SQ64(i-9*j);
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
			diag_mask_nw[i] |= SQ64(i+7*j);
		}
		for (j=0; j < i+1; j++) {
			diag_mask_nw[i+7*j] = diag_mask_nw[i];
		}
	}

	/* top half of the board */
	for (i=56; i < 64; i++) {
		diag_mask_nw[i] = EMPTY;
		for (j=0; j < 64-i; j++) {
			diag_mask_nw[i] |= SQ64(i-7*j);
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
		fromSquare = SQ64(sq1);
		for (sq2=0; sq2 < TOTAL_SQUARES; sq2++) {
			toSquare = SQ64(sq2);
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
	/* Init bin2alg array */
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
	return bin2alg[bitboard_bsf(bb)];
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

U64 bitboard_getKingMoves(U64 bb)
{
	return king_moves[bitboard_bsf(bb)];
}

U64 bitboard_getKnightMoves(U64 bb)
{
	return knight_moves[bitboard_bsf(bb)];
}


U64 bitboard_getFile(U64 bb)
{
	return file_mask[bitboard_bsf(bb)];
}

int bitboard_getFileIdx(U64 bb)
{
	return file[bitboard_bsf(bb)];
}

U64 bitboard_getRank(U64 bb)
{
	return rank_mask[bitboard_bsf(bb)];
}

U64 bitboard_getDiagNE(U64 bb)
{
	return diag_mask_ne[bitboard_bsf(bb)];
}

U64 bitboard_getDiagNW(U64 bb)
{
	return diag_mask_nw[bitboard_bsf(bb)];
}

U64 bitboard_getObstructed(U64 from, U64 to)
{
	return obstructed_mask[bitboard_bsf(from)][bitboard_bsf(to)];
}


U64 bitboard_fileAttacks(U64 occupancy, U64 fromSquare)
{
	Square idx= bitboard_bsf(fromSquare);
	U64 Rattacks = Rmagic(idx, occupancy);
	return Rattacks & ~rank_mask[idx];
}

U64 bitboard_rankAttacks(U64 occupancy, U64 fromSquare)
{
	Square idx= bitboard_bsf(fromSquare);
	U64 Rattacks = Rmagic(idx, occupancy);
	return Rattacks &  ~file_mask[idx];
}

U64 bitboard_diagonalAttacks(U64 occupancy, U64 fromSquare)
{
	return Bmagic(bitboard_bsf(fromSquare), occupancy);
}

U64 bitboard_xrayFileAttacks(U64 occupancy, U64 blockers, U64 fromSquare)
{
	U64 attacks = bitboard_fileAttacks(occupancy, fromSquare);
	blockers &= attacks & C64(0x00FFFFFFFFFFFF00);
	if (blockers == 0) {
		return blockers;
	}
	return attacks ^ bitboard_fileAttacks(occupancy ^ blockers, fromSquare);
}

U64 bitboard_xrayRankAttacks(U64 occupancy, U64 blockers, U64 fromSquare)
{
	U64 attacks = bitboard_rankAttacks(occupancy, fromSquare);
	blockers &= attacks & C64(0x7E7E7E7E7E7E7E7E);
	if (blockers == 0) {
		return blockers;
	}
	return attacks ^ bitboard_rankAttacks(occupancy ^ blockers, fromSquare);
}

U64 bitboard_xrayDiagonalAttacks(U64 occupancy, U64 blockers, U64 fromSquare)
{
	U64 attacks = bitboard_diagonalAttacks(occupancy, fromSquare);
	blockers &= attacks & C64(0x007E7E7E7E7E7E00);
	if (blockers == 0) {
		return blockers;
	}
	return attacks ^ bitboard_diagonalAttacks(occupancy ^ blockers, fromSquare);
}
