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
	int material[10];
	Piece i;
	for (i=0; i < 10; i++) {
		material[i] = COUNT_MATERIAL(i);
	}

	// Material eval in centipawn;
	int score = 0;

	score += 100 * (material[P] - material[p]);
	score += 900 * (material[Q] - material[q]);
	score += 300 * (material[N] - material[n]);
	score += 300 * (material[B] - material[b]);
	score += 600 * (material[R] - material[r]);

	return score * who2move;
}

int eval_move(Move * move)
{
	return 1;
}
