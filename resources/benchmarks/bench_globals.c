#include "engine/chess_lib/engine.h"

Jogada killer_moves[MAX_DEPTH_SEARCH][2] = {0};
int history_table[NUMBER_PIECES*2][NUM_SQUARES] = {0};

uint64_bit zobrist_pieces[2][NUMBER_PIECES][64];
uint64_bit zobrist_castle[2][2];
uint64_bit zobrist_ep[64];
uint64_bit zobrist_turn;

int hash_stack_indx = 0;
uint64_bit hash_key_stack[2048];

int lmr_lt[MAX_DEPTH_SEARCH][256];

unsigned long total_nodes_searched = 0;
