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

char move_getPromotionPieceChar(U8 flags)
{
	if (flags & MOVE_PROMOTION_QUEEN) {
		return 'q';
	}
	if (flags & MOVE_PROMOTION_BISHOP) {
		return 'b';
	}
	if (flags & MOVE_PROMOTION_ROOK) {
		return 'r';
	}
	if (flags & MOVE_PROMOTION_KNIGHT) {
		return 'n';
	}
	return ' ';
}

void move_displayAlg(Move *move)
{
	printf("%s%c%s%c", bitboard_binToAlg(SQ64(move->from)),
						(move->flags & MOVE_CAPTURE) ? 'x' : '-',
						bitboard_binToAlg(SQ64(move->to)),
						move_getPromotionPieceChar(move->flags));
}

