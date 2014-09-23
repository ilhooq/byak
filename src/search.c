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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "tt.h"
#include "move.h"
#include "position.h"
#include "search.h"
#include "eval.h"
#include "time.h"
#include "uci.h"

static SearchInfos infos;

static void _updatePV(Move * mv, int ply)
{
	/*
	int pvIndex = 0.5 * ply * ((2*MAX_DEPTH) + 1 - ply);
	int pvNextIndex = pvIndex + MAX_DEPTH - ply;
	assert(pvIndex >= 0 && pvIndex <= 528);
	assert(pvNextIndex >= 0 && pvNextIndex <= 528);

	pv_array[pvIndex] = *mv;
	int infos.pv_length;
	Move * pTarget = pv_array + pvIndex + 1;
	Move * pSource = pv_array + pvNextIndex;
	for (infos.pv_length = MAX_DEPTH - ply - 1; infos.pv_length >=0; infos.pv_length--) {
		*pTarget++ = *pSource++;
	}
	*/

	int j;

	if (!infos.pv_length[ply]) {
		infos.pv_length[ply] = ply;
	}

	infos.pv[ply][ply] = *mv;
	for (j = ply + 1; j < infos.pv_length[ply + 1]; ++j)
		infos.pv[ply][j] = infos.pv[ply + 1][j];

	if (infos.pv_length[ply + 1])
		infos.pv_length[ply] = infos.pv_length[ply + 1];
	else if (!ply)
		infos.pv_length[0] = 1;

}

static void sortMoves(Move * movelist, int listlen, int ply)
{
	int i;
	Move temp;
	for (i=0; i < listlen; i++)  {
		// If the move in the list matches the first move in the PV...
		if (movelist[i].from == infos.pv[0][ply].from &&
			movelist[i].to == infos.pv[0][ply].to) {
			// ... we move it on the top of the stack.
			temp = movelist[0];
			movelist[0] = movelist[i];
			movelist[i] = temp;
			break;
		}
	}
}

static void timeControl()
{
	/*
	time_used = GET_TIME() - time_start
	timeleft = movetime - time_used;

	if (time_used > timeleft) we stop the search now

	Simplifying the equations:
	time_used > timeleft = timeused > (movetime - time_used) = (time_used * 2) > movetime

	*/
	if (!infos.movetime) return;

	infos.time_used = GET_TIME() - infos.time_start;

	if ((infos.time_used) * 2 > infos.movetime) {
		infos.stop = 1;
	}
}


void* search_start(void* data)
{
	SearchInfos * pInfos = data;
	// Dereference pointer
	infos = *pInfos;

	infos.time_start = GET_TIME();
	infos.my_side = pos.side;
	infos.stop = 0;

	if (!infos.depth) infos.depth = MAX_DEPTH;

	memset(infos.pv, 0, sizeof(infos.pv));
	memset(infos.pv_length, 0, sizeof(infos.pv_length));

	search_iterate();

	Move bestMove = infos.pv[0][0];

	uci_print_bestmove(&bestMove);

	return NULL;
}

void search_stop()
{
	infos.stop = 1;
}

void search_iterate()
{
	int depth;

	for (depth=1; depth <= infos.depth; depth++) {

		if (infos.stop) break;

		search_root(-INFINITY, INFINITY, depth);
	}
}

int search_root(int alpha, int beta, int depth)
{
	Move movelist[256];
	int score;

	int listLen = position_generateMoves(movelist);

	if (listLen == 1) {
		// If there is only one move, stop the search
		infos.stop = 1;
		infos.pv[0][0] = movelist[0];
	}

	sortMoves(movelist, listLen, 0);
	int i;

	for (i=0; i < listLen; i++)  {
		position_makeMove(&movelist[i]);

		score = -search_alphaBeta(-beta, -alpha, depth, 1);

		uci_print_currmove(&movelist[i],depth, i+1);
		uci_print_nps(infos.time_start, infos.nodes);

		position_undoMove(&movelist[i]);

		if (score > alpha && !infos.stop) {
			alpha = score;
			_updatePV(&movelist[i], 0);
			uci_print_pv(score, depth, &infos);
		}

	}

	return alpha;
}


int search_alphaBeta(int alpha, int beta, int depth, int ply)
{
	timeControl();

	if (infos.stop) return 0;

	if (depth == 0) {
		infos.nodes++;
		return search_quiesce(alpha, beta);
	}

	U16 tt_flag = TT_ALPHA;

	int tt_val = tt_probe(pos.hash, alpha, beta, depth);

	if (tt_val) return tt_val;

	Move movelist[256];
	int i, score;
	int listLen = position_generateMoves(movelist);

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
	int listLen = position_generateMoves(movelist);

	for (i=0; i < listLen; i++) {

		position_makeMove(&movelist[i]);
		score = -search_negamax(depth, 1);
		position_undoMove(&movelist[i]);

		if (score > max) {
			max = score;
			_updatePV(&movelist[i], 0);
			uci_print_pv(score, depth,&infos);
		}
	}
}

int search_negamax(int depth, int ply)
{
	if (depth == 0) return eval_position();

	int max = -INFINITY;

	int i, score;

	Move movelist[256];
	int listLen = position_generateMoves(movelist);

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

U64 search_perft(int depth)
{
	U64 nodes = 0;
	int val = tt_perft_probe(pos.hash, depth);
	int listLen = 0;
	int i;

	if (depth == 0) {
		return 1;
	}

	if (val) {
		return val;
	}

	Move movelist[256];
	listLen = position_generateMoves(movelist);

	if (depth == 1) {
		tt_perft_save(pos.hash, listLen, depth);
		return listLen;
	}

	for (i=0; i < listLen; i++) {
		position_makeMove(&movelist[i]);
		nodes += search_perft(depth - 1);
		position_undoMove(&movelist[i]);
	}

	tt_perft_save(pos.hash, nodes, depth);
	return nodes;
}

