#include <engine/chess_lib/engine.h>

#include <stdlib.h>
#include <string.h>
 
// Chaves de Zobrist: um número aleatório fixo por (cor, tipo de peça, casa),
// mais chaves para direitos de castle, casa de en passant e de quem joga.
// O hash de uma posição é o XOR de todas as chaves que "estão ativas" nela.
static uint64_bit zobrist_pieces[2][NUMBER_PIECES][64];
static uint64_bit zobrist_castle[2][2];
static uint64_bit zobrist_ep[64];
static uint64_bit zobrist_turn;
 
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
 

void tt_store(uint64_bit key, int depth, int score, TTFlag flag, Jogada best_move){
    TTEntry * entry = &transposition_table[key % TT_SIZE];
    // Política de substituição simples: só substitui se a nova entrada tem profundidade
    // igual ou maior (mais fiável), ou se o slot pertence a outra posição.
    if(entry->key != key || depth >= entry->depth){
        entry->key = key;
        entry->depth = depth;
        entry->score = score;
        entry->flag = flag;
        entry->best_move = best_move;
    }
}


void getPositionTTMove(uint64_bit key , int depth , int * alpha , int * beta , int * move_eval , Jogada * * hash_move){
    TTEntry * entry = tt_probe(key);
    if(entry!=NULL){
        *hash_move = &entry->best_move;
        if(entry->depth >= depth){
            if(entry->flag == TT_EXACT){
                *move_eval = entry->score;
            }
            else if(entry->flag == TT_LOWERBOUND && entry->score > *alpha){
                *alpha = entry->score;
            }
            else if(entry->flag == TT_UPPERBOUND && entry->score < *beta){
                *beta = entry->score;
            }
            if(alpha >= beta){
                *move_eval = entry->score;
            }
        }
    }
}