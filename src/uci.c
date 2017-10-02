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
#include "move.h"
#include "search.h"
#include "time.h"


static void uci_go(char * command)
{
	char * subcommand = NULL;
	SearchInfos infos = {0};

	infos.time[WHITE] = 0;
	infos.time[BLACK] = 0;

	if ((subcommand = strstr(command, "wtime"))) {
		infos.time[WHITE] = atoi(subcommand+6);
	}

	if ((subcommand = strstr(command, "btime"))) {
		infos.time[BLACK] = atoi(subcommand+6);
	}

	if ((subcommand = strstr(command, "infinite"))) {
		infos.depth = MAX_DEPTH;
	}

	#if !defined(_WIN32) && !defined(_WIN64)
	/* Linux - Unix */
	pthread_t SearchThread;
	pthread_create(&SearchThread, NULL, search_start, &infos);
	/* Don't need to call pthread_join() as the thread never calls pthread_exit() */
	#else
	/* windows and Mingw */
	HANDLE SearchThread;
	DWORD  threadId;
	SearchThread = CreateThread( 
		NULL,         // default security attributes
		0,            // use default stack size  
		(LPTHREAD_START_ROUTINE) search_start, // thread function name
		&infos,       // argument to thread function 
		0,            // use default creation flags 
		&threadId);   // returns the thread identifier

		CloseHandle(SearchThread);
	#endif
}

static Move uci_prepare_move(U64 from, U64 to, Piece promotion )
{
	Move move;
	move.from = lsb(from);
	move.to = lsb(to);
	move.flags = MOVE_NORMAL;
	move.captured_piece = NONE_PIECE;
	move.castling_rights = 0;
	move.ep = NONE_SQUARE;

	/* Check castling */
	if ((from & pos.bb_pieces[K]) && 
		(move.from == e1 && (move.to == g1 || move.to == c1))) {
		move.flags = MOVE_CASTLE;
	}
	if ((from & pos.bb_pieces[k]) && 
		(move.from == e8 && (move.to == g8 || move.to == c8))) {
		move.flags = MOVE_CASTLE;
	}
	/* check promotion */
	if (promotion != NONE_PIECE) {
		move.flags = MOVE_PROMOTION;
		switch(promotion) {
			case Q : move.flags |= MOVE_PROMOTION_QUEEN; break;
			case R : move.flags |= MOVE_PROMOTION_ROOK; break;
			case B : move.flags |= MOVE_PROMOTION_BISHOP; break;
			case K : move.flags |= MOVE_PROMOTION_KNIGHT; break;
			default: break;
		}
	}
	/* check capture */
	if (pos.bb_occupied & to) {
		move.flags |= MOVE_CAPTURE;
	}

	if (from & pos.bb_pieces[P + pos.side]) {
		/* check double pawn*/
		if (abs(move.from - move.to) == 16) {
			move.flags = MOVE_PAWN_DOUBLE;
		}
		/* check enpassant */
		if (!(move.flags & MOVE_CAPTURE) && 
			(abs(move.from - move.to) == 9 || abs(move.from - move.to) == 7)) {
			move.flags = (MOVE_CAPTURE|MOVE_ENPASSANT);
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
			promotion = NONE_PIECE;

			/* check for promotion */
			if (moves[4] && moves[4] != ' ') {
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

void uci_exec(char * command)
{
	if (!strcmp(command, "uci")) {
		printf("id name chess_engine\n");
		printf("id author Sylvain Philip\n");

		/* the engine can change the hash size from 1 to 128 MB */
		printf("option name Hash type spin default 64 min 1 max 1024\n");
		/* the engine has sent all parameters and is ready */
		printf("uciok\n");
	}

	if (!strcmp(command, "isready")) {
		printf("readyok\n");
	}

	if (!strncmp(command, "setoption", 9)) {
		char name[256];
		char value[256];

		sscanf(command, "setoption name %255s value %255s", name, value);

		if (!strcmp(name, "Hash")) {
			int val;
			sscanf(value, "%4u", &val);
			/* transform val to a power of two number */
			tt_setsize(val << 20);
		}
	}

	if (!strcmp(command, "ucinewgame")) {
		
	}

	if (!strncmp(command, "position", 8)) {
		/* position [fen | startpos] [moves ...] */
		position_init();

		if (!strncmp(command,"position fen",12)) {
			position_fromFen(command + 13);
		} else {
			position_fromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		}

		char * moves = strstr(command, "moves");
		if (moves) {
			uci_parse_moves(moves+6);
		}
	}
	
	if (!strncmp(command, "go", 2)) {
		uci_go(command);
	}

	if (!strncmp(command, "debug", 5)) {
		/* 
		debug = strcmp(command,"debug off");
		*/
	}

	if (!strcmp(command, "ponderhit")) {
		/*
		time_uci_ponderhit();
		*/
	}

	if (!strcmp(command, "stop")) {
		search_stop();
	}

	if (!strcmp(command, "quit")) {
		exit(EXIT_SUCCESS);
	}

	/* Not UCI protocol */

	if (!strcmp(command, "display")) {
		position_display();
	}
}

void uci_print_move(Move *move)
{
	printf("%s%s%c", bitboard_binToAlg(SQ64(move->from)),
					 bitboard_binToAlg(SQ64(move->to)), 
					 move_getPromotionPieceChar(move->flags));
}

void uci_print_currmove(Move * move, int depth, int mvNbr)
{
	printf("info depth %d currmove ", depth);
	uci_print_move(move);
	printf(" currmovenumber %d\n",  mvNbr);
}

void uci_print_pv(int score, int depth, SearchInfos * infos)
{
	int timeused = GET_TIME() - infos->time_start;
	
	printf("info depth %i score cp %i nodes %i time %i", depth, score, infos->nodes, timeused);

	printf(" pv ");

	int j;
	for (j = 0; j < infos->pv_length[0]; ++j) {
		uci_print_move(&infos->pv[0][j]);
		printf(" ");
	}

	printf("\n");
}

void uci_print_nps(int time_start, int nodes)
{
	if (!nodes) return;
	float time_used_in_sec, nps;

	time_used_in_sec = (float) (GET_TIME() - time_start) / 1000;

	if (!time_used_in_sec) return;

	nps = (float) nodes / time_used_in_sec;

	printf("info nps %.0f\n", nps);
}

void uci_print_bestmove(Move * move)
{
	printf("bestmove ");
	uci_print_move(move);
	printf("\n");
}



