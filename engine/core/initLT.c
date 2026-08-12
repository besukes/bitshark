#include <engine/chess_lib/engine.h>
#include <math.h>



void init_lmrLT_table(){
    for(int depth=1;depth<MAX_DEPTH_SEARCH;depth++){
        for(int jogadas=1;jogadas<MAX_NUMBER_MOVES;jogadas++){
            lmr_lt[depth][jogadas] = (int)( 0.6 + (log(depth)*log(jogadas))/1.6);
        }
    }
}
