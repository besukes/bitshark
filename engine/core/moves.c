#include "engine/chess_lib/engine.h"
#include <engine/chess_lib/evals.h>


Jogada * pick_best_move(Jogada * jogadas, int num_jogadas, int start_index) {
    int best_indx = start_index;
    for (int i = start_index + 1; i < num_jogadas; i++) {
        if (jogadas[i].score > jogadas[best_indx].score) {
            best_indx = i;
        }
    }
    Jogada temp = jogadas[start_index];
    jogadas[start_index] = jogadas[best_indx];
    jogadas[best_indx] = temp;
    return &jogadas[start_index];
}


int matches_killer_move(int depth, Jogada * jogada, int index) {
    if (depth < 0 || depth >= MAX_DEPTH_SEARCH) {
        return 0; // Out of bounds
    }
    Jogada * killer = &killer_moves[depth][index];
    return (killer->origem == jogada->origem && killer->destino == jogada->destino && killer->peca_movida == jogada->peca_movida);
}


void moveScoringCaptures(GameStruct * game ,Jogada * jogadas , int num_jogadas , Jogada * hash_move , CorPiece turn){
    for(int i=0;i<num_jogadas;i++){
        Jogada * atual = &jogadas[i];
        Boolean matches_hash_move = (hash_move != NULL && atual->origem == hash_move->origem 
                                        && atual->destino == hash_move->destino 
                                        && hash_move->peca_movida == atual->peca_movida);
        if(matches_hash_move){
            atual->score = 2000000;
        }
        else{
            int captured = atual->peca_capturada;
            int moved = atual->peca_movida;
            if(captured < 0 || captured >= NUMBER_PIECES || moved < 0 || moved >= NUMBER_PIECES){
                printf("[moveScoringCaptures] Invalid piece indices: piece moved %d captured piece %d in position %d\n", 
                    moved, captured, atual->destino);
            }
            else{
                int see = static_exchange_eval(game,atual,turn);
                if(see<0) atual->score = see;
                else{
                    atual->score = mvv_lva_table[captured][moved];
                    atual->score += history_table[moved][atual->destino];
                    atual->score += see;
                    if(atual->promocao) atual->score += 100*pieces_value[atual->promocao];
                }
            }
        }
    }
}


void moveScoring(GameStruct * game ,Jogada * jogadas , int num_jogadas , Jogada * hash_move , int depth , CorPiece turn){
    for(int i=0;i<num_jogadas;i++){
        Jogada * atual = &jogadas[i];
        Boolean matches_hash_move = (hash_move != NULL && atual->origem == hash_move->origem 
                                        && atual->destino == hash_move->destino 
                                        && hash_move->peca_movida == atual->peca_movida);
        if(matches_hash_move){
            atual->score = 2000000;
        }
        else if(atual->peca_capturada != Empty){
            int captured = atual->peca_capturada;
            int moved = atual->peca_movida;
            int see = static_exchange_eval(game,atual,turn);
            if(see<0) atual->score = see;
            else atual->score = 1000000 + mvv_lva_table[captured][moved] + see;
        }
        else if(matches_killer_move(depth,atual,1)){
            atual->score = 900000;
        }
        else if(matches_killer_move(depth,atual,0)){
            atual->score = 800000;
        }
        else{
            atual->score = history_table[atual->peca_movida][atual->destino];
            if(atual->promocao) atual->score += 100*pieces_value[atual->promocao];
        }
    }
}


int is_repeated_position(uint64_bit key){
    int counter = 0;
    for(int i = hash_stack_indx - 1;i>=0;i--){
        if(hash_key_stack[i] == key) counter++;
        //We have to check if the position has occurred twice because if we repeat it the 3rd time 
        //Opponent might repeated it the 4th time , so we need to acknowledge it at the 2nd time it occurs
        if(counter>1) return 1;
    }
    return 0;
}


int applyDeltaMove(GameStruct * game , Jogada * jogada , CorPiece turn , CorPiece op_turn){
    uint64_bit origem_bit = 1ULL << jogada->origem;
    uint64_bit destino_bit = 1ULL << jogada->destino;
    Pieces peca_movida = (Pieces)jogada->peca_movida;
    int old_moved_eval = evaluate_piece(origem_bit, peca_movida, turn, game);

    Pieces peca_capturada = jogada->peca_capturada;
    uint64_bit captured_bit = destino_bit;
    if(jogada->especial == FLAG_ENPASSANT) captured_bit = (turn == brancas) ? (destino_bit >> 8) : (destino_bit << 8);
    int old_captured_eval = (peca_capturada != Empty) ? evaluate_piece(captured_bit, peca_capturada, op_turn, game) : 0;
    
    atualizaJogada(game, jogada, turn);
    int promote_value = (jogada->promocao) ? (pieces_value[jogada->promocao] - 100) : 0;

    int new_moved_eval = evaluate_piece(destino_bit, peca_movida, turn, game);

    int who2Move = (turn == brancas) ? 1 : -1;
    int castleBonus = (jogada->especial == FLAG_CASTLE) ? 50 : 0;
    return (who2Move * (new_moved_eval - old_moved_eval + old_captured_eval + promote_value + castleBonus));
}


int calculate_extension_depth(GameStruct * game , int depth , CorPiece op_turn){
    int extension = 0;
    if(depth < MAX_DEPTH_SEARCH + 2){
        if(is_in_check(&game->estadoJogo,game->estadoJogo.tabuleirojogo[op_turn][King],op_turn))
            extension++;
    }
    return extension;
}



