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

#ifndef EVAL_H
#define EVAL_H

#include "types.h"
#include "move.h"

#define INFINITY 10000

typedef enum enumStage {
	OPENING,
	MIDDLEGAME,
	ENDGAME,
} Stage;

/*16 Bytes */
typedef struct {
	U64 mask;
	int score;
	Piece piece;
} Eval;

void eval_init();
int eval_position();
int eval_move(Move * move);
#endif
