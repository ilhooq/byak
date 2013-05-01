
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "tt.h"
#include "position.h"
#include "search.h"

pthread_t SearchThread;

static void uci_go(char * token)
{
	// search_start();
	pthread_create(&SearchThread, NULL, search_start, NULL);
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
		position_init();
		printf("readyok\n");
	}

	if (!strncmp(token, "setoption", 9)) {
		char name[256];
		char value[256];

		sscanf(token, "setoption name %s value %s", name, value);

		if (!strcmp(name, "Hash")) {
			int val;
			sscanf(value, "%d", &val);
			/* transform val to a power of two number */
			tt_setsize(val << 20);
		}
	}

	if (!strcmp(token, "ucinewgame")) {
		printf("Not implemented\n");
	}

	if (!strncmp(token, "position", 8)) {
		//position [fen | startpos] [moves ...]

		if (!strncmp(token,"position fen",12)) {
			position_fromFen(token + 13);
		} else {
			position_fromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		}
		/*
		char * moves = strstr(token, "moves");
		if (moves) algebraic_moves(moves+6);
		*/
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
}



