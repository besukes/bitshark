#include <engine.h>
#include <stdlib.h>


int depth_search(GameStruct * game , SearchInfo * search_info , int cur_depth){
    int cur_alpha = evaluate_pos(game,search_info,0) ,
        cur_beta = evaluate_pos(game,search_info,1)  ;
    if(cur_depth >= search_info->depth || cur_alpha < search_info->alpha || cur_beta > search_info->beta) 
        return ((search_info->turn == brancas) ? cur_alpha : cur_beta);
    int new_turn = (search_info->turn == brancas) ? pretas : brancas;
    return move_algorithm(game,new_turn,search_info->depth-1,search_info->ai_level,cur_alpha,cur_beta).move_evaluation;
    //necessita de verificar o turno atual e aplicar a melhor jogada , decrementando o turno e fazendo recursividade para procurar
    //os proximos melhor moves
    //secalhar utilizar a funcao move_algorithm e fazer algumas alteracoes
}


Moves search_algorithm (uint64_bit atks , uint64_bit pos, GameStruct * game ,PieceEvaluation evals[2][NUMBER_PIECES] , SearchInfo * search_info){
    int cntr = 0;
    uint64_bit casa_atual = 0 , bst = 0;
    while(atks!=0){
        if(atks & 1ULL){
            casa_atual = 1ULL<<cntr;
            atualizaJogada(game,casa_atual,0,0);
            search_info->turn = (search_info->turn == brancas) ? pretas : brancas;
            int new_eval = depth_search(game,search_info,0);
            //quando o turno troca , se chegarmos ao fim da depth estamos a conseguir a evaluation da jogada que fizemos , com o turno ja trocado
            if(new_eval > search_info->alpha && search_info->turn == brancas){
                search_info->alpha = new_eval;
                bst = casa_atual;
            }
            else if(new_eval < search_info->beta && search_info->turn == pretas){
                search_info->beta = new_eval;
                bst = casa_atual;
            }
            undoMove(game,casa_atual,pos,0,search_info->piece_type,search_info->turn);
        }
        atks>>=1; 
        cntr++;
    }
    Moves ret = {.move = bst , .move_evaluation = search_info->alpha};
    return ret;
}