#include <engine/chess_lib/engine.h>
#include <stdlib.h>
#include <string.h>

#define MATE_SCORE_THRESHOLD (VALOR_INFINITO - 1000)
 

TTEntry * transposition_table = NULL;
 
// Gerador xorshift64 simples e determinístico (não depende de rand() global,
// para não interferir com outras partes do jogo que possam usar rand()).
uint64_bit xorshift64(uint64_bit *state){
    uint64_bit x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}
 
void init_zobrist(void){
    uint64_bit seed = 0x9E3779B97F4A7C15ULL; // semente fixa, só importa ser != 0
    for(int c=0;c<2;c++)
        for(int p=0;p<NUMBER_PIECES;p++)
            for(int sq=0;sq<64;sq++)
                zobrist_pieces[c][p][sq] = xorshift64(&seed);
    for(int c=0;c<2;c++)
        for(int t=0;t<2;t++)
            zobrist_castle[c][t] = xorshift64(&seed);
    for(int sq=0;sq<64;sq++)
        zobrist_ep[sq] = xorshift64(&seed);
    zobrist_turn = xorshift64(&seed);
}
 
void tt_init(void){
    if(transposition_table == NULL){
        transposition_table = calloc(TT_SIZE, sizeof(TTEntry));
    }
    else{
        memset(transposition_table, 0, TT_SIZE * sizeof(TTEntry));
    }
}
 
// Calcula o hash do zero a partir do estado atual do tabuleiro. Não é incremental
// (não é atualizado dentro de applyDeltaMove/undoMove) de propósito: é mais seguro
// e simples de manter correto, e o custo (percorrer bitboards com XORs) é muito
// pequeno comparado ao custo de gerar jogadas e pesquisar a árvore.
uint64_bit compute_zobrist(GameStruct * game, CorPiece turn){
    uint64_bit h = 0;
    for(int cor=0;cor<2;cor++){
        for(int pieces=0;pieces<NUMBER_PIECES;pieces++){
            uint64_bit bb = game->estadoJogo.tabuleirojogo[cor][pieces];
            while(bb){
                int sq = __builtin_ctzll(bb);
                h ^= zobrist_pieces[cor][pieces][sq];
                bb &= bb - 1;
            }
        }
    }
    if(game->estadoJogo.canCastle[brancas][Short]) h ^= zobrist_castle[brancas][Short];
    if(game->estadoJogo.canCastle[brancas][Long])  h ^= zobrist_castle[brancas][Long];
    if(game->estadoJogo.canCastle[pretas][Short])  h ^= zobrist_castle[pretas][Short];
    if(game->estadoJogo.canCastle[pretas][Long])   h ^= zobrist_castle[pretas][Long];
    if(game->estadoJogo.enpassant){
        int sq = __builtin_ctzll(game->estadoJogo.enpassant);
        h ^= zobrist_ep[sq];
    }
    if(turn == pretas) h ^= zobrist_turn;
    return h;
}
 

TTEntry * tt_probe(uint64_bit key){
    TTEntry * entry = &transposition_table[key % TT_SIZE ];
    if(entry->flag != TT_EMPTY && entry->key == key) return entry;
    return NULL; // slot vazio, ou colisão de índice com outra posição (entrada substituída)
}

int score_to_tt(int score, int ply){
    if(score >= MATE_SCORE_THRESHOLD) return score + ply;
    if(score <= -MATE_SCORE_THRESHOLD) return score - ply;
    return score;
}
 
int score_from_tt(int score, int ply){
    if(score >= MATE_SCORE_THRESHOLD) return score - ply;
    if(score <= -MATE_SCORE_THRESHOLD) return score + ply;
    return score;
}
 


void tt_store(uint64_bit key, int depth, int score, TTFlag flag, Jogada best_move , int ply){
    TTEntry * entry = &transposition_table[key % TT_SIZE];
    // Política de substituição simples: só substitui se a nova entrada tem profundidade
    // igual ou maior (mais fiável), ou se o slot pertence a outra posição.
    if(entry->key != key || depth >= entry->depth){
        entry->key = key;
        entry->depth = depth;
        entry->score = score_to_tt(score,ply);
        entry->flag = flag;
        entry->best_move = best_move;
    }
}




TTEntry * getPositionTTMove(uint64_bit key , int depth , int * alpha , int * beta , int * move_eval , Jogada * * hash_move , int ply){
    TTEntry * entry = tt_probe(key);
    if(entry!=NULL){
        *hash_move = &entry->best_move;
        if(entry->depth >= depth){
            int adjusted_score = score_from_tt(entry->score, ply);
            if(entry->flag == TT_EXACT){
                *move_eval = adjusted_score;
            }
            else if(entry->flag == TT_LOWERBOUND && adjusted_score > *alpha){
                *alpha = adjusted_score;
            }
            else if(entry->flag == TT_UPPERBOUND && adjusted_score < *beta){
                *beta = adjusted_score;
            }
            if(*alpha >= *beta){
                *move_eval = adjusted_score;
            }
        }
    }
    return entry;
}



void updateZobrist(GameStruct * game , Jogada * jogada ,CorPiece turn , CorPiece op_turn){
    uint64_bit h = game->cur_pos_key;
    if(jogada->especial == FLAG_CASTLE){
        int dest_indx = jogada->destino % 8;
        int castle_indx = (dest_indx >= 4) ? Short : Long;
        int shortc = F1 , longc = D1 , before_sc = H1 , before_lc = A1;
        if(turn == pretas){
            shortc = F8;
            longc = D8;
            before_sc = H8;
            before_lc = A8;
        }
        h^=(castle_indx == Short) ? zobrist_pieces[turn][Rook][shortc] : zobrist_pieces[turn][Rook][longc];
        h^=(castle_indx == Short) ? zobrist_pieces[turn][Rook][before_sc] : zobrist_pieces[turn][Rook][before_lc];
    }
    else if(jogada->especial == FLAG_ENPASSANT){
        int sq_taken = (turn == brancas) ? (jogada->destino - 8) : (jogada->destino + 8);
        h^=zobrist_pieces[op_turn][Pawn][sq_taken];
    }
    else if(jogada->peca_capturada != Empty){
        h^=zobrist_pieces[op_turn][jogada->peca_capturada][jogada->destino];
    }

    if(jogada->promocao) h^=zobrist_pieces[turn][jogada->promocao][jogada->destino];
    else h^=zobrist_pieces[turn][jogada->peca_movida][jogada->destino];

    h^=zobrist_pieces[turn][jogada->peca_movida][jogada->origem];
    
    h^=zobrist_turn;

    if(jogada->prev_castlerights[turn][Short] != game->estadoJogo.canCastle[turn][Short])
        h^=zobrist_castle[turn][Short];

    if(jogada->prev_castlerights[turn][Long] != game->estadoJogo.canCastle[turn][Long])
        h^=zobrist_castle[turn][Long];

    if(jogada->prev_castlerights[op_turn][Short] != game->estadoJogo.canCastle[op_turn][Short])
        h^=zobrist_castle[op_turn][Short];

    if(jogada->prev_castlerights[op_turn][Long] != game->estadoJogo.canCastle[op_turn][Long])
        h^=zobrist_castle[op_turn][Long];
    
    if(jogada->prev_enpassant != 255){
        h ^= zobrist_ep[jogada->prev_enpassant];
    }
    if(game->estadoJogo.enpassant != 0){
        h ^= zobrist_ep[posTabuleiro(game->estadoJogo.enpassant)];
    }
    game->cur_pos_key = h;
}