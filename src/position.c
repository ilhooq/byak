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
#include <stdlib.h>
#include <string.h>
/*#include "util.h"*/
#include "bitboard.h"
#include "magicmoves.h"
#include "position.h"
#include "move.h"
#include "tt.h"

#define POS_ADD_PIECE(piece, sq) pos.bb_pieces[(piece)] |= SQ64((sq)); pos.hash ^= zobrist.piecesquare[(piece)][(sq)]

#define POS_DEL_PIECE(piece, sq) pos.bb_pieces[(piece)] ^= SQ64((sq)); pos.hash ^= zobrist.piecesquare[(piece)][(sq)]

#define POS_MOVE_PIECE(piece, sq_from, sq_to) POS_DEL_PIECE(piece, sq_from); POS_ADD_PIECE(piece, sq_to);

#define POS_ADD_ATTACK(from_square, to_square) \
	pos.attacks_from[bitboard_bitScanForward(from_square)] |= to_square; \
	pos.attacks_to[bitboard_bitScanForward(to_square)] |= from_square;


Position pos;

static int movelistcount;

void position_init()
{
	memset(pos.bb_pieces, 0, sizeof(pos.bb_pieces));

	pos.bb_side[WHITE] = EMPTY;
	pos.bb_side[BLACK] = EMPTY;
	pos.bb_occupied = EMPTY;
	pos.bb_empty = EMPTY;

	pos.pinned = EMPTY;

	pos.in_check = 0;
	pos.checkmated = 0;

	pos.last_double = EMPTY;
	pos.enpassant = NONE_SQUARE;
	pos.castling_rights = 0;
	pos.side = WHITE; // White To Move

	memset(pos.attacks_from, 0, sizeof(pos.attacks_from));
	memset(pos.attacks_to, 0, sizeof(pos.attacks_to));
	memset(pos.pinner, 0, sizeof(pos.pinner));

	pos.hash = EMPTY;
	movelistcount=0;
}


void position_display()
{
	int i=0, rankIndex = 7, fileIndex = 0, offset= 0, white = 0;
	char buffer[700]="";

	for( i=0; i < TOTAL_SQUARES; i++) {
		if (fileIndex == 0) {
			strcat(buffer, "\n    +---+---+---+---+---+---+---+---+\n");
			char rankIdx[6];
			sprintf(rankIdx, "  %i |", rankIndex+1);
			strcat(buffer, rankIdx);
		}

		offset = 8*rankIndex + fileIndex;

		if (pos.bb_pieces[P] & C64(1) << offset) {
			strcat(buffer, " P |");
		} else if (pos.bb_pieces[K] & C64(1) << offset) {
			strcat(buffer, " K |");
		} else if (pos.bb_pieces[Q] & C64(1) << offset) {
			strcat(buffer, " Q |");
		} else if (pos.bb_pieces[N] & C64(1) << offset) {
			strcat(buffer, " N |");
		} else if (pos.bb_pieces[B] & C64(1) << offset) {
			strcat(buffer, " B |");
		} else if (pos.bb_pieces[R] & C64(1) << offset) {
			strcat(buffer, " R |");
		} else if (pos.bb_pieces[p] & C64(1) << offset) {
			strcat(buffer, " p |");
		} else if (pos.bb_pieces[k] & C64(1) << offset) {
			strcat(buffer, " k |");
		} else if (pos.bb_pieces[q] & C64(1) << offset) {
			strcat(buffer, " q |");
		} else if (pos.bb_pieces[n] & C64(1) << offset) {
			strcat(buffer, " n |");
		} else if (pos.bb_pieces[b] & C64(1) << offset) {
			strcat(buffer, " b |");
		} else if (pos.bb_pieces[r] & C64(1) << offset) {
			strcat(buffer, " r |");
		} else if (white) {
			strcat(buffer, "   |");
		} else {
			strcat(buffer, " . |");
		}

		white ^= 1;
		fileIndex++;

		if (fileIndex > 7) {
			rankIndex -=1;
			fileIndex = 0;
			white ^= 1;
		}
	}

	strcat(buffer, "\n    +---+---+---+---+---+---+---+---+\n");
	strcat(buffer, "      a   b   c   d   e   f   g   h\n");
	printf("%s", buffer);
}

int position_fromFen(const char *fen)
{
	int length = strlen(fen);
	int i = 0, part = 0, rankIndex = 7, fileIndex = 0, squareIndex = 0;
	U64 enPassantTarget = EMPTY;

	for (i=0; i < length; i++) {

		if (fen[i] == ' ') {
			part++;
			continue;
		}

		switch (part) {
			case  0:
				squareIndex = 8*rankIndex + fileIndex;
				switch(fen[i]) {
					case '/':
						rankIndex -=1;
						fileIndex = 0;
						break;
					case '1': fileIndex += 1; break;
					case '2': fileIndex += 2; break;
					case '3': fileIndex += 3; break;
					case '4': fileIndex += 4; break;
					case '5': fileIndex += 5; break;
					case '6': fileIndex += 6; break;
					case '7': fileIndex += 7; break;
					case '8': fileIndex += 8; break;
					case 'P': POS_ADD_PIECE(P, squareIndex); fileIndex++; break;
					case 'K': POS_ADD_PIECE(K, squareIndex); fileIndex++; break;
					case 'Q': POS_ADD_PIECE(Q, squareIndex); fileIndex++; break;
					case 'N': POS_ADD_PIECE(N, squareIndex); fileIndex++; break;
					case 'B': POS_ADD_PIECE(B, squareIndex); fileIndex++; break;
					case 'R': POS_ADD_PIECE(R, squareIndex); fileIndex++; break;
					case 'p': POS_ADD_PIECE(p, squareIndex); fileIndex++; break;
					case 'k': POS_ADD_PIECE(k, squareIndex); fileIndex++; break;
					case 'q': POS_ADD_PIECE(q, squareIndex); fileIndex++; break;
					case 'n': POS_ADD_PIECE(n, squareIndex); fileIndex++; break;
					case 'b': POS_ADD_PIECE(b, squareIndex); fileIndex++; break;
					case 'r': POS_ADD_PIECE(r, squareIndex); fileIndex++; break;
					default:
						return -1;
				}
				break;
			case 1:
				pos.side = (fen[i] == 'w') ? WHITE : BLACK;
				pos.hash ^= zobrist.side;
				break;
			case 2:
				switch(fen[i]) {
					case 'K':
						pos.castling_rights |= W_CASTLE_K;
						break;
					case 'Q':
						pos.castling_rights |= W_CASTLE_Q;
						break;
					case 'k':
						pos.castling_rights |= B_CASTLE_K;
						break;
					case 'q':
						pos.castling_rights |= B_CASTLE_Q;
						break;
				}
				pos.hash ^= zobrist.castling[pos.castling_rights];
				break;
			case 3:
				if (pos.enpassant != NONE_SQUARE) {
					// Break the test if enpassant is set
					break;
				}
				enPassantTarget = bitboard_algToBin(&(fen[i]));
				if (enPassantTarget & RANK6) {
					pos.last_double = bitboard_soutOne(enPassantTarget);
				}
				if (enPassantTarget & RANK3) {
					pos.last_double = bitboard_nortOne(enPassantTarget);
				}
				if (pos.last_double) {
					pos.enpassant = bitboard_bitScanForward(enPassantTarget);
					pos.hash ^= zobrist.ep[pos.enpassant];
				}
				break;
		}

	}
	position_refresh();

	return 0;
}



void position_refresh()
{
	memset(pos.attacks_from, 0, sizeof(pos.attacks_from));
	memset(pos.attacks_to, 0, sizeof(pos.attacks_to));
	memset(pos.pinner, 0, sizeof(pos.pinner));

	pos.pinned = EMPTY;
	pos.in_check = 0;
	pos.checkmated = 0;

	pos.bb_side[WHITE] = pos.bb_pieces[P] | pos.bb_pieces[K] |
				 pos.bb_pieces[Q] | pos.bb_pieces[N] |
				 pos.bb_pieces[B] | pos.bb_pieces[R];

	pos.bb_side[BLACK] = pos.bb_pieces[p] | pos.bb_pieces[k] |
				 pos.bb_pieces[q] | pos.bb_pieces[n] |
				 pos.bb_pieces[b] | pos.bb_pieces[r];

	pos.bb_occupied = pos.bb_side[WHITE] | pos.bb_side[BLACK];
	pos.bb_empty = ~pos.bb_occupied;
}

void position_makeMove(Move *move)
{
	U64 bb_from   = SQ64(move->from);
	U64 bb_to     = SQ64(move->to);
	Piece pieceFrom = NONE_PIECE;
	int i = 0;

	pos.last_double = EMPTY;
	
	move->castling_rights = pos.castling_rights;

	// move->prevHash = pos.hash;

	/* Determine which piece type to move */
	Piece piece[12] = {P,K,Q,N,B,R,p,k,q,n,b,r};
	for (i=0; i < 12; i++) {
		if (pos.bb_pieces[piece[i]] & bb_from) {
			pieceFrom = piece[i];
		}
		if (pos.bb_pieces[piece[i]] & bb_to) {
			move->captured_piece = piece[i];
		}
	}
	
	/*
	Piece piece[6] = {P,K,Q,N,B,R};
	int other_side = 1 ^ pos.side;
	Piece tmp1 = NONE_PIECE;
	Piece tmp2 = NONE_PIECE;
	for (i=0; i < 6; i++) {
		tmp1 = piece[i] + pos.side;
		tmp2 = piece[i] + other_side;
		if (pos.bb_pieces[tmp1] & bb_from) {
			pieceFrom = tmp1;
		}
		if (pos.bb_pieces[tmp2] & bb_to) {
			move->captured_piece = tmp2;
		}
	}
	*/
	/* Move the piece in one instruction : */
	pos.bb_pieces[pieceFrom] ^=  bb_from ^ bb_to;
	pos.hash ^= zobrist.piecesquare[pieceFrom][move->from];
	pos.hash ^= zobrist.piecesquare[pieceFrom][move->to];

	if (pos.enpassant != NONE_SQUARE) {
		/* 
		* Backup the "en passant" state into the move in order to 
		* retrieve the old state when this move will be undone
		*/
		move->ep = pos.enpassant;
		pos.hash ^= zobrist.ep[pos.enpassant];
		pos.enpassant = NONE_SQUARE;
	}

	if (move->capture) {
		if (move->type == ENPASSANT) {
			if (bb_to & RANK6) {
				/* We're dealing with white capturing black */
				POS_DEL_PIECE(p, move->to - 8);
			}
			if (bb_to & RANK3) {
				/* We're dealing with black capturing white */
				POS_DEL_PIECE(P, move->to + 8);
			}
		} else {
			POS_DEL_PIECE(move->captured_piece, move->to);
		}
	}

	/* castle flags
	 * if either a king or a rook leaves its initial square, the side looses its castling-right.
	 * The same happens if another piece moves to pos square (eg.: captures a rook on its initial square)
	 */
	switch (move->from) {
		case h1: pos.castling_rights &= ~W_CASTLE_K; break;
		case e1: pos.castling_rights &= ~(W_CASTLE_K|W_CASTLE_Q); break;
		case a1: pos.castling_rights &= ~W_CASTLE_Q; break;
		case h8: pos.castling_rights &= ~B_CASTLE_K; break;
		case e8: pos.castling_rights &= ~(B_CASTLE_K|B_CASTLE_Q); break;
		case a8: pos.castling_rights &= ~B_CASTLE_Q; break;
		default:break;
	}
	switch (move->to) {
		case h1: pos.castling_rights &= ~W_CASTLE_K; break;
		case e1: pos.castling_rights &= ~(W_CASTLE_K|W_CASTLE_Q); break;
		case a1: pos.castling_rights &= ~W_CASTLE_Q; break;
		case h8: pos.castling_rights &= ~B_CASTLE_K; break;
		case e8: pos.castling_rights &= ~(B_CASTLE_K|B_CASTLE_Q); break;
		case a8: pos.castling_rights &= ~B_CASTLE_Q; break;
		default:break;
	}
	pos.hash ^= zobrist.castling[move->castling_rights];
	pos.hash ^= zobrist.castling[pos.castling_rights];

	if (move->type == PROMOTION) {
		POS_DEL_PIECE(P + pos.side, move->to);
		POS_ADD_PIECE(move->promoted_piece, move->to);
	}
	else if (move->type == CASTLE) {
		switch (move->to) {
			case g1 : 
				POS_MOVE_PIECE(R, h1, f1);
				break;
			case c1 :
				POS_MOVE_PIECE(R, a1, d1);
				break;
			case g8 :
				POS_MOVE_PIECE(r, h8, f8);
				break;
			case c8 :
				POS_MOVE_PIECE(r, a8, d8);
				break;
		}
	}
	else if (move->type == PAWN_DOUBLE) {
		pos.last_double = bb_to;
		// Activate new enPassant
		pos.enpassant = (move->from + move->to) / 2;
		pos.hash ^= zobrist.ep[pos.enpassant];
	}

	/* switch side to move */
	pos.side = 1 ^ pos.side;
	pos.hash ^= zobrist.side;

	position_refresh();
}

void position_undoMove(Move *move)
{
	// int otherSide = 1 ^ pos.side;
	U64 from_square = SQ64(move->to);
	U64 to_square =  SQ64(move->from);
	U64 bb_fromTo = to_square | from_square;
	Piece piece[12] = {P,K,Q,N,B,R,p,k,q,n,b,r};
	int i = 0;

	Piece pieceFrom = NONE_PIECE;

	pos.side = 1 ^ pos.side;
	pos.hash ^= zobrist.side;

	/* Determine which piece type to move */
	for (i=0; i < 12; i++) {
		if (pos.bb_pieces[piece[i]] & from_square) {
			pieceFrom = piece[i];
			pos.bb_pieces[pieceFrom] ^=  bb_fromTo;
			break;
		}
	}
	pos.hash ^= zobrist.piecesquare[pieceFrom][move->from];
	pos.hash ^= zobrist.piecesquare[pieceFrom][move->to];


	if (pos.enpassant != NONE_SQUARE) {
		// Deactivate En passant
		pos.hash ^= zobrist.ep[pos.enpassant];
	}
	
	if (move->ep != NONE_SQUARE) {
		// Reactivate old state En passant
		pos.hash ^= zobrist.ep[move->ep];
	}
	pos.enpassant = move->ep;

	if (move->capture) {
		if (move->type == ENPASSANT) {
			if (from_square & RANK6) {
				/* Restore white's black pawn capture */
				POS_ADD_PIECE(p, move->to - 8);
			}
			else if (from_square & RANK3) {
				/* Restore black's white pawn capture */
				POS_ADD_PIECE(P, move->to + 8);
			}

		} else {
			POS_ADD_PIECE(move->captured_piece, move->to);
		}
	}

	if (move->type == PROMOTION) {
		POS_DEL_PIECE(move->promoted_piece, move->from);
		POS_ADD_PIECE(P + pos.side, move->from);
	}
	else if (move->type == CASTLE) {
		switch (move->to) {
			case g1 : 
				POS_MOVE_PIECE(R, f1, h1);
				break;
			case c1 :
				POS_MOVE_PIECE(R, d1, a1);
				break;
			case g8 :
				POS_MOVE_PIECE(r, f8, h8);
				break;
			case c8 :
				POS_MOVE_PIECE(r, d8, a8);
				break;
		}
	}
	else if (move->type == PAWN_DOUBLE) {
		pos.last_double = EMPTY;
	}

	pos.hash ^= zobrist.castling[pos.castling_rights];
	pos.hash ^= zobrist.castling[move->castling_rights];
	pos.castling_rights = move->castling_rights;
	// pos.hash = move->prevHash;

	position_refresh();
}

void position_listAdd( Move *movelist, U64 from_square, U64 to_square, 
TypeMove type, char capture, Piece promoted_piece, Piece captured_piece)
{
	Move * move = &movelist[movelistcount];
	move->from = bitboard_bitScanForward(from_square);
	move->to = bitboard_bitScanForward(to_square);
	move->type = type;
	move->capture = (capture)? capture : 0;
	move->promoted_piece = (promoted_piece)? promoted_piece : NONE_PIECE;
	// These will be determined when making move
	move->captured_piece = NONE_PIECE;
	move->ep = NONE_SQUARE;
	move->castling_rights = 0;
	movelistcount++;
}


int position_generateMoves(Move *movelist)
{
	int otherSide = 1 ^ pos.side;
	U64 king = pos.bb_pieces[K + pos.side];
	U64 our_pieces = pos.bb_side[pos.side];
	U64 other_pieces = pos.bb_side[otherSide];

	U64 pieces = EMPTY;
	U64 pawns = EMPTY;
	U64 rank45 = EMPTY;
	U64 otherQueenRooks = EMPTY;
	U64 rank18 = RANK1 | RANK8;
	U64 pinned = EMPTY;
	U64 pinner = EMPTY;
	movelistcount = 0;

	/*
	check to make sure we have a king.
	If we don't, we shouldn't be here.
	*/
	if (king == 0) {
		printf("Error: one of the kings is missing.\n");
		return movelistcount;
	}

	position_generatePinned();
	position_generateAttacks();
	position_generatePawnsAttacks();

	/* Are we in check?! */
	if (pos.attacks_to[bitboard_bitScanForward(king)] & other_pieces) {
		/* we are in check, find check evasions to get out */
		pos.in_check = 1;
		position_generateCheckEvasions(movelist);

		if (!movelistcount) { 
			/* No moves... King is checkmated :( */
			pos.checkmated = 1;
		}

		return movelistcount ;
	}

	/*
	At pos point we already know the attacks_from for all
	pieces...we just need to remove our own pieces from the attack.
	*/
	if (pos.side == WHITE) {
		pieces = pos.bb_side[WHITE];
		pawns = pos.bb_pieces[P];
		rank45 = RANK5;
		otherQueenRooks = pos.bb_pieces[r] | pos.bb_pieces[q];
	}
	else {
		pieces = pos.bb_side[BLACK];
		pawns = pos.bb_pieces[p];
		rank45 = RANK4;
		otherQueenRooks = pos.bb_pieces[R] | pos.bb_pieces[Q];
	}

	U64 from_square = EMPTY;
	U64 to_square = EMPTY;
	U64 moves = EMPTY;

	U64 enpassant_mask = (pos.enpassant != NONE_SQUARE) ? C64(1) << pos.enpassant : EMPTY;

	while (pieces) {
		from_square = LS1B(pieces);
		moves = pos.attacks_from[bitboard_bitScanForward(from_square)]
				& (other_pieces | pos.bb_empty);
		while (moves) {
			to_square = LS1B(moves);

			if (!position_canMove(from_square, to_square)) {
				moves = RESET_LS1B(moves);
				continue;
			}

			/* En passant capture */
			if (to_square == enpassant_mask) {
				position_listAdd(movelist, from_square, to_square, ENPASSANT,1,0,0);
			}

			/* Normal capture */
			else if (to_square & other_pieces) {

				if ((from_square & pawns) && (rank18 & to_square)) {
					/* We have a capture and promotion */
					position_generatePromotionMoves(movelist, from_square, to_square, 1);
				}

				else {
					position_listAdd(movelist, from_square, to_square, NORMAL,1,0,0);
				}
			}

			/* Empty square */
			else if (from_square & ~pawns) {
				position_listAdd(movelist, from_square, to_square, NORMAL,0,0,0);
			}

			moves = RESET_LS1B(moves);
		}
		pieces = RESET_LS1B(pieces);
	}

	/* Generate castling moves */
	U64 attacked_ks = EMPTY;
	U64 attacked_qs = EMPTY;
	U64 occupied_ks = EMPTY;
	U64 occupied_qs = EMPTY;
	if (pos.side == WHITE && (pos.castling_rights & (W_CASTLE_K|W_CASTLE_Q))) {
		attacked_ks = (pos.attacks_to[g1] | pos.attacks_to[f1])
						& other_pieces;
		/* If b1 is attacked, it's not a problem to make the Queen side rock */
		attacked_qs = (pos.attacks_to[c1] | pos.attacks_to[d1])
						& other_pieces;
		occupied_ks = (SQ64(g1) | SQ64(f1)) & pos.bb_occupied;
		occupied_qs = (SQ64(b1) | SQ64(c1) | SQ64(d1)) & pos.bb_occupied;
		if ((pos.castling_rights & W_CASTLE_K) &&
			pos.bb_pieces[R] & SQ64(h1) &&
			!attacked_ks && !occupied_ks) {
			position_listAdd(movelist, king, SQ64(g1), CASTLE,0,0,0);
		}

		if ((pos.castling_rights & W_CASTLE_Q) &&
			pos.bb_pieces[R] & SQ64(a1) &&
			!attacked_qs && !occupied_qs) {
			position_listAdd(movelist, king, SQ64(c1), CASTLE,0,0,0);
		}
	}
	else if (pos.side == BLACK && (pos.castling_rights & (B_CASTLE_K|B_CASTLE_Q))) {
		attacked_ks = (pos.attacks_to[g8] | pos.attacks_to[f8]) 
					& other_pieces;
		/* If b8 is attacked, it's not a problem to make the Queen side rock */
		attacked_qs = (pos.attacks_to[c8] |
					   pos.attacks_to[d8]) & other_pieces;
		occupied_ks = (SQ64(g8) | SQ64(f8)) & pos.bb_occupied;
		occupied_qs = (SQ64(b8) | SQ64(c8) | SQ64(d8)) & pos.bb_occupied;
		if ((pos.castling_rights & B_CASTLE_K) &&
			pos.bb_pieces[r] & SQ64(h8) &&
			!attacked_ks && !occupied_ks) {
			position_listAdd(movelist, king, SQ64(g8), CASTLE,0,0,0);
		}

		if ((pos.castling_rights & B_CASTLE_Q) &&
			pos.bb_pieces[r] & SQ64(a8) &&
			! attacked_qs && !occupied_qs) {
			position_listAdd(movelist, king, SQ64(c8), CASTLE,0,0,0);
		}
	}
	/*
	* produce single pawn moves...these are not handled in generate_attacks
	* since pawns can only attack diagonally (or enpassant)
	*/
	// pinned = pos.pinned & ~position_getFilePinned(pos.bb_occupied, our_pieces);
	U64 singlePushs = EMPTY;
	U64 doublePushs = EMPTY;

	if (pos.side == WHITE) {
		singlePushs = bitboard_nortOne(pos.bb_pieces[P]) & pos.bb_empty;
		doublePushs = bitboard_nortOne(singlePushs) & pos.bb_empty & RANK4;
	}
	else {
		singlePushs = bitboard_soutOne(pos.bb_pieces[p]) & pos.bb_empty;
		doublePushs = bitboard_soutOne(singlePushs) & pos.bb_empty & RANK5;
	}

	while (singlePushs) {
		to_square = LS1B(singlePushs);
		if (pos.side == WHITE)
			from_square = to_square >> 8;
		else
			from_square = to_square << 8;

		if ((from_square & pos.pinned) && position_canMove(from_square, to_square)) {
			pos.pinned ^= from_square;
		}

		if (from_square & ~pos.pinned) {
			/* Check promotion */
			if (to_square & rank18)
				position_generatePromotionMoves(movelist, from_square, to_square, 0);
			else
				position_listAdd(movelist, from_square, to_square, NORMAL,0,0,0);
		}
		singlePushs = RESET_LS1B(singlePushs);
	}

	while (doublePushs) {
		to_square = LS1B(doublePushs);
		if (pos.side == WHITE)
			from_square = to_square >> 16;
		else
			from_square = to_square << 16;
		if (from_square & ~pos.pinned) {
			position_listAdd(movelist, from_square, to_square, PAWN_DOUBLE,0,0,0);
		}
		doublePushs = RESET_LS1B(doublePushs);
	}

	return movelistcount;
}

void position_generateAttacks()
{
	U64 knights = pos.bb_pieces[n] | pos.bb_pieces[N];
	U64 kings = pos.bb_pieces[k] | pos.bb_pieces[K];
	U64 queen_rooks = pos.bb_pieces[Q] | pos.bb_pieces[q] | pos.bb_pieces[R] | pos.bb_pieces[r];
	U64 queen_bishops = pos.bb_pieces[Q] | pos.bb_pieces[q] | pos.bb_pieces[B] | pos.bb_pieces[b];
	U64 non_pawns = pos.bb_occupied & ~pos.bb_pieces[P] & ~pos.bb_pieces[p];
	U64 from_square = EMPTY;
	U64 to_square = EMPTY;
	// U64 rank_pieces = EMPTY;
	// U64 file_pieces = EMPTY;
	// U64 ne_pieces = EMPTY;
	// U64 nw_pieces = EMPTY;
	U64 moves = EMPTY;

	while (non_pawns) {
		from_square = LS1B(non_pawns);

		if (from_square & kings) {
			moves |= bitboard_getKingMoves(from_square & kings);
		}

		if (from_square & knights) {
			moves |= bitboard_getKnightMoves(from_square & knights);
		}

		if (from_square & queen_rooks) {
			// rank_pieces = bitboard_getRank(from_square & queen_rooks) & pos.bb_occupied;
			// file_pieces = bitboard_getFile(from_square & queen_rooks) & pos.bb_occupied;
			//moves |= RmagicNOMASK(bitboard_bitScanForward(from_square & queen_rooks), file_pieces | rank_pieces);
			// Time Consuming
			moves |= Rmagic(bitboard_bitScanForward(from_square & queen_rooks), pos.bb_occupied);
		}

		if (from_square & queen_bishops) {
			// ne_pieces = bitboard_getDiagNE(from_square & queen_bishops) & pos.bb_occupied;
			// nw_pieces = bitboard_getDiagNW(from_square & queen_bishops) & pos.bb_occupied;
			// moves |= BmagicNOMASK(bitboard_bitScanForward(from_square & queen_bishops), ne_pieces | nw_pieces);
			// Time Consuming
			moves |= Bmagic(bitboard_bitScanForward(from_square & queen_bishops), pos.bb_occupied);
		}

		while (moves) {
			to_square = LS1B(moves);
			POS_ADD_ATTACK(from_square, to_square);
			moves = RESET_LS1B(moves);
		}

		non_pawns = RESET_LS1B(non_pawns);
	}
}

void position_generatePawnsAttacks()
{
	/* produce pawn attacks. */
	U64 from_square = EMPTY;
	U64 to_square = EMPTY;
	U64 left_attack = EMPTY;
	U64 right_attack = EMPTY;
	
	int side = 0;
	for (side=0; side < 2; side++) {
		if (side == WHITE) {
			left_attack  = bitboard_noWeOne(pos.bb_pieces[P]);
			right_attack = bitboard_noEaOne(pos.bb_pieces[P]);
		}
		else {
			left_attack  = bitboard_soWeOne(pos.bb_pieces[p]);
			right_attack = bitboard_soEaOne(pos.bb_pieces[p]);
		}

		while (left_attack) {
			to_square = LS1B(left_attack);
			from_square = (side == WHITE) ? to_square >> 7 : to_square << 9;
			POS_ADD_ATTACK(from_square, to_square);
			left_attack = RESET_LS1B(left_attack);
		}

		while (right_attack) {
			to_square = LS1B(right_attack);
			from_square = (side == WHITE) ? to_square >> 9 : to_square << 7;
			POS_ADD_ATTACK(from_square, to_square);
			right_attack = RESET_LS1B(right_attack);
		}
	}
}

void position_generateCheckEvasions( Move *movelist)
{
	/* pos should only get called if we're in check */
	assert(pos.in_check);
	int otherSide = 1 ^ pos.side;

	U64 our_pieces = pos.bb_side[pos.side];
	U64 other_pieces = pos.bb_side[otherSide];
	U64 king = pos.bb_pieces[K + pos.side];
	U64 king_attackers = pos.attacks_to[bitboard_bitScanForward(king)] & other_pieces;
	U64 king_attacker = EMPTY;
	int num_attackers = bitboard_IPopCount(king_attackers);

	U64 pawns = EMPTY;
	
	U64 rank45 = EMPTY;
	U64 our_attacking_pieces = EMPTY;
	U64 rank18 = RANK1 | RANK8;
	U64 from_square = EMPTY;
	U64 to_square = EMPTY;


	// We can either
	// 1. capture the attacking piece
	// 2. block its path
	// 3. move out of the way

	if (num_attackers == 1) {
		U64 blockers = EMPTY;
		int enPassant = 0;
		// U64 occupancy = EMPTY;
		// U64 pinned = EMPTY;
		king_attacker = king_attackers;
		/*
		###########################################################
		1. Try to capture the attacking piece
		###########################################################
		*/
		our_attacking_pieces = pos.attacks_to[bitboard_bitScanForward(king_attacker)] 
								& pos.bb_side[pos.side];

		if (our_attacking_pieces & king) {
			// The king capture is processed in the Move out the way part
			our_attacking_pieces ^= king;
		}
		
		pawns = pos.bb_pieces[P + pos.side];
		rank45 = (pos.side == WHITE) ? RANK5 : RANK4;

		while (our_attacking_pieces) {
			from_square = LS1B(our_attacking_pieces);
			to_square = pos.attacks_from[bitboard_bitScanForward(from_square)] & king_attacker;
			enPassant = 0;

			/* For en passant capture, change the destination square */
			if ((to_square & pos.last_double) && (from_square & rank45 & pawns)) {
				enPassant = 1;
				if (pos.side == WHITE) {
					to_square = bitboard_nortOne(to_square);
				}
				else {
					to_square = bitboard_soutOne(to_square);
				}
			}

			if (((to_square & king_attacker) || enPassant) && position_canMove(from_square, to_square)) {
				// Capture
				if (enPassant) {
					position_listAdd(movelist, from_square, to_square, ENPASSANT,1,0,0);
				} else if ((from_square & pawns) && (rank18 & to_square)) {
					// We have a capture and promotion
					position_generatePromotionMoves(movelist, from_square, to_square, 1);
				} else {
					position_listAdd(movelist, from_square, to_square, NORMAL,1,0,0);
				}
			}

			our_attacking_pieces = RESET_LS1B(our_attacking_pieces);
		}

		/*
		############################################################
		# 2. Try to block the attacking ray
		############################################################
		*/
		U64 obstructed = EMPTY;
		U64 singlePushs = EMPTY;
		U64 doublePushs = EMPTY;

		if (pos.side == WHITE) {
			our_pieces = pos.bb_pieces[R] | pos.bb_pieces[B]
						  | pos.bb_pieces[Q] | pos.bb_pieces[N];
			singlePushs = bitboard_nortOne(pos.bb_pieces[P]) & pos.bb_empty;
			doublePushs = bitboard_nortOne(singlePushs) & pos.bb_empty & RANK4;
		}
		else {
			our_pieces = pos.bb_pieces[r] | pos.bb_pieces[b]
						  | pos.bb_pieces[q] | pos.bb_pieces[n];
			singlePushs = bitboard_soutOne(pos.bb_pieces[p]) & pos.bb_empty;
			doublePushs = bitboard_soutOne(singlePushs) & pos.bb_empty & RANK5;
		}

		obstructed = bitboard_getObstructed(king_attacker, king);

		while (singlePushs) {
			to_square = LS1B(singlePushs);
			if (to_square & obstructed) {
				from_square = (pos.side == WHITE) ? to_square >> 8 : to_square << 8;

				if (position_canMove(from_square, to_square)) {
					if (to_square & rank18)
						position_generatePromotionMoves(movelist, from_square, to_square, 0);
					else
						position_listAdd(movelist, from_square, to_square, NORMAL,0,0,0);
				}
			}
			singlePushs = RESET_LS1B(singlePushs);
		}

		while (doublePushs) {
			to_square = LS1B(doublePushs);
			if (to_square & obstructed) {
				from_square = (pos.side == WHITE) ? to_square >> 16 : to_square << 16;
				if (position_canMove(from_square, to_square)) {
					position_listAdd(movelist, from_square, to_square, PAWN_DOUBLE,0,0,0);
				}
			}
			doublePushs = RESET_LS1B(doublePushs);
		}

		while(obstructed) {
			to_square = LS1B(obstructed);
			blockers = pos.attacks_to[bitboard_bitScanForward(to_square)] & our_pieces;
			while(blockers) {
				from_square = LS1B(blockers);
				if (position_canMove(from_square, to_square)) {
					position_listAdd(movelist, from_square, to_square, NORMAL,0,0,0);
				}
				blockers = RESET_LS1B(blockers);
			}
			obstructed = RESET_LS1B(obstructed);
		}
	}

	/*
	###############################################################
	# 3. Move out the way
	################################################################
	*/

	U64 kingMoves    = bitboard_getKingMoves(king) & (pos.bb_empty | other_pieces);

	while(king_attackers) {
		king_attacker = LS1B(king_attackers);
		kingMoves &= ~pos.attacks_from[bitboard_bitScanForward(king_attacker)];
		king_attackers = RESET_LS1B(king_attackers);
	}

	while(kingMoves) {
		to_square = LS1B(kingMoves);

		// If the square is not attacked by enemy piece
		if (position_canMove(king, to_square)) {
			if (to_square & other_pieces) {
				// capture
				position_listAdd(movelist, king, to_square, NORMAL,1,0,0);
			}
			else {
				// empty square
				position_listAdd(movelist, king, to_square, NORMAL,0,0,0);
			}
		}
		kingMoves = RESET_LS1B(kingMoves);
	}
}


void position_generatePinned()
{
	U64 pinned = EMPTY;
	U64 pinner = EMPTY;
	U64 king = EMPTY;
	U64 QueenRooks = EMPTY;
	U64 QueenBishops = EMPTY;
	U64 blockers = EMPTY;
	U64 sq = EMPTY;
	int otherSide = 1 ^ pos.side;

	king = pos.bb_pieces[K + pos.side];
	QueenRooks = pos.bb_pieces[R + otherSide] | pos.bb_pieces[Q + otherSide];
	QueenBishops = pos.bb_pieces[B + otherSide] | pos.bb_pieces[Q + otherSide];
	blockers = pos.bb_side[pos.side];

	pinner = bitboard_xrayFileAttacks(pos.bb_occupied, blockers, king) & QueenRooks;
	pinner |= bitboard_xrayRankAttacks(pos.bb_occupied, blockers, king) & QueenRooks;
	pinner |= bitboard_xrayDiagonalAttacks(pos.bb_occupied, blockers, king) & QueenBishops;

	while (pinner) {
		sq  = LS1B(pinner);
		pinned = bitboard_getObstructed(sq, king) & blockers;
		pos.pinned |= pinned;
		// pos.pinner[bitboard_bitScanForward(pinned)] |= pinner;
		pos.pinner[bitboard_bitScanForward(pinned)] = sq;
		pinner = RESET_LS1B(pinner);
	}
}

void position_generatePromotionMoves( Move *movelist, U64 from_square, U64 to_square, int capture)
{
	Square queen  = Q + pos.side;
	Square bishop = B + pos.side;
	Square knight = N + pos.side;
	Square rook   = R + pos.side;

	if (capture) {
		position_listAdd(movelist, from_square, to_square, PROMOTION,1,queen,0);
		position_listAdd(movelist, from_square, to_square, PROMOTION,1,bishop,0);
		position_listAdd(movelist, from_square, to_square, PROMOTION,1,knight,0);
		position_listAdd(movelist, from_square, to_square, PROMOTION,1,rook,0);
	}
	else {
		position_listAdd(movelist, from_square, to_square, PROMOTION,0,queen,0);
		position_listAdd(movelist, from_square, to_square, PROMOTION,0,bishop,0);
		position_listAdd(movelist, from_square, to_square, PROMOTION,0,knight,0);
		position_listAdd(movelist, from_square, to_square, PROMOTION,0,rook,0);
	}
}

int position_canMove( U64 from_square, U64 to_square)
{
	int otherSide = 1 ^ pos.side;
	U64 king = pos.bb_pieces[K + pos.side];
	U64 other_king = pos.bb_pieces[K + otherSide];
	U64 other_pieces = pos.bb_side[otherSide];

	U64 otherQueenRooks = pos.bb_pieces[R + otherSide] | pos.bb_pieces[Q + otherSide];
	U64 otherQueenBishops = pos.bb_pieces[B + otherSide] | pos.bb_pieces[Q + otherSide];
	U64 pinner = EMPTY;
	U64 occupancy = EMPTY;

	/* Make sure that the oponent king is not on destination square */
	if (to_square & other_king) {
		return 0;
	}

	/* Need to make sure that king doesn't move into check */
	if ((from_square & king) && (pos.attacks_to[bitboard_bitScanForward(to_square)] & other_pieces)) {
		return 0;
	}

	if (from_square & king) {
		occupancy = pos.bb_occupied ^ king;
		if (bitboard_rankAttacks(occupancy, to_square) & otherQueenRooks) {
			return 0;
		}
		if (bitboard_fileAttacks(occupancy, to_square) & otherQueenRooks) {
			return 0;
		}
		if (bitboard_diagonalAttacks(occupancy, to_square) & otherQueenBishops) {
			return 0;
		}
	}

	if (from_square & pos.pinned) {
		pinner = pos.pinner[bitboard_bitScanForward(from_square)];
		/* Pinned piece can only move in the direction of the pinner attack ray */
		/* Knights cannot move if pinned */
		if (pinner & otherQueenBishops) {
			if ((bitboard_getDiagNE(pinner) & from_square) && (bitboard_getDiagNE(pinner) & to_square)) {
				return 1;
			}
			else if ((bitboard_getDiagNW(pinner) & from_square) && (bitboard_getDiagNW(pinner) & to_square)) {
				return 1;
			}
		}

		if (pinner & otherQueenRooks) {
			if ((bitboard_getRank(pinner) & from_square) && (bitboard_getRank(pinner) & to_square)) {
				return 1;
			}
			else if ((bitboard_getFile(pinner) & from_square) && (bitboard_getFile(pinner) & to_square)) {
				return 1;
			}
		}
		return 0;
	}

	return 1;
}
