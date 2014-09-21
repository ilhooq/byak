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

#ifndef MOVE_H
#define MOVE_H

#include "bitboard.h"

#define MOVE_NULL              0 
#define MOVE_NORMAL            0x1 
#define MOVE_PAWN_DOUBLE       0x2   // 1 << 1
#define MOVE_CAPTURE           0x4   // 1 << 2
#define MOVE_PROMOTION         0x8   // 1 << 3
#define MOVE_PROMOTION_QUEEN   0x10  // 1 << 4
#define MOVE_PROMOTION_BISHOP  0x20  // 1 << 5
#define MOVE_PROMOTION_KNIGHT  0x40  // 1 << 6
#define MOVE_PROMOTION_ROOK    0x80  // 1 << 7
#define MOVE_ENPASSANT         0x100 // 1 << 8
#define MOVE_CASTLE            0x200 // 1 << 9
#define MOVE_CASTLE_KS         0x400 // 1 << 10
#define MOVE_CASTLE_QS         0x800 // 1 << 11

/* 16 bytes */
typedef struct {
	// for search
	/* 4 B */ unsigned int score;
	/* 4 B */ unsigned int padding; // This is just to adjust the struct size to 16B
	/* 2 B */ U16 flags;
	/* 1 B */ U8 from; // Square from
	/* 1 B */ U8 to;   // Square to
	/* 1 B */ U8 captured_piece; // This will be determined when making move
	/* 1 B */ U8 ep;   // Square enpassant
	/* 1 B */ U8 castling_rights;
} Move;

void move_display(Move *move);
void move_displayAlg(Move *move);
char move_getPromotionPieceChar(U8 flags);
#endif
