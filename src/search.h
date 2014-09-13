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

#ifndef SEARCH_H
#define SEARCH_H

#define MAX_DEPTH 32

typedef struct {
	int time[2];
	int time_start;
	int time_used;
	int movetime;
	int my_side;
	int stop;
	int nodes;
	Move pv[MAX_DEPTH][MAX_DEPTH];
	int pv_length[MAX_DEPTH];
} SearchInfos;

void* search_start(void* data);
void search_stop();

Move ** search_get_pv();
int * search_get_pv_length();

void search_iterate();
int search_root(int alpha, int beta, U8 depth);
void search_root_negamax(int depth);
int search_negamax(int depth, int ply);
int search_alphaBeta(int alpha, int beta, int depth, int ply);
int search_quiesce(int alpha, int beta);

/* 
The total size of the triangular array in moves 
can be calculated by the Triangular number :
(MAX_DEPTH*MAX_DEPTH + MAX_DEPTH)/2
*/
// Move pv[528];

#endif
