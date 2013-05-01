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

#include "eval.h"
#include "position.h"
#include "move.h"

#define COUNT_MATERIAL(pieceType) bitboard_IPopCount(pos.bb_pieces[(pieceType)])

/* side to evaluate
static int who2move = 1;

void eval_init(U8 side)
{
	who2move = (side == WHITE) ? 1 : -1;
}
 */

int eval_position()
{
	
	/**
	* Note! In order for negaMax to work,  the Static Evaluation 
	* function must return a score relative to the side to being 
	* evaluated, and this score must be symmetric 
	* (e.g. the simplest score evaluation could be: 
	* score = materialWeight * (numWhitePieces - numBlackPieces) * who2move 
	* //where who2move = 1 for white, and who2move = -1 for black).
	*/
	int who2move = (pos.side == WHITE) ? 1 : -1;

/*
	if (pos.checkmated) {
		if (pos.side == WHITE) {
			return -INFINITY-1;
		}
		return INFINITY+1;
	}*/

	
	// Material eval;
	int score = 0;

	score += 10 * (COUNT_MATERIAL(P) - COUNT_MATERIAL(p));
	score += 90 * (COUNT_MATERIAL(Q) - COUNT_MATERIAL(q));
	score += 30 * (COUNT_MATERIAL(N) - COUNT_MATERIAL(n));
	score += 30 * (COUNT_MATERIAL(B) - COUNT_MATERIAL(b));
	score += 60 * (COUNT_MATERIAL(R) - COUNT_MATERIAL(r));

	return score * who2move;
}

int eval_move(Move * move)
{
	return 1;
}
