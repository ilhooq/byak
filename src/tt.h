#ifndef TT_H
#define TT_H

#include "types.h"

typedef struct {
	U64 piecesquare[12][64];
	U64 side;
	U64 castling[16];
	U64 ep[64];
} Zobrist;

enum ttflags {
	TT_EXACT,
	TT_ALPHA,
	TT_BETA
};

/* 16 bytes long */
typedef struct {
	U64 hash;
	int val;
	U16 depth;
	U16 flag;
} TranspositionTable;


extern Zobrist zobrist;

void tt_init(int size);
int tt_setsize(int size);
void tt_save(U64 hash, int val, U16 depth, U16 flag);
int tt_probe(U64 hash, int alpha, int beta, U16 depth);
void tt_perft_save(U64 hash, int data, int depth);
int tt_perft_probe(U64 hash, int depth);

#endif
