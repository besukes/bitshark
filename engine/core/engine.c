#include "engine/chess_lib/engine.h"
#include <stdio.h>
#include <SDL2/SDL.h>

#define NO_FLAGS 0

//alpha usually starts at -99999 , int alpha = -99999; --Current best white evaluation
//beta usually starts at 99999 , int beta = 99999; --Current best black evaluation
//alpha - white eval , beta - black eval

typedef struct jogadabot{
    Jogada best_move;
    int move_eval;
    int move_time;
}jogadabot;


jogadabot engine_search(GameStruct * game , CorPiece turn , int depth){
    CorPiece op_turn = (turn == brancas) ? pretas : brancas;
    Jogada jogadas[256];
    int num_jogadas = gerar_jogadas_legais(game, jogadas,turn, NO_FLAGS);
    moveScoring(jogadas, num_jogadas, NULL, depth); // Ordena as jogadas para melhorar a poda alpha-beta , ainda nao existe hash_moves
    int melhor_eval = -VALOR_INFINITO ,
        alpha = -VALOR_INFINITO,
        beta = VALOR_INFINITO;
    double initial_time = SDL_GetTicks();
    int eval_wb_inicial = evaluate(game, brancas); // avaliação completa, calculada só uma vez (na raiz)
    Jogada best_move = {.origem = 64, .destino = 64, .peca_movida = Empty, .peca_capturada = Empty, .promocao = 0, .especial = 0};
    for (int i = 0; i < num_jogadas; i++) {
        Jogada * cur_move = pick_best_move(jogadas, num_jogadas, i);
        // Aplica a jogada nas Bitboards e atualiza a Avaliação Incremental (Delta)
        int delta = applyDeltaMove(game,cur_move,turn);
        // Chamada recursiva do NEGAMAX:
        int eval = -search(game, depth - 1, -beta , -alpha, eval_wb_inicial + delta, initial_time, initial_time + 5000, op_turn);
        undoMove(game,cur_move,turn);
        // Se o tempo acabou em algum nó filho, propaga o timeout para cima sem salvar nada
        if((-eval) == FLAG_TIMEOUT) {
            printf("[engine] engine_search: timeout reached during search\n");
            return (jogadabot){{.origem = 64, .destino = 64, .peca_movida = Empty, .peca_capturada = Empty, .promocao = 0, .especial = 0}
                                , FLAG_TIMEOUT
                                , SDL_GetTicks() - initial_time};
        }
        // Guarda a melhor pontuação encontrada para o jogador atual
        if(eval > melhor_eval) {
            melhor_eval = eval;
            best_move = *cur_move;
        }
        alpha = (melhor_eval>alpha) ? melhor_eval : alpha;
    }
    jogadabot result = {.best_move = best_move,.move_eval = alpha ,.move_time = SDL_GetTicks() - initial_time};
    return result;
}



Jogada get_best_move(GameStruct * game , CorPiece turn){
    int depth = MAX_DEPTH_SEARCH;
    jogadabot best_move = engine_search(game,turn,depth);
    printf("[engine] get_best_move: piece %d from %d to %d , took %d ms with an eval of %f\n", best_move.best_move.peca_movida, 
                (int)best_move.best_move.origem, (int)best_move.best_move.destino, best_move.move_time, (float)(best_move.move_eval / 100));
    return (best_move.best_move);
}

