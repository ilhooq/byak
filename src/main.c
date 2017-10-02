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
#include "time.h"
#include "uci.h"

#define MAX_INPUT_SIZE 1024

int main (int argc, char ** argv) {

	bitboard_init();
	prng_init(73);
	/* 144MB */
	tt_init(144000000);
	position_init();
	eval_init();

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

			uci_exec(input);

		}
	}

	return 0;
}


