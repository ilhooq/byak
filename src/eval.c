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

#include "bitboard.h"
#include "eval.h"
#include "position.h"
#include "move.h"

static const Eval w_opening[] = {
	// {RANK7                  , -60 , P },
	// {RANK6                  , -20 , P },

	{C64(0x0000000018000000),  25 , P }, // d4 e4
	{C64(0x0000000024000000),  15 , P }, // c4 f4
	// {C64(0x0000000024000000), -10 , P }, // b4 g4
	// {RANK3                  ,  20 , P },
	{C64(0x0000000000001800), -40 , P }, // d2 e2
	{(FILEA|FILEH) & ~RANK2 , -40 , P },
	{(FILEB|FILEG) & ~RANK2 , -30 , P },

	{C64(0x0000000000240000),  30 , N }, // c3 f3
	{C64(0x0000000000240000),  15 , N }, // d3 e3
	{(FILEA|FILEH)          , -25 , N },
	{RANK1                  , -15,  N },
	// {RANK6                  ,  30 , N },
	// {RANK7                  ,  10 , N },

	{C64(0x8142241818244281),  30 , B }, // a1 - h8 / a8 - h1 diagonals
	{C64(0x00000042245A0000),  20 , B }, // e3, d3, b3, g3, c4, f4, b5, g5
	{RANK1                  , -15 , B },

	{C64(0x0000000000000018),  25 , R }, // d1 e1
	{C64(0x0000000000000081), -20 , R }, // a1 h1

	{RANK1                  ,  25 , Q },

	{C64(0x0000000000000040),  25 , K }, // g1
	{C64(0x0000000000000004),  15 , K }, // c1
	{FILEE                  , -20 , K }, 
	{(FILED|FILEF|RANK2)    , -40 , K }, 
};

static Eval b_opening[sizeof(w_opening) / sizeof(Eval)];

static const Eval w_middlegame[] = {
	{RANK7                  , -40 , P },
	{RANK6                  , -20 , P },
	{RANK5                  ,  20 , P },
	{C64(0x0000000018000000),  50 , P }, // d4 e4
	{C64(0x0000000024000000),  30 , P }, // c4 f4
	{C64(0x0000000024000000), -10 , P }, // b4 g4
	{RANK3                  ,  20 , P },
	{C64(0x0000000000001800), -40 , P }, // d2 e2
	{(FILEA|FILEH) & ~RANK2 , -40 , P },
	{(FILEB|FILEG) & ~RANK2 , -30 , P },

	{(FILEA|FILEH)          , -50 , N },
	{(RANK1|RANK8)          , -30,  N },
	{RANK6                  ,  60 , N },
	{RANK7                  ,  80 , N },

	{C64(0x8142241818244281),  40 , B }, // a1 - h8 / a8 - h1 diagonals
	{C64(0x00000042245A0000),  20 , B }, // e3, d3, b3, g3, c4, f4, b5, g5
	{RANK1                  , -10 , B },

	{C64(0x0000000000000018),  25 , R }, // d1 e1
	{C64(0x0000000000000024),  25 , R }, // c1 f4
	{C64(0x0000000000000081), -20 , R }, // a1 h1

	// {RANK1                  ,  60 , Q },

	{C64(0x0000000000000040),  40 , K }, // g1
	{C64(0x0000000000000004),  30 , K }, // c1
	{(FILED|FILEF|RANK2|FILEE),-100, K }, 
};

static Eval b_middlegame[sizeof(w_middlegame) / sizeof(Eval)];

static const Eval w_endgame[] = {
	{RANK7                  ,  80 , P },
	{RANK6                  ,  60 , P },
	{RANK5                  ,  40 , P },
	{RANK4                  ,  20 , P },
	{RANK3                  ,  10 , P },
	{RANK2                  , -10 , P },

	{(FILEA|FILEH|RANK1|RANK8), -50 , N },
	{((FILEB|FILEG|RANK2|RANK7) & ~(FILEA|FILEH|RANK1|RANK8)) , -20 , N },

	{C64(0x8142241818244281),  40 , B }, // a1 - h8 / a8 - h1 diagonals

	{RANK7                  , 50 , R }, // a1 h1

	{C64(0x0000001818000000), 30 , K }, // d4 e4 e5 d5 
	{C64(0x00003C24243C0000), 30 , K }, // c3 -> f3 -> f6 -> c6
	{C64(0x007E424242427E00), 10 , K }, // b2 -> g2 -> g7 -> b7
	{RANK8                  , 10 , K },
	{RANK7                  , 20 , K },
	{RANK6                  , 30 , K },
	{RANK5                  , 30 , K }, // cumul the bonus of e5 d5 
	{RANK4                  , 20 , K }, // cumul the bonus of d4 e4
	{RANK3                  , -5 , K }, // cumul the bonus of c3 -> f3
	{RANK2                  ,-10 , K },
	{RANK1                  ,-30 , K },
};

static Eval b_endgame[sizeof(w_endgame) / sizeof(Eval)];


void eval_init()
{
	U32 i;

	for (i=0; i < sizeof(w_opening) / sizeof(Eval); i++) {
		b_opening[i].mask  = bitboard_swap(w_opening[i].mask);
		b_opening[i].score = w_opening[i].score;
		b_opening[i].piece = w_opening[i].piece + 1;
	}
}


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

	int score = 0; // Material evaluation in centipawn

	int popCnt[NONE_PIECE] = {0};
	int i, popCntTotal = 0;
	// Stage stage;
	U64 bb_pieces;

	for (i=P ; i < NONE_PIECE ; i++) {
		popCntTotal += popCnt[i] = bitboard_popCount(pos.bb_pieces[i]);
	}

	score += 100 * (popCnt[P] - popCnt[p]);
	score += 900 * (popCnt[Q] - popCnt[q]);
	score += 300 * (popCnt[N] - popCnt[n]);
	score += 300 * (popCnt[B] - popCnt[b]);
	score += 600 * (popCnt[R] - popCnt[r]);

	// Try to determine if we are in the opening, the middle game or the endgame
	if (pos.castling_rights && (popCntTotal - (popCnt[P] + popCnt[p])) > 10) {
		// Opening
		for (i=0; i < sizeof(w_opening) / sizeof(Eval); i++) {
			bb_pieces = pos.bb_pieces[w_opening[i].piece];
			if (w_opening[i].mask & bb_pieces) {
				score += bitboard_popCount(w_opening[i].mask & bb_pieces) * w_opening[i].score;
			}
			bb_pieces = pos.bb_pieces[b_opening[i].piece];
			if (b_opening[i].mask & bb_pieces) {
				score -= bitboard_popCount(b_opening[i].mask & bb_pieces) * b_opening[i].score;
			}
		}
	} 
	else if ((popCntTotal - (popCnt[P] + popCnt[p])) < 7) {
		// Engame
		for (i=0; i < sizeof(w_endgame) / sizeof(Eval); i++) {
			bb_pieces = pos.bb_pieces[w_endgame[i].piece];
			if (w_endgame[i].mask & bb_pieces) {
				score += bitboard_popCount(w_endgame[i].mask & bb_pieces) * w_endgame[i].score;
			}
			bb_pieces = pos.bb_pieces[b_opening[i].piece];
			if (b_opening[i].mask & bb_pieces) {
				score -= bitboard_popCount(b_endgame[i].mask & bb_pieces) * b_endgame[i].score;
			}
		}
	}
	else {
		// Midlegame
		for (i=0; i < sizeof(w_middlegame) / sizeof(Eval); i++) {
			bb_pieces = pos.bb_pieces[w_middlegame[i].piece];
			if (w_middlegame[i].mask & bb_pieces) {
				score += bitboard_popCount(w_middlegame[i].mask & bb_pieces) * w_middlegame[i].score;
			}
			bb_pieces = pos.bb_pieces[b_opening[i].piece];
			if (b_opening[i].mask & bb_pieces) {
				score -= bitboard_popCount(b_middlegame[i].mask & bb_pieces) * b_middlegame[i].score;
			}
		}
	}

	/* Todo : 
	- Minor Piece activity (good bishop vs bad bishop, knight outpost...)
	- Rooks controlling open files
	- pawn structure check
	*/

	return score * who2move;
}

int eval_move(Move * move)
{
	return 1;
}
