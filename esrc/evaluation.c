#include <engine.h>
#include <evals.h>

//Still havent done any of the evals , work in progress
//Evaluation functions need to do a incremental evaluation to only check the new piece moved evaluation and compare it
//Changing alpha and beta depending to that comparation

int evaluate_pos(GameStruct * game , SearchInfo * search , int * cur_alpha , int * cur_beta , int pieces_evals[2][NUMBER_PIECES] , int cur_piece_eval){
    int past_alpha = search->alpha ,  past_beta = search->beta;
    
}


int evaluate(GameStruct * game , CorPiece turno , int ai_level , int pieces_evals[2][NUMBER_PIECES]){
    int eval = 0;

    return eval;
}