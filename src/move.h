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
#include "pieces.h"

typedef enum {
	NORMAL, PAWN_DOUBLE, PROMOTION, ENPASSANT, CASTLE
} TypeMove;

typedef struct {
	Square from; // Square from
	Square to;   // Square to
	TypeMove type;
	char capture; // 1: capture, 0: no capture
	Piece promoted_piece; // Q,R,B,N,q,r,b,n
	Piece captured_piece; // This will be determined when making move
	Square ep;
	short castling_rights;
	// for search
	int score;

} Move;

Move *move_create(U64 from_square, U64 to_square, TypeMove type, 
			char capture, Piece promoted_piece, Piece captured_piece);

void move_display(Move *move);
void move_displayAlg(Move *move);
#endif
