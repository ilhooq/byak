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

#ifndef PIECES_H
#define PIECES_H

typedef enum enumPiece {
P, // Any white pawns
p, // Any black pawns
K, // White king
k, // Black king
Q, // Any white Quuens
q, // Any black Quuens
N, // Any white knights
n, // Any black knights
B, // Any white bishops
b, // Any black bishops
R, // Any white rooks
r,  // Any black rooks
NONE_PIECE
} Piece;

static char get_piece_letter(Piece piece)
{
	char c = ' ';
	switch (piece) {
		case P : c = 'P'; break;
		case K : c = 'K'; break;
		case Q : c = 'Q'; break;
		case N : c = 'N'; break;
		case B : c = 'B'; break;
		case R : c = 'R'; break;
		case p : c = 'p'; break;
		case k : c = 'k'; break;
		case q : c = 'q'; break;
		case n : c = 'n'; break;
		case b : c = 'b'; break;
		case r : c = 'r'; break;
		case NONE_PIECE : c = '0';
	}
	return c;
}

#endif
