
#include <assert.h>
#include <stdio.h>
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

#include <string.h>
#include "types.h"
#include "tt.h"
#include "move.h"
#include "position.h"
#include "search.h"
#include "eval.h"


#define MAX_DEPTH 32

/* 
The total size of the triangular array in moves 
can be calculated by the Triangular number :
(MAX_DEPTH*MAX_DEPTH + MAX_DEPTH)/2
*/
// Move pv[528];

Move pv[MAX_DEPTH][MAX_DEPTH];

int pv_length[MAX_DEPTH];

static struct s_searchState {
	int stop;
	int nodes;
} sState;


static void _updatePV(Move * mv, int ply)
{
	/*
	int pvIndex = 0.5 * ply * ((2*MAX_DEPTH) + 1 - ply);
	int pvNextIndex = pvIndex + MAX_DEPTH - ply;
	assert(pvIndex >= 0 && pvIndex <= 528);
	assert(pvNextIndex >= 0 && pvNextIndex <= 528);

	pv_array[pvIndex] = *mv;
	int pv_length;
	Move * pTarget = pv_array + pvIndex + 1;
	Move * pSource = pv_array + pvNextIndex;
	for (pv_length = MAX_DEPTH - ply - 1; pv_length >=0; pv_length--) {
		*pTarget++ = *pSource++;
	}
	*/

	int j;

	if (!pv_length[ply]) {
		pv_length[ply] = ply;
	}

	pv[ply][ply] = *mv;
	for (j = ply + 1; j < pv_length[ply + 1]; ++j)
		pv[ply][j] = pv[ply + 1][j];

	if (pv_length[ply + 1])
		pv_length[ply] = pv_length[ply + 1];
	else if (!ply)
		pv_length[0] = 1;

}


static void _printPV(int score, int depth)
{
	/*
	int ply, pvIndex;
	for (ply=0; ply < MAX_DEPTH; ply++) {
		pvIndex = 0.5 * ply * ((2*MAX_DEPTH) + 1 - ply);
		if (pv_array[pvIndex].from == 0 && pv_array[pvIndex].to == 0) {
			break;
		}
		move_displayAlg(&pv_array[pvIndex]);
		printf(" ");
	}
	*/
	
	printf("info depth %i score cp %i", depth, score);

	printf(" pv ");

	int j;
	for (j = 0; j < pv_length[0]; ++j) {
		move_displayUCI(&pv[0][j]);
		printf(" ");
	}
	printf("\n");
}

static void printCurmove(Move * move, int depth, int n)
{
	printf("info depth %d currmove ", depth);
	move_displayUCI(move);
	printf(" currmovenumber %d\n",  n);
}

static void sortMoves(Move * movelist, U8 listlen, int ply)
{
	int i;
	Move temp;
	for (i=0; i < listlen; i++)  {
		// Si le coup de la liste correspond au premier coup de la PV
		if (movelist[i].from == pv[0][ply].from &&
			movelist[i].to == pv[0][ply].to) {
			// On le déplace en premier
			temp = movelist[0];
			movelist[0] = movelist[i];
			movelist[i] = temp;
			break;
		}
	}
	/*
	Move temp = m[high];
	m[high] = m[current];
	m[current] = temp;
	*/
}



void* search_start(void* data)
{
	sState.stop = 0;

	search_iterate();
	// search_root_negamax(4);

	printf("bestmove ");
	move_displayUCI(&pv[0][0]);
	printf("\n");

	return NULL;

}


void search_stop()
{
	sState.stop = 1;
}

void search_iterate()
{
	U8 depth;
	memset(pv, 0, sizeof(pv));
	memset(pv_length, 0, sizeof(pv_length));

	for (depth=1; depth <= MAX_DEPTH; depth++) {
		
		if (depth == 7) break;
		
		if (sState.stop) break;

		search_root(-INFINITY, INFINITY, depth);
	}
}

int search_root(int alpha, int beta, U8 depth)
{
	Move movelist[256];
	int score;

	U8 listLen = position_generateMoves(movelist);
	// if (depth > 2)
	sortMoves(movelist, listLen, 0);
	int i;

	for (i=0; i < listLen; i++)  {
		position_makeMove(&movelist[i]);

		score = -search_alphaBeta(-beta, -alpha, depth, 1);

		printCurmove(&movelist[i],depth, i+1);

		position_undoMove(&movelist[i]);

		if (score > alpha) {
			alpha = score;
			_updatePV(&movelist[i], 0);
			_printPV(score, depth);
		}
	}

	return alpha;
}


int search_alphaBeta(int alpha, int beta, int depth, int ply)
{
	if (depth == 0) {
		return search_quiesce(alpha, beta);
	}
	
	U16 tt_flag = TT_ALPHA;

	sState.nodes++;

	if (sState.stop) return 0;

	int tt_val = tt_probe(pos.hash, alpha, beta, depth);

	if (tt_val) return tt_val;

	Move movelist[256];
	int i, score;
	U8 listLen = position_generateMoves(movelist);
	// if (depth > 2)
	// sortMoves(movelist, listLen, ply);

	if (!listLen) {
		score = (pos.side == WHITE) ? INFINITY : -INFINITY;
		tt_save( pos.hash, score, depth, TT_EXACT);
		return score;
	}

	for (i=0; i < listLen; i++)  {
		position_makeMove(&movelist[i]);
		score = -search_alphaBeta(-beta, -alpha, depth - 1, ply + 1);
		position_undoMove(&movelist[i]);

		if (score >= beta) {
			//  fail hard beta-cutoff
			tt_save( pos.hash, beta, depth, TT_BETA);
			return beta;
		}

		if (score > alpha) {
			// alpha acts like max in MiniMax
			alpha = score;
			tt_flag = TT_EXACT;
			_updatePV(&movelist[i], ply);
		}
	}

	tt_save( pos.hash, alpha, depth, tt_flag);

	return alpha;
}

int search_quiesce(int alpha, int beta)
{
	return eval_position();
}

void search_root_negamax(int depth)
{
	int i, score;
	int max = -INFINITY;

	Move movelist[256];
	U8 listLen = position_generateMoves(movelist);
	Move moveToMake;

	for (i=0; i < listLen; i++) {
		
		position_makeMove(&movelist[i]);
		score = -search_negamax(depth, 1);
		position_undoMove(&movelist[i]);

		if (score > max) {
			max = score;
			_updatePV(&movelist[i], 0);
			_printPV(score, depth);
			moveToMake = movelist[i];
		}
	}
	move_displayAlg(&moveToMake);
	printf("\nScore final : %i\n", max);
}

int search_negamax(int depth, int ply)
{
	if (depth == 0) return eval_position();

	int max = -INFINITY;

	int i, score;

	Move movelist[256];
	U8 listLen = position_generateMoves(movelist);

	for (i=0; i < listLen; i++) {
		position_makeMove(&movelist[i]);
		score = -search_negamax(depth - 1, ply + 1);
		position_undoMove(&movelist[i]);

		if ( score > max ) {
			max = score;
			_updatePV(&movelist[i], ply);
		}
	}

	return max;
}

