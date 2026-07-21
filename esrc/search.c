#include <engine.h>
#include <stdlib.h>


int depth_search(GameStruct * game , SearchInfo search_info , int cur_depth){
    int cur_alpha = evaluate_pos(game,search_info,0) ,
        cur_beta = evaluate_pos(game,search_info,1)  ;
    if(cur_depth == search_info.depth || cur_alpha < search_info.alpha) return cur_alpha;
    move_algorithm(game,search_info.turn,cur_depth-1,search_info.ai_level,cur_beta);
    //necessita de verificar o turno atual e aplicar a melhor jogada , decrementando o turno e fazendo recursividade para procurar
    //os proximos melhor moves
    //secalhar utilizar a funcao move_algorithm e fazer algumas alteracoees
}


Moves search_algorithm (uint64_bit atks , uint64_bit pos, GameStruct * game ,PieceEvaluation evals[2][NUMBER_PIECES] , SearchInfo search_info){
    int cntr = 0;
    uint64_bit casa_atual = 0 , bst = 0;
    while(atks!=0){
        if(atks & 1ULL){
            casa_atual = 1ULL<<cntr;
            atualizaJogada(game,casa_atual,0,0);
            search_info.turn = (search_info.turn == brancas) ? pretas : brancas;
            int new_alpha = depth_search(game,search_info,0);
            if(new_alpha > search_info.alpha){
                search_info.alpha = new_alpha;
                bst = casa_atual;
            }
            else undoMove(game,casa_atual,pos,0,search_info.piece_type,search_info.turn);
        }
        atks>>=1; 
        cntr++;
    }
    Moves ret = {.move = bst , .move_evaluation = search_info.alpha};
    return ret;
}