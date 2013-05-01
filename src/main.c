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
#include <string.h>
#include "types.h"
#include "bitboard.h"
#include "position.h"
#include "prng.h"
#include "tt.h"
#include "test.h"
#include "uci.h"

Protocol protocol = DEFAULT;

#define MAX_INPUT_SIZE 1024

int main (int argc, char ** argv) {
	printf("Chess Engine By Sylvain Philip\n");
	bitboard_init();
	prng_init(73);
	/* 144MB */
	tt_init(144000000);
	position_init();

	// testSearch("r1bqkbnr/p1p2ppp/1pnp4/1B2p3/3PP3/5N2/PPP2PPP/RNBQK2R w KQkq - 0 5", 4);
	// testSearch("6k1/1R6/8/R7/8/8/8/4K3 w - - 0 1", 4);
	// testSearch("5K2/1r6/8/8/r7/3k4/8/8 b - - 0 1", 4);
	// test_perftSuite2();
	// return 0;
	

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


