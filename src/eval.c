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
