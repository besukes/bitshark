#include "engine/chess_lib/engine.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#define NO_FLAGS 0
#define FLAG_ONLY_CAPTURES 1

/*  Quiescence search funciona de modo a evitar um "Efeito Horizonte" na procura.
    Esta função executa após serem executados todos os moves da depth da search (entre 1-MAX_DEPTH_SEARCH) , e procura possíveis capturas
que possam acontecer , até não encontrar mais , ou seja , evita acabar a pesquisa em posições complexas onde podem haver muitas transposições vantajosas
para um lado , e assim prefere acabar a pesquisa numa posição mais calma e melhor avaliável.
*/
int quiescence(GameStruct * game, int alpha, int beta, int quiescence_eval, CorPiece turn , int q_depth , int init_time , int max_time){
    CorPiece op_turn = (turn == brancas) ? pretas : brancas;
    total_nodes_searched++;
    int stc_eval = quiescence_eval; // Avaliação estática da posição atual
    //Como depth agora é 0  , é seguro usar mopup evaluation aqui , pois assim não ha inflação de valores
    //Est_eval nunca é usada para além de verificações
    stc_eval += mopup_eval(game);
    stc_eval = (turn==brancas) ? stc_eval : -stc_eval;

    if (SDL_GetTicks() - init_time >= max_time) {
        return FLAG_TIMEOUT;
    }
    if (stc_eval >= beta) return beta;
    if (alpha < stc_eval) alpha = stc_eval;
    
    int orig_alpha = alpha;
    Jogada * hash_move = NULL; int move_eval = 0;
    uint64_bit key = game->cur_pos_key;
    getPositionTTMove(key,0,&alpha,&beta,&move_eval,&hash_move,0);
    // Transposition table mostrou nos que é um beta cutoff
    if(alpha >= beta) return move_eval;

    Jogada jogadas[MAX_NUMBER_MOVES];
    int num_jogadas = gerar_jogadas_legais(game, jogadas, turn, FLAG_ONLY_CAPTURES); // idealmente só capturas aqui
    moveScoringCaptures(game,jogadas, num_jogadas, hash_move,turn); // Ordena as jogadas de captura para melhorar a poda alpha-beta

    int best_eval = -2*VALOR_INFINITO;
    Jogada best_move_found = jogadas[0];

    for (int i = 0; i < num_jogadas; i++){
        pick_best_move(jogadas, num_jogadas, i);
        if(jogadas[i].score >= 0){
            int delta = applyDeltaMove(game,&jogadas[i],turn,op_turn);
            if(!is_in_check(&game->estadoJogo,game->estadoJogo.tabuleirojogo[turn][King],turn)){
                int eval = -quiescence(game, -beta, -alpha, quiescence_eval + delta, op_turn , q_depth + 1 , init_time , max_time);
                undoMove(game,&jogadas[i],turn);
                if((-eval) == FLAG_TIMEOUT) {
                    return FLAG_TIMEOUT;
                }
                if(eval > best_eval){
                    best_eval = eval;
                    best_move_found = jogadas[i];
                }
                if (eval >= beta){
                    history_table[jogadas[i].peca_movida][jogadas[i].destino] += q_depth * q_depth; // Atualiza a tabela de histórico para capturas
                    tt_store(key, 0 , beta, TT_LOWERBOUND, jogadas[i],0);
                    return beta;
                }
                alpha = (eval > alpha) ? eval : alpha;
            }
            else undoMove(game,&jogadas[i],turn);
        }
    }
    if(num_jogadas>0){
        TTFlag flag = (best_eval > orig_alpha) ? TT_EXACT : TT_UPPERBOUND;
        tt_store(key, 0 , alpha, flag, best_move_found,0);
    }
    return alpha;
}


/*Retorna a zobrist key e enpassant para oque ela era antes do null move*/
void undoNullMove(GameStruct * game , int passant){
    uint64_bit h = game->cur_pos_key;
    if(passant != (-1)){
        h^=zobrist_ep[passant];
        game->estadoJogo.enpassant = (1ULL<<passant);
    }
    h^=zobrist_turn;
    game->cur_pos_key = h;
}


/*Apenas altera a zobrist key para representar o novo turno e passa enpassant para 0 porque não seria válido.*/
void applyNullMove(GameStruct * game , int prev_enpassant){
    uint64_bit h = game->cur_pos_key;
    if(prev_enpassant != (-1)){
        h^=zobrist_ep[prev_enpassant];
        game->estadoJogo.enpassant = 0;
    }
    h^=zobrist_turn;
    game->cur_pos_key = h;
}


/*  A função nullmovepruning baseia se em apenas passar o turno do jogador *turn* e verificar se a vantagem é maior que beta , se isso acontecer , então a 
sequência de moves até este é demasiado boa e causa um beta cutoff.*/
int nullmovepruning(GameStruct * game , int in_check , int depth ,int beta, int ply , int wb_eval , int timeI , int budget , CorPiece turn , CorPiece op_turn , int* prunes , int allows_nmp){
    if(allows_nmp && depth >= 3 && !in_check && ply > 0 && has_non_pawn_material(game,turn)){
        int R = (depth > 6) ? 3 : 2; // Redução: quanto maior a profundidade, mais confiamos na poda
        int null_depth = depth - 1 - R;
        if(null_depth < 0) null_depth = 0;
        int prev_enpassant = posTabuleiro(game->estadoJogo.enpassant);
        //Aplicar null move
        applyNullMove(game,prev_enpassant);
        //Search com null move
        //Agora search é chamada com allows_null = 0 , porque aplicar dois null moves num mesmo ramo é arriscado
        int null_eval = -search(game, null_depth, -beta, -beta+1, wb_eval, timeI , budget, op_turn, ply+1,0);
        //Undo null move
        undoNullMove(game,prev_enpassant);
        if(null_eval == FLAG_TIMEOUT){
            *prunes = 1;
            return FLAG_TIMEOUT;
        }
        if(null_eval >= beta){
            *prunes = 1;
            return beta;
        }
    }
    return 0;
}


/* A função Search usando Negamax + Alpha-Beta , com outras técnicas como :
    ->Principal Variation Search - Procura os moves depois do primeiro melhor com uma janela reduzida [alpha,alpha+1];
    ->Late Move Reductions - Reduz a profundidade de moves mais tardios por serem considerados inferiores;
    ->Null Move Pruning - Passa a sua vez um turno e verifica se a posição continua muito superior , se sim não vale a pena continuar a pesquisar;
*/
int search(GameStruct * game, int depth, int alpha, int beta, int wb_eval , double initial_time, double time_limit , CorPiece turn , int ply , int allows_nmp){
    total_nodes_searched++;
    if (SDL_GetTicks() - initial_time >= time_limit) {
        return FLAG_TIMEOUT;
    }
    // Quando atinge a profundidade 0 ou o jogo acaba, lê a avaliação incremental atual
    if (depth == 0) return quiescence(game, alpha, beta, wb_eval, turn, MAX_DEPTH_SEARCH , initial_time , time_limit);

    // Acessar à tranposition table para ver qual move ela dá para esta posição
    int orig_alpha = alpha;
    Jogada * hash_move = NULL; 
    int hash_move_eval = 0;
    uint64_bit key = game->cur_pos_key;
    getPositionTTMove(key,depth,&alpha,&beta,&hash_move_eval,&hash_move,ply);
    if(is_repeated_position(key)) return 0;
    // Transposition table mostrou que é um beta cutoff
    if(alpha >= beta) return (hash_move_eval);

    CorPiece op_turn = (turn == brancas) ? pretas : brancas;
    int starts_in_check = is_in_check(&game->estadoJogo,game->estadoJogo.tabuleirojogo[turn][King],turn);

    //Null move pruning para otimizar a procura
    int safe2prune = 0;
    int nmp = nullmovepruning(game,starts_in_check,depth,beta,ply,wb_eval,initial_time,time_limit,turn,op_turn,&safe2prune,allows_nmp);
    if(safe2prune) return nmp;

    //Get all legal moves
    Jogada jogadas[MAX_NUMBER_MOVES];
    int num_jogadas = gerar_jogadas_legais(game, jogadas,turn, NO_FLAGS);
    moveScoring(game,jogadas, num_jogadas, hash_move , depth , turn); // Ordena as jogadas para melhorar a poda alpha-beta , ainda nao existe hash_moves

    int best_score = -2*VALOR_INFINITO;
    Jogada best_move_found = jogadas[0];
    int legal_moves = 0;
    hash_key_stack[hash_stack_indx++] = key;

    for (int i = 0; i < num_jogadas; i++) {
        // 0. Incremental Sort & Incremental evaluation , depois de ordenados os moves , começamos por escolher o melhor deles e aplicar uma
        // incremental evaluation ao move atual , de modo a não ter de recalcular a evaluation em todas as folhas da search tree
        pick_best_move(jogadas, num_jogadas, i); //This function sets jogadas[i] to be the best move in terms of score
        int delta = applyDeltaMove(game,&jogadas[i],turn,op_turn);
        Boolean in_check = is_in_check(&game->estadoJogo,game->estadoJogo.tabuleirojogo[turn][King],turn);
        if(!in_check){ // Verifica se a jogada é válida (rei atual não fica em check)
            int op_king_in_check = is_in_check(&game->estadoJogo,game->estadoJogo.tabuleirojogo[op_turn][King],op_turn);
            // 1. Late Move reductions , só procura os primeiros 3 ordered moves em full depth , a menos que eles apresentem uma vantagem promissora
            int can_apply_lmr = i >= 3 && depth >= 3 , 
                isnt_important_move = !jogadas[i].promocao && (jogadas[i].peca_capturada == Empty || jogadas[i].score < 0)
                                     && !op_king_in_check && !starts_in_check;
            int applied_reduction = (can_apply_lmr && isnt_important_move) ? lmr_lt[depth][i] : 0;
            int reduced_depth = (applied_reduction) ? maximum(1,depth - 1 - applied_reduction) : (depth - 1);

            // 2. Principal Variation Search , depois do primeiro move , procura numa window [alpha,alpha+1] ao invés de [alpha,beta]
            // Se esse move ultrapassar alpha , então pvs foi refutado , e portanto procuramos na full window [alpha,beta]
            int pvs_beta = (i==0) ? beta : (alpha + 1); //Define a janela
            legal_moves = 1; //Para verificações de checkmate
            // 3. Chamada recursiva do NEGAMAX:
            int eval = -search(game, reduced_depth , -pvs_beta, -alpha, wb_eval + delta, initial_time, time_limit, op_turn , ply + 1 , allows_nmp);
            // So faz full depth search se a jogada for muito boa , dentro da janela alpha-beta
            // Volta a procurar com null window
            if(applied_reduction && eval > alpha)
                    eval = -search(game , depth - 1 , -pvs_beta , -alpha , wb_eval+delta , initial_time , time_limit, op_turn , ply + 1 , allows_nmp);
            // Re-pesquisa na profundidade normal com JANELA CHEIA
            if (eval > alpha && eval < beta && i > 0)
                eval = -search(game, depth - 1, -beta, -alpha, wb_eval + delta, initial_time, time_limit, op_turn , ply + 1 , allows_nmp);
            undoMove(game,&jogadas[i],turn);
            // Se o tempo acabou em algum nó filho, propaga o timeout para cima sem salvar nada
            if ((-eval) == FLAG_TIMEOUT) {
                hash_stack_indx--;
                return FLAG_TIMEOUT;
            }
            // Se a nova eval for melhor do que a ultima que tinhamos arranjado , entao
            if(eval > best_score){best_score = eval;best_move_found = jogadas[i];}
            // 4. PODA ALPHA-BETA (Pruning):
            // Se a avaliação atual ultrapassa o Beta do adversário, ele nunca deixará esta posição acontecer.
            if (eval >= beta) {
                if(jogadas[i].peca_capturada == Empty) {
                    // Se não for uma captura, registra como um killer move
                    killer_moves[depth][1] = killer_moves[depth][0];
                    killer_moves[depth][0] = jogadas[i];

                    history_table[jogadas[i].peca_movida][jogadas[i].destino] += depth * depth; // Atualiza a tabela de histórico
                }
                // Guardamos um move na transposition table como um LOWERBOUND (causou beta pruning antes)
                tt_store(key, depth, beta, TT_LOWERBOUND, jogadas[i] , ply);
                hash_stack_indx--;
                return beta;
            }
            alpha = (eval > alpha) ? eval : alpha; // Atualiza o Alpha se a avaliação atual for melhor
        }
        else undoMove(game,&jogadas[i],turn);
    }
    hash_stack_indx--;
    //Se não tiverem sido executado moves nenhuns , então é porque os movimentos eram inválidos
    if(!legal_moves){
        if(starts_in_check){
            return (-VALOR_INFINITO + ply); // Xeque-mate ,prioriza mates mais rápidos
        }
        return 0; // Empate por afogamento
    }
    //Se best_score > orig_alpha , entao encontramos uma jogada melhor , caso contrario esta jogada piora a posicao (fail)
    TTFlag flag = (best_score > orig_alpha) ? TT_EXACT : TT_UPPERBOUND;
    tt_store(key, depth, alpha, flag, best_move_found , ply);
    return alpha;
}
