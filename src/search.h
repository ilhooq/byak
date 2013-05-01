#ifndef SEARCH_H
#define SEARCH_H

void* search_start(void* data);
void search_stop();
void search_iterate();
int search_root(int alpha, int beta, U8 depth);
void search_root_negamax(int depth);
int search_negamax(int depth, int ply);
int search_alphaBeta(int alpha, int beta, int depth, int ply);
int search_quiesce(int alpha, int beta);
#endif
