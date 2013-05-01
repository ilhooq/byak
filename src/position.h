#ifndef POSITION_H
#define POSITION_H
#include "bitboard.h"
#include "pieces.h"
#include "move.h"
#include "util.h"

#define WHITE 0
#define BLACK 1

/* 16 possible states */
#define W_CASTLE_K 0x1 /* 0001 : 1 */
#define W_CASTLE_Q 0x2 /* 0010 : 2 */
#define B_CASTLE_K 0x4 /* 0100 : 4 */
#define B_CASTLE_Q 0x8 /* 1000 : 8 */


typedef struct {
	U64 bb_pieces[12]; // Pieces occupancy
	U64 bb_side[2]; // Side occupancy
	U64 bb_occupied;
	U64 bb_empty ;
	U64 attacks_from[64];
	U64 attacks_to[64];
	// Move movelist[256];
	U8 movelistcount;

	U64 pinned; // Pinned squares
	U64 pinner[64];

	int in_check;
	int checkmated;

	U64 last_double;
	int enpassant;
	

	short castling_rights;

	int side; // White : 0, black : 1
	U64 hash;
} Position;

extern Position pos;

typedef struct {
	U64 nodes;
	U64 checks;
	U64 checkmated;
	U64 captures;
	U64 EP;
	U64 castles;
	U64 promotions;
} PerftData;

void position_init();

/**
 * Fen parser
 * @param Position
 * @param char* the fen string
 * @return 0 if OK ether -1 if an error occured
 */
int position_fromFen(const char *fen);

void position_display();

void position_refresh();

void position_makeMove(Move *move);

void position_undoMove(Move *move);

INLINE void position_addAttack(U64 from_square, U64 to_square);

/**
 * Generate all legal moves
 */
U8 position_generateMoves(Move *movelist);

void position_generateAttacks();

void position_generateCheckEvasions(Move *movelist);

void position_generatePinned();

void position_listAdd(Move *movelist, U64 from_square, U64 to_square, TypeMove type, char capture, Piece promoted_piece, Piece captured_piece);

void position_generatePromotionMoves(Move *movelist, U64 from_square, U64 to_square, int capture);

INLINE int position_canMove(U64 from_square, U64 to_square);

U64 position_perft(int depth);
#endif
