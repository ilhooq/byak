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
#include "test.h"
#include "position.h"
#include "util.h"
#include "move.h"
#include "prng.h"
#include "tt.h"
#include "time.h"
#include "search.h"
#include "eval.h"

void test_suite()
{
	test_bin2alg();
	test_alg2bin();
	test_FileRankAccess();
	test_kingMoves();
	test_knightMoves();
	test_DiagNW();
	test_DiagNE();
	test_magicMoves();
	testInBetweenSquares();
	test_fen();
}

void test_timeSearch(const char *fen, int mstime)
{
	position_init();
	position_fromFen(fen);
	SearchInfos infos = {0};
	infos.time[WHITE] = mstime;
	infos.time[BLACK] = mstime;
	infos.movetime = infos.time[pos.side] / 40;
	search_start(&infos);
}

void test_depthSearch(const char *fen, int depth)
{
	position_init();
	position_fromFen(fen);
	SearchInfos infos = {0};
	infos.time[WHITE] = 0;
	infos.time[BLACK] = 0;
	infos.depth = depth;
	search_start(&infos);
}

void test_divide(const char *fen, int depth)
{
	U64 nodes = 0;
	U64 count = 0;
	U8 i, listlen;

	position_init();
	position_fromFen(fen);
	position_display();

	if (depth > 1) depth--;
	else depth = 0;

	Move movelist[256];
	listlen= position_generateMoves(movelist);

	for (i=0; i < listlen; i++){

		position_makeMove(&movelist[i]);
		count = search_perft(depth);
		move_displayAlg(&movelist[i]);
		printf(" : %llu\n", ULL(count));
		position_undoMove(&movelist[i]);
		nodes += count;
	}
	printf("\n\nNodes : %llu", ULL(nodes));
}

void test_perft(const char *fen, int depth)
{
	int start, timeused;
	float nps;
	position_init();
	position_fromFen(fen);
	start = GET_TIME();
	U64 nodes = search_perft(depth);
	timeused = GET_TIME() - start;
	nps = (float) nodes / ((float) timeused /1000);
	printf("depth:%i;time:%i;nodes:%llu;nps:%.0f\n", depth, timeused, ULL(nodes), nps);
}

void test_eval(const char *fen)
{
	position_init();
	position_fromFen(fen);
	eval_init();
	printf("Score: %i \n", eval_position());
}

/* ************** Perft suite functions below ************** */
void test_fen()
{
	position_init();
	assert(position_fromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") == 0);
}

void test_magicMoves()
{
	printf("Test magicmoves\n");
	assert(Bmagic(d4, C64(0x8000020000002001)) == C64(0x8040221400142201));
}

void testInBetweenSquares() 
{
	printf("Test in between squares\n");
	assert(bitboard_getObstructed(C64(1) << d1, C64(1) << d4) == C64(0x0000000000080800));
	assert(bitboard_getObstructed(C64(1) << a1, C64(1) << h8) == C64(0x0040201008040200));
	assert(bitboard_getObstructed(C64(1) << a8, C64(1) << h8) == C64(0x7E00000000000000));
	assert(bitboard_getObstructed(C64(1) << a1, C64(1) << h1) == C64(0x000000000000007E));
}

void test_kingMoves()
{
	printf("Test King Moves\n");
	assert(bitboard_getKingMoves(C64(1) << a8) == C64(0x0203000000000000));
	assert(bitboard_getKingMoves(C64(1) << d8) == C64(0x141C000000000000));
	assert(bitboard_getKingMoves(C64(1) << h8) == C64(0x40C0000000000000));
	assert(bitboard_getKingMoves(C64(1) << a1) == C64(0x0000000000000302));
	assert(bitboard_getKingMoves(C64(1) << d1) == C64(0x0000000000001C14));
	assert(bitboard_getKingMoves(C64(1) << h1) == C64(0x000000000000C040));
}

void test_knightMoves()
{
	printf("Test Knight Moves\n");
	assert(bitboard_getKnightMoves(C64(1) << a8) == C64(0x0004020000000000));
	assert(bitboard_getKnightMoves(C64(1) << d8) == C64(0x0022140000000000));
	assert(bitboard_getKnightMoves(C64(1) << h8) == C64(0x0020400000000000));
	assert(bitboard_getKnightMoves(C64(1) << a1) == C64(0x0000000000020400));
	assert(bitboard_getKnightMoves(C64(1) << d1) == C64(0x0000000000142200));
	assert(bitboard_getKnightMoves(C64(1) << h1) == C64(0x0000000000402000));
}

void test_FileRankAccess()
{
	printf("Test file and rank access\n");
	assert(bitboard_getRank(C64(1) << a8) == RANK8);
	assert(bitboard_getRank(C64(1) << h8) == RANK8);
	assert(bitboard_getRank(C64(1) << e4) == RANK4);
	assert(bitboard_getRank(C64(1) << d5) == RANK5);
	assert(bitboard_getRank(C64(1) << h1) == RANK1);
	assert(bitboard_getRank(C64(1) << a1) == RANK1);

	assert(bitboard_getFile(C64(1) << a8) == FILEA);
	assert(bitboard_getFile(C64(1) << h8) == FILEH);
	assert(bitboard_getFile(C64(1) << e4) == FILEE);
	assert(bitboard_getFile(C64(1) << d5) == FILED);
	assert(bitboard_getFile(C64(1) << h1) == FILEH);
	assert(bitboard_getFile(C64(1) << a1) == FILEA);
}

void test_DiagNW()
{
	printf("Test diag NW\n");
	assert(bitboard_getDiagNW(C64(1) << a8) == C64(0x0102040810204080));
	assert(bitboard_getDiagNW(C64(1) << h1) == C64(0x0102040810204080));
	assert(bitboard_getDiagNW(C64(1) << a2) == C64(0x0000000000000102));
	assert(bitboard_getDiagNW(C64(1) << b1) == C64(0x0000000000000102));
	assert(bitboard_getDiagNW(C64(1) << a1) == C64(0x0000000000000001));
	assert(bitboard_getDiagNW(C64(1) << h8) == C64(0x8000000000000000));
	assert(bitboard_getDiagNW(C64(1) << g8) == C64(0x4080000000000000));
	assert(bitboard_getDiagNW(C64(1) << h7) == C64(0x4080000000000000));
}

void test_DiagNE()
{
	printf("Test diag NE()\n");
	assert(bitboard_getDiagNE(C64(1) << a8) == C64(0x0100000000000000));
	assert(bitboard_getDiagNE(C64(1) << h1) == C64(0x0000000000000080));
	assert(bitboard_getDiagNE(C64(1) << a7) == C64(0x0201000000000000));
	assert(bitboard_getDiagNE(C64(1) << b8) == C64(0x0201000000000000));
	assert(bitboard_getDiagNE(C64(1) << a1) == C64(0x8040201008040201));
	assert(bitboard_getDiagNE(C64(1) << h8) == C64(0x8040201008040201));
	assert(bitboard_getDiagNE(C64(1) << g1) == C64(0x0000000000008040));
	assert(bitboard_getDiagNE(C64(1) << h2) == C64(0x0000000000008040));
}

void test_bin2alg()
{
	printf("Test bin to alg\n");
	assert(strcmp(bitboard_binToAlg(C64(1) << a1), "a1") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << a2), "a2") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << a3), "a3") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << a4), "a4") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << a5), "a5") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << a6), "a6") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << a7), "a7") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << a8), "a8") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << b1), "b1") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << b2), "b2") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << b3), "b3") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << b4), "b4") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << b5), "b5") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << b6), "b6") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << b7), "b7") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << b8), "b8") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << c1), "c1") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << c2), "c2") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << c3), "c3") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << c4), "c4") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << c5), "c5") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << c6), "c6") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << c7), "c7") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << c8), "c8") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << d1), "d1") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << d2), "d2") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << d3), "d3") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << d4), "d4") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << d5), "d5") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << d6), "d6") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << d7), "d7") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << d8), "d8") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << e1), "e1") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << e2), "e2") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << e3), "e3") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << e4), "e4") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << e5), "e5") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << e6), "e6") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << e7), "e7") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << e8), "e8") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << f1), "f1") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << f2), "f2") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << f3), "f3") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << f4), "f4") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << f5), "f5") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << f6), "f6") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << f7), "f7") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << f8), "f8") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << g1), "g1") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << g2), "g2") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << g3), "g3") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << g4), "g4") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << g5), "g5") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << g6), "g6") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << g7), "g7") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << g8), "g8") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << h1), "h1") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << h2), "h2") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << h3), "h3") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << h4), "h4") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << h5), "h5") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << h6), "h6") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << h7), "h7") == 0);
	assert(strcmp(bitboard_binToAlg(C64(1) << h8), "h8") == 0);
}

void test_alg2bin()
{
	printf("Test alg to bin\n");
	assert(bitboard_algToBin("a1") == C64(1) << a1);
	assert(bitboard_algToBin("a2") == C64(1) << a2);
	assert(bitboard_algToBin("a3") == C64(1) << a3);
	assert(bitboard_algToBin("a4") == C64(1) << a4);
	assert(bitboard_algToBin("a5") == C64(1) << a5);
	assert(bitboard_algToBin("a6") == C64(1) << a6);
	assert(bitboard_algToBin("a7") == C64(1) << a7);
	assert(bitboard_algToBin("a8") == C64(1) << a8);
	assert(bitboard_algToBin("b1") == C64(1) << b1);
	assert(bitboard_algToBin("b2") == C64(1) << b2);
	assert(bitboard_algToBin("b3") == C64(1) << b3);
	assert(bitboard_algToBin("b4") == C64(1) << b4);
	assert(bitboard_algToBin("b5") == C64(1) << b5);
	assert(bitboard_algToBin("b6") == C64(1) << b6);
	assert(bitboard_algToBin("b7") == C64(1) << b7);
	assert(bitboard_algToBin("b8") == C64(1) << b8);
	assert(bitboard_algToBin("c1") == C64(1) << c1);
	assert(bitboard_algToBin("c2") == C64(1) << c2);
	assert(bitboard_algToBin("c3") == C64(1) << c3);
	assert(bitboard_algToBin("c4") == C64(1) << c4);
	assert(bitboard_algToBin("c5") == C64(1) << c5);
	assert(bitboard_algToBin("c6") == C64(1) << c6);
	assert(bitboard_algToBin("c7") == C64(1) << c7);
	assert(bitboard_algToBin("c8") == C64(1) << c8);
	assert(bitboard_algToBin("d1") == C64(1) << d1);
	assert(bitboard_algToBin("d2") == C64(1) << d2);
	assert(bitboard_algToBin("d3") == C64(1) << d3);
	assert(bitboard_algToBin("d4") == C64(1) << d4);
	assert(bitboard_algToBin("d5") == C64(1) << d5);
	assert(bitboard_algToBin("d6") == C64(1) << d6);
	assert(bitboard_algToBin("d7") == C64(1) << d7);
	assert(bitboard_algToBin("d8") == C64(1) << d8);
	assert(bitboard_algToBin("e1") == C64(1) << e1);
	assert(bitboard_algToBin("e2") == C64(1) << e2);
	assert(bitboard_algToBin("e3") == C64(1) << e3);
	assert(bitboard_algToBin("e4") == C64(1) << e4);
	assert(bitboard_algToBin("e5") == C64(1) << e5);
	assert(bitboard_algToBin("e6") == C64(1) << e6);
	assert(bitboard_algToBin("e7") == C64(1) << e7);
	assert(bitboard_algToBin("e8") == C64(1) << e8);
	assert(bitboard_algToBin("f1") == C64(1) << f1);
	assert(bitboard_algToBin("f2") == C64(1) << f2);
	assert(bitboard_algToBin("f3") == C64(1) << f3);
	assert(bitboard_algToBin("f4") == C64(1) << f4);
	assert(bitboard_algToBin("f5") == C64(1) << f5);
	assert(bitboard_algToBin("f6") == C64(1) << f6);
	assert(bitboard_algToBin("f7") == C64(1) << f7);
	assert(bitboard_algToBin("f8") == C64(1) << f8);
	assert(bitboard_algToBin("g1") == C64(1) << g1);
	assert(bitboard_algToBin("g2") == C64(1) << g2);
	assert(bitboard_algToBin("g3") == C64(1) << g3);
	assert(bitboard_algToBin("g4") == C64(1) << g4);
	assert(bitboard_algToBin("g5") == C64(1) << g5);
	assert(bitboard_algToBin("g6") == C64(1) << g6);
	assert(bitboard_algToBin("g7") == C64(1) << g7);
	assert(bitboard_algToBin("g8") == C64(1) << g8);
	assert(bitboard_algToBin("h1") == C64(1) << h1);
	assert(bitboard_algToBin("h2") == C64(1) << h2);
	assert(bitboard_algToBin("h3") == C64(1) << h3);
	assert(bitboard_algToBin("h4") == C64(1) << h4);
	assert(bitboard_algToBin("h5") == C64(1) << h5);
	assert(bitboard_algToBin("h6") == C64(1) << h6);
	assert(bitboard_algToBin("h7") == C64(1) << h7);
	assert(bitboard_algToBin("h8") == C64(1) << h8);
}
