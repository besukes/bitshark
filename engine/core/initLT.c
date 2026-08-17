#include <engine/chess_lib/engine.h>
#include <math.h>

unsigned long long passed_pawn_mask[2][64];




void init_lmrLT_table(){
    for(int depth=1;depth<MAX_DEPTH_SEARCH;depth++){
        for(int jogadas=1;jogadas<MAX_NUMBER_MOVES;jogadas++){
            lmr_lt[depth][jogadas] = (int)( 0.6 + (log(depth)*log(jogadas))/1.6);
        }
    }
}


void init_passedPawnsLT_table(){
    for(int i=0;i<64;i++){
        int line = i/8 , col = i%8;
        int next_line = line + 1 , ant_line = line - 1;
        uint64_bit pp_mask_white = 0 , pp_mask_black = 0;
        int lower_limit = (col > 0) ? (col - 1) : col;
        int upper_limit = (col < 7) ? (col + 1) : col;
        for(int j=next_line;j<=7;j++){
            pp_mask_white |= (1ULL<<(j*8 + lower_limit));
            pp_mask_white |= (1ULL<<(j*8 + col));
            pp_mask_white |= (1ULL<<(j*8 + upper_limit));
        }
        for(int j=ant_line;j>=0;j--){
            pp_mask_black |= (1ULL<<(j*8 + lower_limit));
            pp_mask_black |= (1ULL<<(j*8 + col));
            pp_mask_black |= (1ULL<<(j*8 + upper_limit));
        }
        passed_pawn_mask[brancas][i] = pp_mask_white;
        passed_pawn_mask[pretas][i] = pp_mask_black;
    }
}