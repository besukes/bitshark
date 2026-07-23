#include <engine.h>
#include <stdlib.h>


int depth_search(GameStruct * game , SearchInfo * search_info , int piece_evals[2][NUMBER_PIECES] , int cur_piece_eval){
    int cur_alpha = -99999 , cur_beta = 99999;
    evaluate_pos(game,search_info,&cur_alpha,&cur_beta,piece_evals,cur_piece_eval);
    if(search_info->depth <= 0 || cur_alpha < search_info->alpha || cur_beta > search_info->beta) 
        return ((search_info->turn == brancas) ? cur_alpha : cur_beta); 
    int new_turn = (search_info->turn == brancas) ? pretas : brancas;
    return move_algorithm(game,new_turn,search_info->depth-1,search_info->ai_level,cur_alpha,cur_beta).move_evaluation;
    //necessita de verificar o turno atual e aplicar a melhor jogada , decrementando o turno e fazendo recursividade para procurar
    //os proximos melhor moves
    //secalhar utilizar a funcao move_algorithm e fazer algumas alteracoes
}


Moves search_algorithm (uint64_bit atks , uint64_bit pos, GameStruct * game ,int piece_evals[2][NUMBER_PIECES] , SearchInfo * search_info){
    int cntr = 0;
    uint64_bit casa_atual = 0 , bst = 0;
    CorPiece turn = search_info->turn;
    int piece_eval = piece_evals[turn][search_info->piece_type];
    while(atks!=0){
        if(atks & 1ULL){
            casa_atual = 1ULL<<cntr;
            atualizaJogada(game,casa_atual,0,0,turn);
            if(!is_in_check(&game->estadoJogo,game->estadoJogo.tabuleirojogo[turn][King],turn)){
                int new_eval = depth_search(game,search_info,piece_evals,piece_eval);
                if(new_eval > search_info->alpha && search_info->turn == brancas){
                    search_info->alpha = new_eval;
                    bst = casa_atual;
                }
                else if(new_eval < search_info->beta && search_info->turn == pretas){
                    search_info->beta = new_eval;
                    bst = casa_atual;
                }
            }
            undoMove(game,casa_atual,pos,0,search_info->piece_type,search_info->turn);
            piece_evals[turn][search_info->piece_type] = piece_eval;
        }
        atks>>=1; 
        cntr++;
    }
    Moves ret = {.move = bst , .move_evaluation = search_info->alpha};
    return ret;
}