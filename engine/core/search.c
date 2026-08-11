#include "engine/chess_lib/engine.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#define NO_FLAGS 0
#define FLAG_ONLY_CAPTURES 1




int quiescence(GameStruct * game, int alpha, int beta, int quiescence_eval, CorPiece turn , int q_depth , int init_time , int max_time){
    int est_eval = (turn == brancas) ? quiescence_eval : (-quiescence_eval); // Avaliação estática da posição atual
    if (SDL_GetTicks() - init_time >= max_time) {
        return FLAG_TIMEOUT;
    }
    if (est_eval >= beta) return beta;
    if (alpha < est_eval) alpha = est_eval;
    
    int orig_alpha = alpha;
    Jogada * hash_move = NULL; int move_eval = 0;
    uint64_bit key = compute_zobrist(game,turn);
    getPositionTTMove(key,0,&alpha,&beta,&move_eval,&hash_move);
    if(move_eval != 0) return move_eval;

    Jogada jogadas[256];
    int num_jogadas = gerar_jogadas_legais(game, jogadas, turn, FLAG_ONLY_CAPTURES); // idealmente só capturas aqui
    moveScoringCaptures(jogadas, num_jogadas, hash_move); // Ordena as jogadas de captura para melhorar a poda alpha-beta

    int best_score = -VALOR_INFINITO - 1;
    Jogada best_move_found = jogadas[0];

    for (int i = 0; i < num_jogadas; i++){
        Jogada * best_move = pick_best_move(jogadas, num_jogadas, i);
        CorPiece op_turn = (turn == brancas) ? pretas : brancas;
        int delta = applyDeltaMove(game,best_move,turn,op_turn);
        if(!is_in_check(&game->estadoJogo,game->estadoJogo.tabuleirojogo[turn][King],turn)){
            int eval = -quiescence(game, -beta, -alpha, quiescence_eval + delta, (turn==brancas)?pretas:brancas , q_depth + 1 , init_time , max_time);
            undoMove(game,best_move,turn);
            if (eval >= beta){
                history_table[jogadas[i].peca_movida][jogadas[i].destino] += q_depth * q_depth; // Atualiza a tabela de histórico para capturas
                tt_store(key, 0 , beta, TT_LOWERBOUND, *best_move);
                return beta;
            }
            if((-eval) == FLAG_TIMEOUT) {
                return FLAG_TIMEOUT;
            }
            alpha = (eval > alpha) ? eval : alpha;
        }
        else undoMove(game,best_move,turn);
    }
    TTFlag flag = (best_score > orig_alpha) ? TT_EXACT : TT_UPPERBOUND;
    tt_store(key, 0 , alpha, flag, best_move_found);
    return alpha;
}



// A função Search usando Negamax + Alpha-Beta
int search(GameStruct * game, int depth, int alpha, int beta, int wb_eval , double initial_time, double time_limit , CorPiece turn){
    SDL_Event e ; SDL_PollEvent(&e);
    if (SDL_GetTicks() - initial_time >= time_limit || (e.type == SDL_QUIT)) {
        return FLAG_TIMEOUT;
    }

    if (depth == 0) { // Quando atinge a profundidade 0 ou o jogo acaba, lê a avaliação incremental atual
        double time = SDL_GetTicks();
        return quiescence(game, alpha, beta, wb_eval, turn, MAX_DEPTH_SEARCH , time , time_limit);
    }

    Jogada jogadas[256];
    int num_jogadas = gerar_jogadas_legais(game, jogadas,turn, NO_FLAGS);
    
    int orig_alpha = alpha;
    Jogada * hash_move = NULL; int move_eval = 0;
    uint64_bit key = compute_zobrist(game,turn);
    getPositionTTMove(key,depth,&alpha,&beta,&move_eval,&hash_move);
    if(is_repeated_position(key)) return 0;
    if(move_eval != 0) return move_eval;
    moveScoring(jogadas, num_jogadas, hash_move , depth); // Ordena as jogadas para melhorar a poda alpha-beta , ainda nao existe hash_moves
    
    int best_score = -VALOR_INFINITO - 1;
    Jogada best_move_found = jogadas[0];
    int legal_moves = 0;
    hash_key_stack[hash_stack_indx++] = key;
    for (int i = 0; i < num_jogadas; i++) {
        Jogada * best_move = pick_best_move(jogadas, num_jogadas, i);
        CorPiece op_turn = (turn == brancas) ? pretas : brancas;

        int delta = applyDeltaMove(game,best_move,turn,op_turn);
        Boolean in_check = is_in_check(&game->estadoJogo,game->estadoJogo.tabuleirojogo[turn][King],turn);
        if(!in_check){
            legal_moves = 1;
            // Chamada recursiva do NEGAMAX:
            int eval = -search(game, depth - 1 , -beta, -alpha, wb_eval + delta, initial_time, time_limit, (turn == brancas) ? pretas : brancas);
            undoMove(game,best_move,turn);
            // Se o tempo acabou em algum nó filho, propaga o timeout para cima sem salvar nada
            if ((-eval) == FLAG_TIMEOUT) {
                hash_stack_indx--;
                return FLAG_TIMEOUT;
            }
            if(eval > best_score){
                best_score = eval;
                best_move_found = *best_move;
            }
            // 4. PODA ALPHA-BETA (Pruning):
            // Se a avaliação atual ultrapassa o Beta do adversário, ele nunca deixará esta posição acontecer.
            if (eval >= beta) {
                if(best_move->peca_capturada == Empty) {
                    // Se não for uma captura, registra como um killer move
                    killer_moves[depth][1] = killer_moves[depth][0];
                    killer_moves[depth][0] = *best_move;

                    history_table[best_move->peca_movida][best_move->destino] += depth * depth; // Atualiza a tabela de histórico
                }
                tt_store(key, depth, beta, TT_LOWERBOUND, *best_move);
                hash_stack_indx--;
                return beta;
            }
            alpha = (eval > alpha) ? eval : alpha; // Atualiza o Alpha se a avaliação atual for melhor
        }
        else undoMove(game,best_move,turn);
    }
    hash_stack_indx--;
    if(!legal_moves){
        if (is_in_check(&game->estadoJogo,game->estadoJogo.tabuleirojogo[turn][King],turn)){
            return (-VALOR_INFINITO + depth); // Xeque-mate (prioriza mates mais rápidos)
        }
        return 0; // Empate por afogamento
    }
    //Se best_score > orig_alpha , entao encontramos uma jogada melhor , caso contrario esta jogada piora a posicao (fail)
    TTFlag flag = (best_score > orig_alpha) ? TT_EXACT : TT_UPPERBOUND;
    tt_store(key, depth, alpha, flag, best_move_found);
    return alpha;
}
