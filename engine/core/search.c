#include "engine/chess_lib/engine.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#define NO_FLAGS 0
#define FLAG_ONLY_CAPTURES 1


int quiescence(GameStruct * game, int alpha, int beta, int quiescence_eval, CorPiece turn , int q_depth , int init_time , int max_time){
    CorPiece op_turn = (turn == brancas) ? pretas : brancas;
    int stc_eval = quiescence_eval; // Avaliação estática da posição atual
    //Como depth agora é 0  , é seguro usar mopup evaluation aqui , pois assim não ha inflação de valores
    //Est_eval nunca é usada para além de verificações
    stc_eval += mopup_eval(game,op_turn);
    stc_eval = (turn==brancas) ? stc_eval : -stc_eval;


    if (SDL_GetTicks() - init_time >= max_time) {
        return FLAG_TIMEOUT;
    }
    if (stc_eval >= beta) return beta;
    if (alpha < stc_eval) alpha = stc_eval;
    
    int orig_alpha = alpha;
    Jogada * hash_move = NULL; int move_eval = 0;
    uint64_bit key = compute_zobrist(game,turn);
    getPositionTTMove(key,0,&alpha,&beta,&move_eval,&hash_move);
    if(move_eval != 0) return move_eval;

    Jogada jogadas[MAX_NUMBER_MOVES];
    int num_jogadas = gerar_jogadas_legais(game, jogadas, turn, FLAG_ONLY_CAPTURES); // idealmente só capturas aqui
    moveScoringCaptures(jogadas, num_jogadas, hash_move); // Ordena as jogadas de captura para melhorar a poda alpha-beta

    int best_score = -VALOR_INFINITO - 1;
    Jogada best_move_found = jogadas[0];

    for (int i = 0; i < num_jogadas; i++){
        Jogada * best_move = pick_best_move(jogadas, num_jogadas, i);
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
int search(GameStruct * game, int depth, int alpha, int beta, int wb_eval , double initial_time, double time_limit , CorPiece turn , int ply){
    SDL_Event e ; SDL_PollEvent(&e);
    if (SDL_GetTicks() - initial_time >= time_limit || (e.type == SDL_QUIT)) {
        return FLAG_TIMEOUT;
    }
    // Quando atinge a profundidade 0 ou o jogo acaba, lê a avaliação incremental atual
    if (depth == 0) return quiescence(game, alpha, beta, wb_eval, turn, MAX_DEPTH_SEARCH , initial_time , time_limit);

    Jogada jogadas[MAX_NUMBER_MOVES];
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
    CorPiece op_turn = (turn == brancas) ? pretas : brancas;


    for (int i = 0; i < num_jogadas; i++) {
        // 0. Incremental Sort & Incremental evaluation , depois de ordenados os moves , começamos por escolher o melhor deles e aplicar uma
        // incremental evaluation ao move atual , de modo a não ter de recalcular a evaluation em todas as folhas da search tree
        Jogada * best_move = pick_best_move(jogadas, num_jogadas, i);
        int delta = applyDeltaMove(game,best_move,turn,op_turn);
        Boolean in_check = is_in_check(&game->estadoJogo,game->estadoJogo.tabuleirojogo[turn][King],turn);
        if(!in_check){ // Verifica se a jogada é válida (rei atual não fica em check)
            // 1. Late Move reductions , it only searches the first 3 moves full depth unless the latter ones it get a really nice eval
            int can_apply_lmr = i >= 3 && depth >= 3 , 
                isnt_important_move = !best_move->promocao && best_move->peca_capturada == Empty;
            int applied_reduction = (can_apply_lmr && isnt_important_move) ? lmr_lt[depth][i] : 0;
            int reduced_depth = (applied_reduction) ? maximum(1,depth - 1 - applied_reduction) : (depth - 1);

            // 2. Principal Variation Search , depois do primeiro move , procura numa window [alpha,alpha+1] ao invés de [alpha,beta
            // Se esse move ultrapassar alpha , então pvs foi refutado , e portanto procuramos na full window [alpha,beta]
            int pvs_beta = (i==0) ? beta : (alpha + 1); //Define a janela
            legal_moves = 1; //Para verificações de checkmate
            // 3. Chamada recursiva do NEGAMAX:
            int eval = -search(game, reduced_depth , -pvs_beta, -alpha, wb_eval + delta, initial_time, time_limit, op_turn , ply + 1);
            // So faz full depth search se a jogada for muito boa , dentro da janela alpha-beta
            // Volta a procurar com null window
            if(applied_reduction && eval > alpha)
                    eval = -search(game , depth - 1 , -pvs_beta , -alpha , wb_eval+delta , initial_time , time_limit, op_turn , ply + 1);
            // Re-pesquisa na profundidade normal com JANELA CHEIA
            if (eval > alpha && eval < beta && i > 0)
                eval = -search(game, depth - 1, -beta, -alpha, wb_eval + delta, initial_time, time_limit, op_turn , ply + 1);
            undoMove(game,best_move,turn);
            // Se o tempo acabou em algum nó filho, propaga o timeout para cima sem salvar nada
            if ((-eval) == FLAG_TIMEOUT) {
                hash_stack_indx--;
                return FLAG_TIMEOUT;
            }
            // Se a nova eval for melhor do que a ultima que tinhamos arranjado , entao
            if(eval > best_score){best_score = eval;best_move_found = *best_move;}
            // 4. PODA ALPHA-BETA (Pruning):
            // Se a avaliação atual ultrapassa o Beta do adversário, ele nunca deixará esta posição acontecer.
            if (eval >= beta) {
                if(best_move->peca_capturada == Empty) {
                    // Se não for uma captura, registra como um killer move
                    killer_moves[depth][1] = killer_moves[depth][0];
                    killer_moves[depth][0] = *best_move;

                    history_table[best_move->peca_movida][best_move->destino] += depth * depth; // Atualiza a tabela de histórico
                }
                // Guardamos um move na transposition table como um LOWERBOUND (causou beta pruning antes)
                tt_store(key, depth, beta, TT_LOWERBOUND, *best_move);
                hash_stack_indx--;
                return beta;
            }
            alpha = (eval > alpha) ? eval : alpha; // Atualiza o Alpha se a avaliação atual for melhor
        }
        else undoMove(game,best_move,turn);
    }
    hash_stack_indx--;
    //Se não tiverem sido executado moves nenhuns , então é porque os movimentos eram inválidos
    if(!legal_moves){
        if (is_in_check(&game->estadoJogo,game->estadoJogo.tabuleirojogo[turn][King],turn)){
            return (-VALOR_INFINITO + ply - mopup_eval(game,turn)); // Xeque-mate ,prioriza mates mais rápidos e com o rei nos cantos do tabuleiro
        }
        return 0; // Empate por afogamento
    }
    //Se best_score > orig_alpha , entao encontramos uma jogada melhor , caso contrario esta jogada piora a posicao (fail)
    TTFlag flag = (best_score > orig_alpha) ? TT_EXACT : TT_UPPERBOUND;
    tt_store(key, depth, alpha, flag, best_move_found);
    return alpha;
}
