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
#if !defined(_WIN32) && !defined(_WIN64)
/* Linux - Unix */
#include <pthread.h>
#else
/* windows and Mingw */
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "bitboard.h"
#include "tt.h"
#include "position.h"
#include "search.h"

static void uci_go(char * token)
{
	// search_start();
	#if !defined(_WIN32) && !defined(_WIN64)
	/* Linux - Unix */
	pthread_t SearchThread;
	pthread_create(&SearchThread, NULL, search_start, NULL);
	/* Don't need to call pthread_join() as the thread never calls pthread_exit() */
	#else
	/* windows and Mingw */
	HANDLE SearchThread;
	DWORD  threadId;
	SearchThread = CreateThread( 
		NULL,         // default security attributes
		0,            // use default stack size  
		(LPTHREAD_START_ROUTINE) search_start, // thread function name
		NULL,         // argument to thread function 
		0,            // use default creation flags 
		&threadId);   // returns the thread identifier

		CloseHandle(SearchThread);
	#endif
}

static Move uci_prepare_move(U64 from, U64 to, Piece promotion )
{
	Move move;
	move.from = bitboard_bitScanForward(from);
	move.to = bitboard_bitScanForward(to);
	move.capture = 0;
	move.type = NORMAL;
	move.promoted_piece = NONE_PIECE;
	move.captured_piece = NONE_PIECE;
	move.castling_rights = 0;
	move.ep = NONE_SQUARE;

	/* Check castling */
	if ((from & pos.bb_pieces[K]) && 
		(move.from == e1 && (move.to == g1 || move.to == c1))) {
		move.type = CASTLE;
	}
	if ((from & pos.bb_pieces[k]) && 
		(move.from == e8 && (move.to == g8 || move.to == c8))) {
		move.type = CASTLE;
	}
	/* check promotion */
	if (promotion != NONE_PIECE) {
		move.type = PROMOTION;
		move.promoted_piece = promotion + pos.side;
	}
	/* check capture */
	if (pos.bb_occupied & to) {
		move.capture = 1;
	}

	if (from & pos.bb_pieces[P + pos.side]) {
		/* check double pawn*/
		if (abs(move.from - move.to) == 16) {
			move.type = PAWN_DOUBLE;
		}
		/* check enpassant */
		if (!move.capture && 
			(abs(move.from - move.to) == 9 || abs(move.from - move.to) == 7)) {
			move.capture = 1;
			move.type = ENPASSANT;
		}
	}
	return move;
}


static void uci_parse_moves(const char * moves)
{
	U64 from = EMPTY;
	U64 to = EMPTY;
	Square promotion = NONE_PIECE;

	while (moves[0]) {
		if ((moves[0] >= 'a') && (moves[0] <= 'h')) {

			from = bitboard_algToBin(moves);
			to = bitboard_algToBin(moves+2);

			/* check for promotion */
			if (moves[4] != ' ') {
				switch(moves[4]) {
					case 'q': promotion = Q; moves++; break;
					case 'r': promotion = R; moves++; break;
					case 'b': promotion = B; moves++; break;
					case 'n': promotion = N; moves++; break;
				}
			}

			Move move = uci_prepare_move(from, to, promotion);
			// It still a bug on win32 : move.capture == 1 when there is no capture 
			// move_display(&move);
			position_makeMove(&move);

			/* Go to next move */
			moves += 4;
		} else {
			moves++;
		}
	}
}

void uci_exec(char * token)
{
	if (!strcmp(token, "uci")) {
		printf("id name chess_engine\n");
		printf("id author Sylvain Philip\n");

		/* the engine can change the hash size from 1 to 128 MB */
		printf("option name Hash type spin default 64 min 1 max 1024\n");
		/* the engine has sent all parameters and is ready */
		printf("uciok\n");
	}

	if (!strcmp(token, "isready")) {
		printf("readyok\n");
	}

	if (!strncmp(token, "setoption", 9)) {
		char name[256];
		char value[256];

		sscanf(token, "setoption name %255s value %255s", name, value);

		if (!strcmp(name, "Hash")) {
			int val;
			sscanf(value, "%4u", &val);
			/* transform val to a power of two number */
			tt_setsize(val << 20);
		}
	}

	if (!strcmp(token, "ucinewgame")) {
		printf("Not implemented\n");
	}

	if (!strncmp(token, "position", 8)) {
		/* position [fen | startpos] [moves ...] */
		position_init();

		if (!strncmp(token,"position fen",12)) {
			position_fromFen(token + 13);
		} else {
			position_fromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		}

		char * moves = strstr(token, "moves");
		if (moves) {
			uci_parse_moves(moves+6);
		}
	}
	
	if (!strncmp(token, "go", 2)) {
		uci_go(token);
	}

	if (!strncmp(token, "debug", 5)) {
		/* 
		debug = strcmp(token,"debug off");
		*/
	}

	if (!strcmp(token, "ponderhit")) {
		/*
		time_uci_ponderhit();
		*/
	}

	if (!strcmp(token, "stop")) {
		search_stop();
	}

	if (!strcmp(token, "quit")) {
		exit(EXIT_SUCCESS);
	}

	/* Not UCI protocol */

	if (!strcmp(token, "display")) {
		position_display();
	}
}



