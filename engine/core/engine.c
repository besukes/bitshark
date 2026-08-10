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
    int completed;
}jogadabot;

jogadabot timeout_reached_move(GameStruct * game , Jogada jogadas[256] , CorPiece turn , int n , int eval){
    jogadabot move = {.move_time = 5000};
    for(int i=0;i<n;i++){
        Jogada * cur_move = pick_best_move(jogadas, n, i);
        CorPiece op_turn = (turn == brancas) ? pretas : brancas;
        int delta = applyDeltaMove(game,cur_move,turn,op_turn);
        if(!is_in_check(&game->estadoJogo,game->estadoJogo.tabuleirojogo[turn][King],turn)){
            eval += delta;
            move.move_eval = eval;
            move.best_move = jogadas[i];
            return move;
        }
        else undoMove(game,cur_move,turn);
    }
    Jogada invalid = {.destino = 64 , .origem = 64 , .peca_capturada = Empty , .peca_movida = Empty,
                      .especial = 0 , .promocao = 0 , .score = 0};
    int who2Move = (turn == brancas) ? 1 : (-1);
    move.move_eval = VALOR_INFINITO*who2Move;
    move.best_move = invalid;
    return move;
}

jogadabot engine_search(GameStruct * game , CorPiece turn , int depth , double initial_time , double budget){
    Jogada jogadas[256];
    int num_jogadas = gerar_jogadas_legais(game, jogadas,turn, NO_FLAGS);
    // Consulta a transposition table para obter uma "hash move" que ajuda a ordenar
    // melhor as jogadas logo desde a raiz.
    uint64_bit hash_key = compute_zobrist(game, turn);
    TTEntry * tt_entry = tt_probe(hash_key);
    Jogada * hash_move = (tt_entry != NULL) ? &tt_entry->best_move : NULL;

    moveScoring(jogadas, num_jogadas, hash_move, depth); // Ordena as jogadas para melhorar a poda alpha-beta , ainda nao existe hash_moves
    int melhor_eval = -VALOR_INFINITO ,
        alpha = -VALOR_INFINITO,
        beta = VALOR_INFINITO;
    int eval_wb_inicial = evaluate(game, brancas); // avaliação completa, calculada só uma vez (na raiz)
    Jogada best_move = {.origem = 64, .destino = 64, .peca_movida = Empty, .peca_capturada = Empty, .promocao = 0, .especial = 0};

    int orig_alpha = alpha;
    for (int i = 0; i < num_jogadas; i++) {
        Jogada * cur_move = pick_best_move(jogadas, num_jogadas, i);
        CorPiece op_turn = (turn == brancas) ? pretas : brancas;
        // Aplica a jogada nas Bitboards e atualiza a Avaliação Incremental (Delta)
        int delta = applyDeltaMove(game,cur_move,turn,op_turn);
        Boolean in_check = is_in_check(&game->estadoJogo,game->estadoJogo.tabuleirojogo[turn][King],turn);
        if(!in_check){
            // Chamada recursiva do NEGAMAX:
            int eval = -search(game, depth - 1 , -beta , -alpha, eval_wb_inicial + delta, initial_time, budget , op_turn);
            undoMove(game,cur_move,turn);
            // Se o tempo acabou em algum nó filho, propaga o timeout para cima sem salvar nada
            if((-eval) == FLAG_TIMEOUT) {
                printf("[engine] engine_search: timeout reached during search\n");
                return (timeout_reached_move(game,jogadas,turn,num_jogadas,eval_wb_inicial));
            }
            // Guarda a melhor pontuação encontrada para o jogador atual
            if(eval > melhor_eval) {
                melhor_eval = eval;
                best_move = *cur_move;
            }
            alpha = (melhor_eval>alpha) ? melhor_eval : alpha;
        }
        else undoMove(game,cur_move,turn);
    }
    int completed = best_move.origem != 64;
    if(completed){
        TTFlag flag = (melhor_eval > orig_alpha) ? TT_EXACT : TT_UPPERBOUND;
        tt_store(hash_key, depth, melhor_eval, flag, best_move);
    }
    jogadabot result = {.best_move = best_move,.move_eval = alpha ,.move_time = SDL_GetTicks() - initial_time , .completed = completed};
    return result;
}


jogadabot iterative_deepening(GameStruct * game , CorPiece turn , int * reached_depth){
    double initial_time = SDL_GetTicks();
    const double time_budget = 3000; // orçamento total para a jogada, partilhado por todas as profundidades
    jogadabot best_so_far = {0};
    // Iterative deepening: pesquisa profundidade 1, depois 2, 3... até MAX_DEPTH_SEARCH
    for(int depth = 1; depth <= MAX_DEPTH_SEARCH; depth++){
        double elapsed = SDL_GetTicks() - initial_time;
        if(elapsed >= time_budget) break;
        jogadabot result = engine_search(game, turn, depth, initial_time, time_budget);
        if(result.completed){
            best_so_far = result;
            *reached_depth = depth;
        }
        else{
            if(*reached_depth == 0){
                best_so_far = result;
            }
            break;
        }
    }
    return best_so_far;
}


Jogada get_best_move(GameStruct * game , CorPiece turn , int is_interative_deepening){
    double initial_time = SDL_GetTicks();
    int reached_depth = 0;
    jogadabot best_jogada = {0};
    if(is_interative_deepening){
        best_jogada = iterative_deepening(game,turn,&reached_depth);
    }
    else{
        reached_depth = 5;
        best_jogada = engine_search(game,turn,MAX_DEPTH_SEARCH - 2,initial_time,3000);
    }
    int who2Move = (turn==brancas) ? 1 : (-1);
    printf("[engine] get_best_move: piece %d from %d to %d , depth alcancada %d/%d , took %d ms with an eval of %f\n", best_jogada.best_move.peca_movida, 
                (int)best_jogada.best_move.origem, (int)best_jogada.best_move.destino, reached_depth, MAX_DEPTH_SEARCH,
                (int)(SDL_GetTicks() - initial_time), (float)(best_jogada.move_eval*who2Move) / 100);
    return (best_jogada.best_move);
}

