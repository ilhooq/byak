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


#define OUR_SIDE pos.side
#define OTHER_SIDE (1 ^ pos.side)

#define OUR_KING pos.bb_pieces[K + OUR_SIDE]
#define OTHER_KING pos.bb_pieces[K + OTHER_SIDE]

#define OUR_PAWNS pos.bb_pieces[P + OUR_SIDE]

#define KNIGHTS (pos.bb_pieces[n] | pos.bb_pieces[N])
#define KINGS (pos.bb_pieces[k] | pos.bb_pieces[K])
#define QUEEN_ROOKS (pos.bb_pieces[Q] | pos.bb_pieces[q] | pos.bb_pieces[R] | pos.bb_pieces[r])
#define QUEEN_BISHOPS (pos.bb_pieces[Q] | pos.bb_pieces[q] | pos.bb_pieces[B] | pos.bb_pieces[b])

#define OTHER_QUEEN_ROOKS (pos.bb_pieces[R + OTHER_SIDE] | pos.bb_pieces[Q + OTHER_SIDE])
#define OTHER_QUEEN_BISHOPS (pos.bb_pieces[B + OTHER_SIDE] | pos.bb_pieces[Q + OTHER_SIDE])

#define OUR_PIECES pos.bb_side[OUR_SIDE]
#define OTHER_PIECES pos.bb_side[OTHER_SIDE]

#define W_ROCK_ATTACKED_KS ((pos.attacks_to[g1] | pos.attacks_to[f1]) & OTHER_PIECES)
/* If b1 is attacked, it's not a problem to make the Queen side rock */
#define W_ROCK_ATTACKED_QS ((pos.attacks_to[c1] | pos.attacks_to[d1]) & OTHER_PIECES)

#define W_ROCK_OCCUPIED_KS ((SQ64(g1) | SQ64(f1)) & pos.bb_occupied)
#define W_ROCK_OCCUPIED_QS ((SQ64(b1) | SQ64(c1) | SQ64(d1)) & pos.bb_occupied)

#define B_ROCK_ATTACKED_KS ((pos.attacks_to[g8] | pos.attacks_to[f8]) & OTHER_PIECES)
/* If b8 is attacked, it's not a problem to make the Queen side rock */
#define B_ROCK_ATTACKED_QS ((pos.attacks_to[c8] | pos.attacks_to[d8]) & OTHER_PIECES)
#define B_ROCK_OCCUPIED_KS ((SQ64(g8) | SQ64(f8)) & pos.bb_occupied)
#define B_ROCK_OCCUPIED_QS ((SQ64(b8) | SQ64(c8) | SQ64(d8)) & pos.bb_occupied)

Position pos;

static int movelistcount;

static void INLINE position_refresh()
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

static void listAdd( Move *movelist, U64 from_square, U64 to_square, 
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

static int canTakeEp(U64 from, U64 to)
{
	U64 rank45 = (pos.side == WHITE) ? RANK5 : RANK4;

	// Check if the pawn is not pinned on the rank when the last double move is cleared
	if ((OUR_KING & rank45) && (OTHER_QUEEN_ROOKS & rank45)) {
		U64 last_double = (pos.side == WHITE)? bitboard_soutOne(to) : bitboard_nortOne(to);
		U64 pinner = bitboard_xrayRankAttacks(pos.bb_occupied ^ last_double, OUR_PIECES, OUR_KING) & OTHER_QUEEN_ROOKS;
		if (pinner) {
			U64 pinned = bitboard_getObstructed(pinner, OUR_KING) & OUR_PIECES;
			if (pinned & from) return 0;
		}
	}

	return 1;
}

static int INLINE canMove(U64 from_square, U64 to_square)
{
	/* Make sure that the oponent king is not on destination square */
	if (to_square & OTHER_KING) {
		return 0;
	}

	/* Need to make sure that king doesn't move into check */
	if ((from_square & OUR_KING) && (pos.attacks_to[bitboard_bitScanForward(to_square)] & OTHER_PIECES)) {
		return 0;
	}

	if (from_square & OUR_KING) {
		U64 occupancy = pos.bb_occupied ^ OUR_KING;
		if (bitboard_rankAttacks(occupancy, to_square) & OTHER_QUEEN_ROOKS) return 0;
		if (bitboard_fileAttacks(occupancy, to_square) & OTHER_QUEEN_ROOKS) return 0;
		if (bitboard_diagonalAttacks(occupancy, to_square) & OTHER_QUEEN_BISHOPS) return 0;
	}

	if (from_square & pos.pinned) {
		U64 pinner = pos.pinner[bitboard_bitScanForward(from_square)];
		/* Pinned piece can only move in the direction of the pinner attack ray */
		/* Knights cannot move if pinned */
		if (pinner & OTHER_QUEEN_BISHOPS) {
			U64 diagNE = bitboard_getDiagNE(pinner);
			if ((diagNE & from_square) && (diagNE & to_square)) return 1;
			U64 diagNW = bitboard_getDiagNW(pinner);
			if ((diagNW & from_square) && (diagNW & to_square)) return 1;
		}

		if (pinner & OTHER_QUEEN_ROOKS) {
			U64 rank = bitboard_getRank(pinner);
			if ((rank & from_square) && (rank & to_square)) return 1;
			U64 file = bitboard_getFile(pinner);
			if ((file & from_square) && (file & to_square)) return 1;
		}
		return 0;
	}

	return 1;
}

static void addPromotionMoves( Move *movelist, U64 from_square, U64 to_square, int capture)
{
	Square queen  = Q + pos.side;
	Square bishop = B + pos.side;
	Square knight = N + pos.side;
	Square rook   = R + pos.side;

	listAdd(movelist, from_square, to_square, PROMOTION, capture, queen, 0);
	listAdd(movelist, from_square, to_square, PROMOTION, capture, bishop, 0);
	listAdd(movelist, from_square, to_square, PROMOTION, capture, knight, 0);
	listAdd(movelist, from_square, to_square, PROMOTION, capture, rook, 0);
}

void static INLINE genPinned()
{
	U64 pinned = EMPTY;
	U64 pinner = EMPTY;
	U64 sq = EMPTY;

	pinner = bitboard_xrayFileAttacks(pos.bb_occupied, OUR_PIECES, OUR_KING) & OTHER_QUEEN_ROOKS;
	pinner |= bitboard_xrayRankAttacks(pos.bb_occupied, OUR_PIECES, OUR_KING) & OTHER_QUEEN_ROOKS;
	pinner |= bitboard_xrayDiagonalAttacks(pos.bb_occupied, OUR_PIECES, OUR_KING) & OTHER_QUEEN_BISHOPS;

	while (pinner) {
		sq  = LS1B(pinner);
		pinned = bitboard_getObstructed(sq, OUR_KING) & OUR_PIECES;
		pos.pinned |= pinned;
		pos.pinner[bitboard_bitScanForward(pinned)] = sq;
		pinner = RESET_LS1B(pinner);
	}
}


/*
   For this function, perfs are better without static and 
   INLINE declaration probably because of linker optimizations.
*/
void genPiecesAttacks()
{
	U64 non_pawns = pos.bb_occupied & ~pos.bb_pieces[P] & ~pos.bb_pieces[p];
	U64 from_square = EMPTY;
	U64 moves = EMPTY;

	while (non_pawns) {
		from_square = LS1B(non_pawns);

		if (from_square & KINGS) {
			moves |= bitboard_getKingMoves(from_square & KINGS);
		}

		if (from_square & KNIGHTS) {
			moves |= bitboard_getKnightMoves(from_square & KNIGHTS);
		}

		if (from_square & QUEEN_ROOKS) {
			moves |= Rmagic(bitboard_bitScanForward(from_square & QUEEN_ROOKS), pos.bb_occupied);
		}

		if (from_square & QUEEN_BISHOPS) {
			moves |= Bmagic(bitboard_bitScanForward(from_square & QUEEN_BISHOPS), pos.bb_occupied);
		}

		while (moves) {
			POS_ADD_ATTACK(from_square, LS1B(moves));
			moves = RESET_LS1B(moves);
		}

		non_pawns = RESET_LS1B(non_pawns);
	}
}

static void genPawnsAttacks()
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

static void genCheckEvasions(Move *movelist)
{
	/* pos should only get called if we're in check */
	assert(pos.in_check);

	U64 king_attackers = pos.attacks_to[bitboard_bitScanForward(OUR_KING)] & OTHER_PIECES;

	U64 from_square = EMPTY;
	U64 to_square = EMPTY;

	U64 enpassant_mask = (pos.enpassant != NONE_SQUARE) ? SQ64(pos.enpassant) : EMPTY;

	// We can either
	// 1. capture the attacking piece
	// 2. block its path
	// 3. move out of the way

	if (bitboard_IPopCount(king_attackers) == 1) {
		U64 blockers = EMPTY;

		/*
		###########################################################
		1. Try to capture the attacking piece
		###########################################################
		*/
		U64 our_attacking_pieces = pos.attacks_to[bitboard_bitScanForward(king_attackers)] & OUR_PIECES;

		if (our_attacking_pieces & OUR_KING) {
			// The king capture is processed in the Move out the way part
			our_attacking_pieces ^= OUR_KING;
		}

		// Check for enpassant capture
		if (enpassant_mask) {
			// We know the king attacker is a pawn
			if (bitboard_westOne(king_attackers) & OUR_PAWNS) {
				from_square = bitboard_westOne(king_attackers) & OUR_PAWNS;
				if (canMove(from_square, enpassant_mask)) {
					listAdd(movelist, from_square, enpassant_mask, ENPASSANT,1,0,0);
					our_attacking_pieces ^= from_square;
				}
			}
			if (bitboard_eastOne(king_attackers) & OUR_PAWNS) {
				from_square = bitboard_eastOne(king_attackers) & OUR_PAWNS;
				if (canMove(from_square, enpassant_mask)) {
					listAdd(movelist, from_square, enpassant_mask, ENPASSANT,1,0,0);
					our_attacking_pieces ^= from_square;
				}
			}
		}

		while (our_attacking_pieces) {
			from_square = LS1B(our_attacking_pieces);
			to_square = pos.attacks_from[bitboard_bitScanForward(from_square)] & king_attackers;

			if ((to_square & king_attackers) && canMove(from_square, to_square)) {
				// Capture
				if ((from_square & OUR_PAWNS) && (RANK18 & to_square)) {
					// We have a capture and promotion
					addPromotionMoves(movelist, from_square, to_square, 1);
				} else {
					listAdd(movelist, from_square, to_square, NORMAL,1,0,0);
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
			singlePushs = bitboard_nortOne(pos.bb_pieces[P]) & pos.bb_empty;
			doublePushs = bitboard_nortOne(singlePushs) & pos.bb_empty & RANK4;
		}
		else {
			singlePushs = bitboard_soutOne(pos.bb_pieces[p]) & pos.bb_empty;
			doublePushs = bitboard_soutOne(singlePushs) & pos.bb_empty & RANK5;
		}

		obstructed = bitboard_getObstructed(king_attackers, OUR_KING);

		while (singlePushs) {
			to_square = LS1B(singlePushs);
			if (to_square & obstructed) {
				from_square = (pos.side == WHITE) ? to_square >> 8 : to_square << 8;

				if (canMove(from_square, to_square)) {
					if (to_square & RANK18)
						addPromotionMoves(movelist, from_square, to_square, 0);
					else
						listAdd(movelist, from_square, to_square, NORMAL,0,0,0);
				}
			}
			singlePushs = RESET_LS1B(singlePushs);
		}

		while (doublePushs) {
			to_square = LS1B(doublePushs);
			if (to_square & obstructed) {
				from_square = (pos.side == WHITE) ? to_square >> 16 : to_square << 16;
				if (canMove(from_square, to_square)) {
					listAdd(movelist, from_square, to_square, PAWN_DOUBLE,0,0,0);
				}
			}
			doublePushs = RESET_LS1B(doublePushs);
		}

		while(obstructed) {
			to_square = LS1B(obstructed);
			blockers = pos.attacks_to[bitboard_bitScanForward(to_square)] & (OUR_PIECES & ~OUR_KING & ~OUR_PAWNS);
			while(blockers) {
				from_square = LS1B(blockers);
				if (canMove(from_square, to_square)) {
					listAdd(movelist, from_square, to_square, NORMAL,0,0,0);
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

	U64 kingMoves = bitboard_getKingMoves(OUR_KING) & (pos.bb_empty | OTHER_PIECES);

	while(king_attackers) {
		kingMoves &= ~pos.attacks_from[bitboard_bitScanForward(LS1B(king_attackers))];
		king_attackers = RESET_LS1B(king_attackers);
	}

	while(kingMoves) {
		to_square = LS1B(kingMoves);

		// If the square is not attacked by enemy piece
		if (canMove(OUR_KING, to_square)) {
			if (to_square & OTHER_PIECES) {
				// capture
				listAdd(movelist, OUR_KING, to_square, NORMAL,1,0,0);
			}
			else {
				// empty square
				listAdd(movelist, OUR_KING, to_square, NORMAL,0,0,0);
			}
		}
		kingMoves = RESET_LS1B(kingMoves);
	}
}

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
		if (pos.bb_pieces[piece[i]] & bb_from) pieceFrom = piece[i];
		if (pos.bb_pieces[piece[i]] & bb_to) move->captured_piece = piece[i];
	}

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
		switch ((int) move->to) {
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
		if ((bitboard_westOne(bb_to) | bitboard_eastOne(bb_to)) & pos.bb_pieces[P + (1 ^ pos.side)]) {
			// Activate new enPassant
			pos.enpassant = (move->from + move->to) / 2;
			pos.hash ^= zobrist.ep[pos.enpassant];
		}
	}

	/* switch side to move */
	pos.side = 1 ^ pos.side;
	pos.hash ^= zobrist.side;

	position_refresh();
}

void position_undoMove(Move *move)
{
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
		switch ((int) move->to) {
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

int position_generateMoves(Move *movelist)
{
	movelistcount = 0;

	genPinned();
	genPiecesAttacks();
	genPawnsAttacks();

	/* Are we in check?! */
	if (pos.attacks_to[bitboard_bitScanForward(OUR_KING)] & OTHER_PIECES) {
		/* we are in check, find check evasions to get out */
		pos.in_check = 1;
		genCheckEvasions(movelist);

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

	U64 from_square = EMPTY;
	U64 to_square = EMPTY;
	U64 moves = EMPTY;
	U64 pieces = OUR_PIECES;

	U64 enpassant_mask = (pos.enpassant != NONE_SQUARE) ? SQ64(pos.enpassant) : EMPTY;

	while (pieces) {
		from_square = LS1B(pieces);
		moves = pos.attacks_from[bitboard_bitScanForward(from_square)] & (OTHER_PIECES | pos.bb_empty);
		while (moves) {
			to_square = LS1B(moves);

			if (!canMove(from_square, to_square)) {
				moves = RESET_LS1B(moves);
				continue;
			}

			/* En passant capture */
			if (enpassant_mask && (to_square == enpassant_mask) && (from_square & OUR_PAWNS) && canTakeEp(from_square, enpassant_mask)) {
				listAdd(movelist, from_square, to_square, ENPASSANT,1,0,0);
			}

			/* Normal capture */
			else if (to_square & OTHER_PIECES) {

				if ((from_square & OUR_PAWNS) && (RANK18 & to_square)) {
					/* We have a capture and promotion */
					addPromotionMoves(movelist, from_square, to_square, 1);
				}

				else {
					listAdd(movelist, from_square, to_square, NORMAL,1,0,0);
				}
			}

			/* Empty square */
			else if (from_square & ~OUR_PAWNS) {
				listAdd(movelist, from_square, to_square, NORMAL,0,0,0);
			}

			moves = RESET_LS1B(moves);
		}
		pieces = RESET_LS1B(pieces);
	}

	/* Generate castling moves */

	if (pos.side == WHITE && (pos.castling_rights & (W_CASTLE_K|W_CASTLE_Q))) {

		if ((pos.castling_rights & W_CASTLE_K) && pos.bb_pieces[R] & SQ64(h1) && !W_ROCK_ATTACKED_KS && !W_ROCK_OCCUPIED_KS) {
			listAdd(movelist, OUR_KING, SQ64(g1), CASTLE,0,0,0);
		}

		if ((pos.castling_rights & W_CASTLE_Q) && pos.bb_pieces[R] & SQ64(a1) && !W_ROCK_ATTACKED_QS && !W_ROCK_OCCUPIED_QS) {
			listAdd(movelist, OUR_KING, SQ64(c1), CASTLE,0,0,0);
		}
	}
	else if (pos.side == BLACK && (pos.castling_rights & (B_CASTLE_K|B_CASTLE_Q))) {

		if ((pos.castling_rights & B_CASTLE_K) && pos.bb_pieces[r] & SQ64(h8) && !B_ROCK_ATTACKED_KS && !B_ROCK_OCCUPIED_KS) {
			listAdd(movelist, OUR_KING, SQ64(g8), CASTLE,0,0,0);
		}

		if ((pos.castling_rights & B_CASTLE_Q) && pos.bb_pieces[r] & SQ64(a8) && !B_ROCK_ATTACKED_QS && !B_ROCK_OCCUPIED_QS) {
			listAdd(movelist, OUR_KING, SQ64(c8), CASTLE,0,0,0);
		}
	}

	/*
	* produce single pawn moves...these are not handled in generate_attacks
	* since pawns can only attack diagonally (or enpassant)
	*/

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
		from_square = (pos.side == WHITE) ? to_square >> 8 : to_square << 8;

		if ((from_square & pos.pinned) && canMove(from_square, to_square)) {
			pos.pinned ^= from_square;
		}

		if (from_square & ~pos.pinned) {
			/* Check promotion */
			if (to_square & RANK18)
				addPromotionMoves(movelist, from_square, to_square, 0);
			else
				listAdd(movelist, from_square, to_square, NORMAL,0,0,0);
		}
		singlePushs = RESET_LS1B(singlePushs);
	}

	while (doublePushs) {
		to_square = LS1B(doublePushs);
		from_square = (pos.side == WHITE) ? to_square >> 16 : to_square << 16;
		if (from_square & ~pos.pinned) {
			listAdd(movelist, from_square, to_square, PAWN_DOUBLE,0,0,0);
		}
		doublePushs = RESET_LS1B(doublePushs);
	}

	return movelistcount;
}

