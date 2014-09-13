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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "move.h"
#include "bitboard.h"
#include "position.h"

Move *move_create(U64 from_square, U64 to_square, TypeMove type, 
			char capture, Piece promoted_piece, Piece captured_piece)
{
	Move *move = malloc(sizeof(Move));
	move->from = bitboard_bitScanForward(from_square);
	move->to = bitboard_bitScanForward(to_square);
	move->type = type;
	move->capture = (capture)? capture : 0;
	move->promoted_piece = (promoted_piece)? promoted_piece : NONE_PIECE;
	// This will be determined when making move
	move->captured_piece = (captured_piece)? captured_piece : NONE_PIECE;
	move->ep = NONE_SQUARE;
	move->castling_rights = 0;
	// move->prevHash = EMPTY;
	return move;
}

void move_display(Move *move)
{
	char type[15] = "";
	switch(move->type) {
		case NORMAL: strcat(type, "normal"); break;
		case PAWN_DOUBLE: strcat(type, "pawn double"); break;
		case PROMOTION: strcat(type, "promotion"); break;
		case ENPASSANT: strcat(type, "enpassant"); break;
		case CASTLE: strcat(type, "castle"); break;
	}
	
	printf("Move: from square = %s  to square = %s  type = %s capture = %i  promoted piece = %c capture_name = %c\n",
			bitboard_binToAlg(SQ64(move->from)),
			bitboard_binToAlg(SQ64(move->to)),
			type,
			move->capture,
			get_piece_letter(move->promoted_piece),
			get_piece_letter(move->captured_piece));
}

void move_displayAlg(Move *move)
{
	printf("%s%c%s%c", bitboard_binToAlg(SQ64(move->from)),
						move->capture ? 'x' : '-',
						bitboard_binToAlg(SQ64(move->to)),
						(move->type == PROMOTION) ? get_piece_letter(move->promoted_piece) : ' ');
}

