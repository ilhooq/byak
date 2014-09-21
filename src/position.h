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

#ifndef POSITION_H
#define POSITION_H
#include "types.h"
#include "move.h"

#define WHITE 0
#define BLACK 1

/* 16 possible states */
#define W_CASTLE_K 0x1 /* 0001 : 1 */
#define W_CASTLE_Q 0x2 /* 0010 : 2 */
#define B_CASTLE_K 0x4 /* 0100 : 4 */
#define B_CASTLE_Q 0x8 /* 1000 : 8 */


typedef struct {
	U64 bb_pieces[12]; // Pieces occupancy
	U64 bb_side[2]; // Side occupancy
	U64 bb_occupied;
	U64 bb_empty ;

	int movelistcount;

	U64 pinned; // Pinned squares
	U64 pinner[64];

	U64 kingAttacks[2];
	U64 knightsAttacks[2];
	U64 queenRooksAttacks[2];
	U64 queenBishopsAttacks[2];

	U64 pawnAttacks[2];

	int in_check;
	int checkmated;

	U8 enpassant;

	short castling_rights;

	int side; // White : 0, black : 1
	U64 hash;
} Position;

extern Position pos;

typedef struct {
	U64 nodes;
	U64 checks;
	U64 checkmated;
	U64 captures;
	U64 EP;
	U64 castles;
	U64 promotions;
} PerftData;

void position_init();

/**
 * Fen parser
 * @param char* the fen string to parse
 * @return 0 if OK ether -1 if an error occured
 */
int position_fromFen(const char *fen);

/**
 * Display the board
 */
void position_display();

/**
 * Generate all legal moves
 * @param movelist pointer to a moves array
 */
int position_generateMoves(Move *movelist);

/**
 * Make a move
 */
void position_makeMove(Move *move);

/**
 * Undo a move
 */
void position_undoMove(Move *move);

int position_inCheck();

#endif
