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
#include <stdlib.h>
#include "tt.h"
#include "prng.h"

Zobrist zobrist;

static TranspositionTable *tt;
static int tt_size;

int tt_setsize(int size) 
{
	/* 
	check if size is a power of 2
	if not, make it the next lower power of 2
	this allows for a faster access of the entry needed:

	as sizeof(TranspositionTable) in our case is 16 Bytes long (see definition of TranspositionTable),
	we are creating size / 16 tt entries. The idea of making the size a power of 2
	is important when accessing the table. By 'anding' the hash value and the number
	of entries -1 (tt_size), we get a number in the range of 0 and the number of
	entries very quickly, this is used to index the entry.
	*/

	free(tt);
	int i;
	/*
	check if size is a power of 2
	if not, make it the next lower power of 2
	this allows for a faster access of the entry needed:
	*/
	if (size & (size - 1)) {
		size--;
		for (i=1; i < 32; i=i*2) {
			size |= size >> i;
		}

		size++;
		size >>= 1;
	}

	if (size < 16) {
		tt_size = 0;
		return 0;
	}

	tt_size = (size / sizeof(TranspositionTable)) -1;
	tt = (TranspositionTable *) malloc(size);

	return 1;
}

/**
 * @param int size in bytes
 */
void tt_init(int size) 
{
	int p, s, castling, ep = 0;
	/* fill the zobrist struct with random numbers */
	for (p = 0; p <= 11; p++) {
		for (s = 0; s <= 63; s++) {
			zobrist.piecesquare[p][s] = rand64();
		}
	}

	zobrist.side = rand64();
	
	for (castling = 0; castling <= 15; castling++) {
		zobrist.castling[castling] = rand64();
	}

	for (ep = 0; ep <= 63; ep++) {
		zobrist.ep[ep] = rand64();
	}

	assert(tt_setsize(size));
}

void tt_save(U64 hash, int val, U16 depth, U16 flag)
{
	if (!tt_size) return;
	TranspositionTable * entry = &tt[hash & tt_size];
	
	// The only criteria in deciding whether to overwrite an entry is 
	// whether the new entry has a higher depth than the old entry if exists.
	if ((entry->hash == hash) && (entry->depth > depth)) return;

	entry->hash = hash;
	entry->val = val;
	entry->depth = depth;
	entry->flag = flag;
}


int tt_probe(U64 hash, int alpha, int beta, U16 depth)
{
	if (!tt_size) return 0;

	TranspositionTable * entry = &tt[hash & tt_size];
	
	/*
	Index collisions or type-2 errors , 
	where different hash keys index same entries, happen regularly. 
	They require detection, realized by storing the signature as part 
	of the hash entry, to check whether a stored entry matches 
	the position while probing.
	*/

	if ((hash == entry->hash) && (entry->depth >= depth)) {
		// return entry->val;
		if (entry->flag == TT_EXACT) {
			return entry->val;
		}

		if ((entry->flag == TT_ALPHA) && (entry->val <= alpha)) {
			return alpha;
		}

		if ((entry->flag == TT_BETA) && (entry->val >= beta)) {
			return beta;
		}
	}

	return 0;
}

void tt_perft_save(U64 hash, int data, int depth)
{
	if (!tt_size) return;
	TranspositionTable * entry = &tt[hash & tt_size];
	
	// The only criteria in deciding whether to overwrite an entry is 
	// whether the new entry has an equal depth than the old entry if exists.
	if ((entry->hash == hash) && (entry->depth != depth)) return;

	entry->hash = hash;
	entry->val = data;
	entry->depth = depth;
}


int tt_perft_probe(U64 hash, int depth)
{
	if (!tt_size) return 0;

	TranspositionTable * entry = &tt[hash & tt_size];

	if (hash == entry->hash  && (entry->depth == depth)) {
		return entry->val;
	}

	return 0;
}




