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

#ifndef TEST_H
#define TEST_H

void test_suite();
void test_search(const char *fen);
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
void test_genMoves(const char *fen);
void test_perft(const char *fen, int depth);
void test_divide(const char *fen, int depth);
void test_perftSuite();
#endif
