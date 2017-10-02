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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "bitboard.h"
#include "position.h"
#include "eval.h"
#include "prng.h"
#include "tt.h"
#include "test.h"
#include "time.h"
#include "uci.h"

Protocol protocol = DEFAULT;

#define MAX_INPUT_SIZE 1024

int main (int argc, char ** argv) {

	bitboard_init();
	prng_init(73);
	/* 144MB */
	tt_init(144000000);
	position_init();
	eval_init();

	int i;

	for (i=0; i < argc; i++) {

		if (!strcmp(argv[i], "testsuite")) {
			test_suite();
			return 0;
		}

		if (!strcmp(argv[i], "timesearch")) {
			/* Command to test a search against a fen string with a 
			   limited amount of time (in ms) for each side.
			   For instance :
			  ./byak testsearch "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1" 300000
			*/
			if (i+1 < argc) {
				char * fen = argv[i+1];
				if (i+2 < argc) {
					int time = atoi(argv[i+2]);
					test_timeSearch(fen, time);
				}
			}
			return 0;
		}

		if (!strcmp(argv[i], "depthsearch")) {
			/* Command to test a search against a fen string with a 
			   limited depth.
			   For instance :
			  ./byak testsearch "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1" 6
			*/
			if (i+1 < argc) {
				char * fen = argv[i+1];
				if (i+2 < argc) {
					int depth = atoi(argv[i+2]);
					test_depthSearch(fen, depth);
				}
			}
			return 0;
		}

		if (!strcmp(argv[i], "perft")) {
			/* Perft against a fen string.
			   byak perft [fenstring] [depth]
			   For instance :
			  ./byak perft "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1" 3
			*/
			if (i+1 < argc) {
				char * fen = argv[i+1];
				if (i+2 < argc) {
					int depth = atoi(argv[i+2]);
					test_perft(fen, depth);
				}
			}
			return 0;
		}

		if (!strcmp(argv[i], "divide")) {
			if (i+1 < argc) {
				char * fen = argv[i+1];
				if (i+2 < argc) {
					int depth = atoi(argv[i+2]);
					test_divide(fen, depth);
				}
			}
			return 0;
		}
		
		if (!strcmp(argv[i], "eval")) {
			/* Evaluate a position a fen string.
			   For instance :
			  ./byak eval "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"
			*/
			if (i+1 < argc) {
				char * fen = argv[i+1];
				test_eval(fen);
			}
			return 0;
		}
	}

	printf("Chess Engine By Sylvain Philip\n");

	/* deactivate buffering */
	setvbuf(stdin,  NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	char input[MAX_INPUT_SIZE];

	while(1) {
		
		if (fgets(input, MAX_INPUT_SIZE, stdin) != NULL) {
			/* Remove the newline character */
			int s = strlen(input);
			if (s > 1 && input[s-1] == '\n') {
				input[s-1] = '\0';
			}

			if (!strcmp(input, "uci")) {
				protocol = UCI;
			}
			
			if (!strcmp(input, "help")) {
				// print help
			}

			if (!strncmp(input, "perft", 5)) {
				int start, timeused, depth;
				float nps;
				depth =  atoi(input + 6);

				start = GET_TIME();
				U64 nodes = search_perft(depth);
				timeused = GET_TIME() - start;
				nps = (float) nodes / ((float) timeused /1000);
				printf("depth:%i;time:%i;nodes:%llu;nps:%.0f\n", depth, timeused, ULL(nodes), nps);
			}

			switch (protocol) {
				case UCI : uci_exec(input); break;
				/* case XBOARD: break;  Not yet implemented */
				default:
					break;
			}
		}
	}

	return 0;
}


