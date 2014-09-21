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

#define W_ROCK_ATTACKED_KS (squareAttacked(g1) | squareAttacked(f1))
/* If b1 is attacked, it's not a problem to make the Queen side rock */
#define W_ROCK_ATTACKED_QS (squareAttacked(c1) | squareAttacked(d1))
#define W_ROCK_OCCUPIED_KS ((SQ64(g1) | SQ64(f1)) & pos.bb_occupied)
#define W_ROCK_OCCUPIED_QS ((SQ64(b1) | SQ64(c1) | SQ64(d1)) & pos.bb_occupied)

#define B_ROCK_ATTACKED_KS (squareAttacked(g8) | squareAttacked(f8))
/* If b8 is attacked, it's not a problem to make the Queen side rock */
#define B_ROCK_ATTACKED_QS (squareAttacked(c8) | squareAttacked(d8))
#define B_ROCK_OCCUPIED_KS ((SQ64(g8) | SQ64(f8)) & pos.bb_occupied)
#define B_ROCK_OCCUPIED_QS ((SQ64(b8) | SQ64(c8) | SQ64(d8)) & pos.bb_occupied)

Position pos;

static int movelistcount;

static void INLINE position_refresh()
{
	memset(pos.pinner, 0, sizeof(pos.pinner));

	pos.kingAttacks[WHITE] = EMPTY;
	pos.knightsAttacks[WHITE] = EMPTY;
	pos.queenBishopsAttacks[WHITE] = EMPTY;
	pos.queenRooksAttacks[WHITE] = EMPTY;

	pos.kingAttacks[BLACK] = EMPTY;
	pos.knightsAttacks[BLACK] = EMPTY;
	pos.queenBishopsAttacks[BLACK] = EMPTY;
	pos.queenRooksAttacks[BLACK] = EMPTY;

	pos.pawnAttacks[WHITE] = EMPTY;
	pos.pawnAttacks[BLACK] = EMPTY;

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

static void INLINE listAdd(Move *movelist, U8 from_square, U8 to_square, U16 type)
{
	Move * move = &movelist[movelistcount];
	move->from = from_square;
	move->to = to_square;
	move->flags = type;
	movelistcount++;
}


static U64 position_getAttackersTo(Square sq) {
	U64 bb_sq = SQ64(sq);
	U64 attackers = EMPTY;

	if (pos.pawnAttacks[WHITE] & bb_sq) {
		attackers |= ((bitboard_soWeOne(bb_sq) | bitboard_soEaOne(bb_sq)) & pos.bb_pieces[P]);
	}

	if (pos.pawnAttacks[BLACK] & bb_sq) {
		attackers |= ((bitboard_noWeOne(bb_sq) | bitboard_noEaOne(bb_sq)) & pos.bb_pieces[p]);
	}

	if ((pos.knightsAttacks[WHITE] | pos.knightsAttacks[BLACK]) & bb_sq) {
		attackers |= bitboard_getKnightMoves(bb_sq) & (pos.bb_pieces[N] | pos.bb_pieces[n]);
	}

	if ((pos.kingAttacks[WHITE] | pos.kingAttacks[BLACK]) & bb_sq) {
		attackers |= bitboard_getKingMoves(bb_sq) & (pos.bb_pieces[K] | pos.bb_pieces[k]);
	}

	if ((pos.queenRooksAttacks[WHITE] | pos.queenRooksAttacks[BLACK]) & bb_sq) {
		attackers |= Rmagic(sq, pos.bb_occupied) & QUEEN_ROOKS;
	}

	if ((pos.queenBishopsAttacks[WHITE] | pos.queenBishopsAttacks[BLACK]) & bb_sq) {
		attackers |= Bmagic(sq, pos.bb_occupied) & QUEEN_BISHOPS;
	}

	return attackers;
}

static U64 position_attacksFrom(Square sq)
{
	U64 bb_sq = SQ64(sq);
	U64 attacks = EMPTY;

	if (bb_sq & pos.bb_pieces[P]) {
		return (bitboard_noWeOne(bb_sq) | bitboard_noEaOne(bb_sq));
	}

	if (bb_sq & pos.bb_pieces[p]) {
		return (bitboard_soWeOne(bb_sq) | bitboard_soEaOne(bb_sq));
	}

	if (bb_sq & (pos.bb_pieces[K] | pos.bb_pieces[k])) {
		return bitboard_getKingMoves(bb_sq);
	}

	if (bb_sq & (pos.bb_pieces[N] | pos.bb_pieces[n])) {
		return bitboard_getKnightMoves(bb_sq);
	}

	if (bb_sq & QUEEN_ROOKS) {
		attacks |= Rmagic(sq, pos.bb_occupied);
	}

	if (bb_sq & QUEEN_BISHOPS) {
		attacks |= Bmagic(sq, pos.bb_occupied);
	}

	return attacks;
}

static int squareAttacked(Square sq)
{
	U64 bb_sq = SQ64(sq);

	if (pos.pawnAttacks[OTHER_SIDE] & bb_sq) return 1;

	if (pos.knightsAttacks[OTHER_SIDE] & bb_sq) return 1;

	if (pos.kingAttacks[OTHER_SIDE] & bb_sq) return 1;

	if (pos.queenRooksAttacks[OTHER_SIDE] & bb_sq) return 1;

	if (pos.queenBishopsAttacks[OTHER_SIDE] & bb_sq) return 1;

	return 0;
}

int position_inCheck()
{
	if (pos.pawnAttacks[OTHER_SIDE] & OUR_KING) return 1;

	if (pos.knightsAttacks[OTHER_SIDE] & OUR_KING) return 1;

	if (pos.queenRooksAttacks[OTHER_SIDE] & OUR_KING) return 1;

	if (pos.queenBishopsAttacks[OTHER_SIDE] & OUR_KING) return 1;

	return 0;
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

	/* Make sure that king doesn't move into check */
	if ((from_square & OUR_KING) && squareAttacked(bitboard_bitScanForward(to_square))) {
		return 0;
	}

	/* Make sure that king doesn't move into the opposite attacking ray */
	if (pos.in_check && (from_square & OUR_KING)) {
		U64 occupancy = pos.bb_occupied ^ OUR_KING;
		if (Rmagic(bitboard_bitScanForward(to_square), occupancy) & OTHER_QUEEN_ROOKS) return 0;
		if (Bmagic(bitboard_bitScanForward(to_square), occupancy) & OTHER_QUEEN_BISHOPS) return 0;
	}

	return 1;
}

static void addPromotionMoves( Move *movelist, Square from_square, Square to_square, unsigned short type)
{
	/*
	Square queen  = Q + pos.side;
	Square bishop = B + pos.side;
	Square knight = N + pos.side;
	Square rook   = R + pos.side;
	*/

	listAdd(movelist, from_square, to_square, (type|MOVE_PROMOTION|MOVE_PROMOTION_QUEEN));
	listAdd(movelist, from_square, to_square, (type|MOVE_PROMOTION|MOVE_PROMOTION_BISHOP));
	listAdd(movelist, from_square, to_square, (type|MOVE_PROMOTION|MOVE_PROMOTION_KNIGHT));
	listAdd(movelist, from_square, to_square, (type|MOVE_PROMOTION|MOVE_PROMOTION_ROOK));
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
void genAttacks()
{
	U64 pieces = EMPTY;

	/* Generate king attacks */
	pos.kingAttacks[WHITE] = bitboard_getKingMoves(pos.bb_pieces[K]);
	pos.kingAttacks[BLACK] = bitboard_getKingMoves(pos.bb_pieces[k]);

	/* Generate knight attacks */

	if (pos.bb_pieces[N]) {
		pieces = pos.bb_pieces[N];
		do {
			pos.knightsAttacks[WHITE] |= bitboard_getKnightMoves(LS1B(pieces));
		}
		while ((pieces = RESET_LS1B(pieces)));
	}

	if (pos.bb_pieces[n]) {
		pieces = pos.bb_pieces[n];
		do {
			pos.knightsAttacks[BLACK] |= bitboard_getKnightMoves(LS1B(pieces));
		}
		while ((pieces = RESET_LS1B(pieces)));
	}

	/* Generate queen attacks */

	if (pos.bb_pieces[Q]) {
		pos.queenRooksAttacks[WHITE] |= Rmagic(bitboard_bitScanForward(pos.bb_pieces[Q]), pos.bb_occupied);
		pos.queenBishopsAttacks[WHITE] |= Bmagic(bitboard_bitScanForward(pos.bb_pieces[Q]), pos.bb_occupied);
	}

	if (pos.bb_pieces[q]) {
		pos.queenRooksAttacks[BLACK] |= Rmagic(bitboard_bitScanForward(pos.bb_pieces[q]), pos.bb_occupied);
		pos.queenBishopsAttacks[BLACK] |= Bmagic(bitboard_bitScanForward(pos.bb_pieces[q]), pos.bb_occupied);
	}

	/* Generate rook attacks */

	if (pos.bb_pieces[R]) {
		pieces = pos.bb_pieces[R];
		do {
			pos.queenRooksAttacks[WHITE] |= Rmagic(bitboard_bitScanForward(LS1B(pieces)), pos.bb_occupied);
		}
		while ((pieces = RESET_LS1B(pieces)));
	}

	if (pos.bb_pieces[r]) {
		pieces = pos.bb_pieces[r];
		do {
			pos.queenRooksAttacks[BLACK] |= Rmagic(bitboard_bitScanForward(LS1B(pieces)), pos.bb_occupied);
		}
		while ((pieces = RESET_LS1B(pieces)));
	}

	/* Generate bishop attacks */

	if (pos.bb_pieces[B]) {
		pieces = pos.bb_pieces[B];
		do {
			pos.queenBishopsAttacks[WHITE] |= Bmagic(bitboard_bitScanForward(LS1B(pieces)), pos.bb_occupied);
		}
		while ((pieces = RESET_LS1B(pieces)));
	}

	if (pos.bb_pieces[b]) {
		pieces = pos.bb_pieces[b];
		do {
			pos.queenBishopsAttacks[BLACK] |= Bmagic(bitboard_bitScanForward(LS1B(pieces)), pos.bb_occupied);
		}
		while ((pieces = RESET_LS1B(pieces)));
	}

	/* Generate pawn attacks */

	pos.pawnAttacks[WHITE] = (bitboard_noWeOne(pos.bb_pieces[P]) | bitboard_noEaOne(pos.bb_pieces[P]));
	pos.pawnAttacks[BLACK] = (bitboard_soWeOne(pos.bb_pieces[p]) | bitboard_soEaOne(pos.bb_pieces[p]));
}

static void genCheckEvasions(Move *movelist)
{
	/* pos should only get called if we're in check */
	assert(pos.in_check);
	U64 bb_from = EMPTY, bb_to = EMPTY;
	int from = 0, to = 0;
	int king_sq = bitboard_bitScanForward(OUR_KING);

	U64 king_attackers = position_getAttackersTo(king_sq) & OTHER_PIECES;

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
		U64 our_attacking_pieces = position_getAttackersTo(bitboard_bitScanForward(king_attackers)) & OUR_PIECES;

		if (our_attacking_pieces & OUR_KING) {
			// The king capture is processed in the Move out the way part
			our_attacking_pieces ^= OUR_KING;
		}

		// Check for enpassant capture
		if (pos.enpassant != NONE_SQUARE) {
			// We know the king attacker is a pawn
			bb_to = SQ64(pos.enpassant);
			if ((bb_from = (bitboard_westOne(king_attackers) & OUR_PAWNS)) && canMove(bb_from, bb_to)) {
				listAdd(movelist, bitboard_bitScanForward(bb_from), pos.enpassant, (MOVE_CAPTURE|MOVE_ENPASSANT));
				our_attacking_pieces ^= bb_from;
			}
			if ((bb_from = (bitboard_eastOne(king_attackers) & OUR_PAWNS)) && canMove(bb_from, bb_to)) {
				listAdd(movelist, bitboard_bitScanForward(bb_from), pos.enpassant, (MOVE_CAPTURE|MOVE_ENPASSANT));
				our_attacking_pieces ^= bb_from;
			}
		}

		while (our_attacking_pieces) {
			bb_from = LS1B(our_attacking_pieces);
			from = bitboard_bitScanForward(bb_from);
			bb_to = position_attacksFrom(from) & king_attackers;

			if ((bb_to & king_attackers) && canMove(bb_from, bb_to)) {
				to = bitboard_bitScanForward(bb_to);
				// Capture
				if ((bb_from & OUR_PAWNS) && (RANK18 & bb_to)) {
					// We have a capture and promotion
					addPromotionMoves(movelist, from, to, MOVE_CAPTURE);
				} else {
					listAdd(movelist, from, to, MOVE_CAPTURE);
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
			bb_to = LS1B(singlePushs);
			if (bb_to & obstructed) {
				bb_from = (pos.side == WHITE) ? bb_to >> 8 : bb_to << 8;

				if (canMove(bb_from, bb_to)) {
					from = bitboard_bitScanForward(bb_from);
					to = bitboard_bitScanForward(bb_to);
					if (bb_to & RANK18)
						addPromotionMoves(movelist, from, to, 0);
					else
						listAdd(movelist, from, to, MOVE_NORMAL);
				}
			}
			singlePushs = RESET_LS1B(singlePushs);
		}

		while (doublePushs) {
			bb_to = LS1B(doublePushs);
			if (bb_to & obstructed) {
				bb_from = (pos.side == WHITE) ? bb_to >> 16 : bb_to << 16;
				if (canMove(bb_from, bb_to)) {
					listAdd(movelist, bitboard_bitScanForward(bb_from), bitboard_bitScanForward(bb_to), MOVE_PAWN_DOUBLE);
				}
			}
			doublePushs = RESET_LS1B(doublePushs);
		}

		while(obstructed) {
			bb_to = LS1B(obstructed);
			to = bitboard_bitScanForward(bb_to);
			blockers = position_getAttackersTo(to) & (OUR_PIECES & ~OUR_KING & ~OUR_PAWNS);
			while(blockers) {
				bb_from = LS1B(blockers);
				if (canMove(bb_from, bb_to)) {
					from = bitboard_bitScanForward(bb_from);
					listAdd(movelist, from, to, MOVE_NORMAL);
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
	
	from = bitboard_bitScanForward(OUR_KING);

	while(kingMoves) {
		bb_to = LS1B(kingMoves);

		// If the square is not attacked by enemy piece
		if (canMove(OUR_KING, bb_to)) {
			if (bb_to & OTHER_PIECES) {
				// capture
				listAdd(movelist, from, bitboard_bitScanForward(bb_to), MOVE_CAPTURE);
			}
			else {
				// empty square
				listAdd(movelist, from, bitboard_bitScanForward(bb_to), MOVE_NORMAL);
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

	pos.enpassant = NONE_SQUARE;
	pos.castling_rights = 0;
	pos.side = WHITE; // White To Move

	// memset(pos.attacks_from, 0, sizeof(pos.attacks_from));
	// memset(pos.attacks_to, 0, sizeof(pos.attacks_to));
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
	U64 last_double = EMPTY;

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
					last_double = bitboard_soutOne(enPassantTarget);
				}
				if (enPassantTarget & RANK3) {
					last_double = bitboard_nortOne(enPassantTarget);
				}
				if (last_double) {
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
	move->captured_piece = NONE_PIECE;
	move->ep = NONE_SQUARE;
	move->castling_rights = pos.castling_rights;

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

	if (move->flags & MOVE_CAPTURE) {
		if (move->flags & MOVE_ENPASSANT) {
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

	if (move->flags & MOVE_PROMOTION) {
		POS_DEL_PIECE(P + pos.side, move->to);

		if (move->flags & MOVE_PROMOTION_QUEEN) {
			POS_ADD_PIECE(Q + pos.side , move->to);
		}
		else if (move->flags & MOVE_PROMOTION_BISHOP) {
			POS_ADD_PIECE(B + pos.side , move->to);
		}
		else if (move->flags & MOVE_PROMOTION_KNIGHT) {
			POS_ADD_PIECE(N + pos.side , move->to);
		}
		else if (move->flags & MOVE_PROMOTION_ROOK) {
			POS_ADD_PIECE(R + pos.side , move->to);
		}
	}
	else if (move->flags & MOVE_CASTLE) {
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
	else if (move->flags & MOVE_PAWN_DOUBLE) {

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

	if (move->flags & MOVE_CAPTURE) {
		if (move->flags & MOVE_ENPASSANT) {
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

	if (move->flags & MOVE_PROMOTION) {

		if (move->flags & MOVE_PROMOTION_QUEEN) {
			POS_DEL_PIECE(Q + pos.side , move->from);
		}
		else if (move->flags & MOVE_PROMOTION_BISHOP) {
			POS_DEL_PIECE(B + pos.side , move->from);
		}
		else if (move->flags & MOVE_PROMOTION_KNIGHT) {
			POS_DEL_PIECE(N + pos.side , move->from);
		}
		else if (move->flags & MOVE_PROMOTION_ROOK) {
			POS_DEL_PIECE(R + pos.side , move->from);
		}

		POS_ADD_PIECE(P + pos.side, move->from);
	}
	else if (move->flags & MOVE_CASTLE) {
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

	pos.hash ^= zobrist.castling[pos.castling_rights];
	pos.hash ^= zobrist.castling[move->castling_rights];
	pos.castling_rights = move->castling_rights;

	position_refresh();
}

int position_generateMoves(Move *movelist)
{
	movelistcount = 0;

	genPinned();
	genAttacks();

	/* Are we in check?! */
	if (position_inCheck()) {
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

	U64 bb_from = EMPTY;
	U64 bb_to = EMPTY;
	int from = 0;
	U64 moves = EMPTY;
	U64 pieces = OUR_PIECES;
	U64 singlePushs = EMPTY;
	U64 doublePushs = EMPTY;

	U64 bb_enpassant = (pos.enpassant != NONE_SQUARE) ? SQ64(pos.enpassant) : EMPTY;

	while (pieces) {
		bb_from = LS1B(pieces);
		from = bitboard_bitScanForward(bb_from);
		moves = position_attacksFrom(from) & (OTHER_PIECES | pos.bb_empty);
		while (moves) {
			bb_to = LS1B(moves);
			if (!canMove(bb_from, bb_to)) goto reset_move;

			/* En passant capture */
			if (bb_enpassant && (bb_to == bb_enpassant) && (bb_from & OUR_PAWNS) && canTakeEp(bb_from, bb_enpassant)) {
				listAdd(movelist, from, bitboard_bitScanForward(bb_to), (MOVE_CAPTURE|MOVE_ENPASSANT));
				goto reset_move;
			}

			/* Normal capture */
			if (bb_to & OTHER_PIECES) {

				if ((bb_from & OUR_PAWNS) && (RANK18 & bb_to)) {
					/* We have a capture and promotion */
					addPromotionMoves(movelist, from, bitboard_bitScanForward(bb_to), MOVE_CAPTURE);
				}
				else {
					listAdd(movelist, from, bitboard_bitScanForward(bb_to), MOVE_CAPTURE);
				}
			}

			/* Empty square */
			else if (bb_from & ~OUR_PAWNS) {
				listAdd(movelist, from, bitboard_bitScanForward(bb_to), MOVE_NORMAL);
			}

			reset_move:
			moves = RESET_LS1B(moves);
		}
		pieces = RESET_LS1B(pieces);
	}

	/* Generate castling moves */

	if (pos.side == WHITE && (pos.castling_rights & (W_CASTLE_K|W_CASTLE_Q))) {

		from = bitboard_bitScanForward(OUR_KING);
		if ((pos.castling_rights & W_CASTLE_K) && !W_ROCK_ATTACKED_KS && !W_ROCK_OCCUPIED_KS) {
			listAdd(movelist, from, g1, (MOVE_CASTLE|MOVE_CASTLE_KS));
		}

		if ((pos.castling_rights & W_CASTLE_Q) && !W_ROCK_ATTACKED_QS && !W_ROCK_OCCUPIED_QS) {
			listAdd(movelist, from, c1, (MOVE_CASTLE|MOVE_CASTLE_QS));
		}
	}
	else if (pos.side == BLACK && (pos.castling_rights & (B_CASTLE_K|B_CASTLE_Q))) {

		from = bitboard_bitScanForward(OUR_KING);
		if ((pos.castling_rights & B_CASTLE_K) && !B_ROCK_ATTACKED_KS && !B_ROCK_OCCUPIED_KS) {
			listAdd(movelist, from, g8, (MOVE_CASTLE|MOVE_CASTLE_KS));
		}

		if ((pos.castling_rights & B_CASTLE_Q) && !B_ROCK_ATTACKED_QS && !B_ROCK_OCCUPIED_QS) {
			listAdd(movelist, from, c8, (MOVE_CASTLE|MOVE_CASTLE_QS));
		}
	}

	/*
	* produce single pawn moves...these are not handled in generate_attacks
	* since pawns can only attack diagonally (or enpassant)
	*/

	if (pos.side == WHITE) {
		singlePushs = bitboard_nortOne(pos.bb_pieces[P]) & pos.bb_empty;
		doublePushs = bitboard_nortOne(singlePushs) & pos.bb_empty & RANK4;
	}
	else {
		singlePushs = bitboard_soutOne(pos.bb_pieces[p]) & pos.bb_empty;
		doublePushs = bitboard_soutOne(singlePushs) & pos.bb_empty & RANK5;
	}

	while (singlePushs) {
		bb_to = LS1B(singlePushs);
		bb_from = (pos.side == WHITE) ? bb_to >> 8 : bb_to << 8;

		if ((bb_from & pos.pinned) && canMove(bb_from, bb_to)) {
			pos.pinned ^= bb_from;
		}

		if (bb_from & ~pos.pinned) {
			/* Check promotion */
			if (bb_to & RANK18)
				addPromotionMoves(movelist, bitboard_bitScanForward(bb_from), bitboard_bitScanForward(bb_to), MOVE_NORMAL);
			else
				listAdd(movelist, bitboard_bitScanForward(bb_from), bitboard_bitScanForward(bb_to), MOVE_NORMAL);
		}
		singlePushs = RESET_LS1B(singlePushs);
	}

	while (doublePushs) {
		bb_to = LS1B(doublePushs);
		bb_from = (pos.side == WHITE) ? bb_to >> 16 : bb_to << 16;
		if (bb_from & ~pos.pinned) {
			listAdd(movelist, bitboard_bitScanForward(bb_from), bitboard_bitScanForward(bb_to), MOVE_PAWN_DOUBLE);
		}
		doublePushs = RESET_LS1B(doublePushs);
	}

	return movelistcount;
}

