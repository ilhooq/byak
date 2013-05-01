#ifndef EVAL_H
#define EVAL_H
#define INFINITY 10000
#include "move.h"
// void eval_init(U8 side);
int eval_position();
int eval_move(Move * move);
#endif
