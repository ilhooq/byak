#ifndef MOVE_H
#define MOVE_H

#include "bitboard.h"
#include "pieces.h"

typedef enum {
	NORMAL, PAWN_DOUBLE, PROMOTION, ENPASSANT, CASTLE
} TypeMove;

typedef struct {
	Square from; // Square from
	Square to;   // Square to
	TypeMove type;
	char capture; // 1: capture, 0: no capture
	Piece promoted_piece; // Q,R,B,N,q,r,b,n
	Piece captured_piece; // This will be determined when making move
	Square ep;
	short castling_rights;
	// for search
	int score;

} Move;

Move *move_create(U64 from_square, U64 to_square, TypeMove type, 
			char capture, Piece promoted_piece, Piece captured_piece);

void move_display(Move *move);
void move_displayAlg(Move *move);
void move_displayUCI(Move *move);
#endif
