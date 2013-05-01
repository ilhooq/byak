#ifndef TEST_H
#define TEST_H

void testSuite();
void test_kingMoves();
void test_knightMoves();
void test_bin2alg();
void test_alg2bin();
void test_FileRankAccess();
void test_DiagNW();
void test_DiagNE();
void test_magicMoves();
void testInBetweenSquares();
void test_fen();
void test_move();
void test_linkedList();
void test_genMoves(const char *fen);
U64 test_perft(const char *fen, int depth);
void test_perftAdvanced(const char *fen, int depth);
void test_divide(const char *fen, int depth);
void test_perftSuite();
void test_perftSuite2();
void test_castle();
void testTT();
void testSearch(const char *fen, int depth);
#endif
