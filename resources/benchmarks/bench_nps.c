#include "engine/chess_lib/engine.h"
#include <stdio.h>
#include <string.h>


EstadoJogo empty_state(void){ 
    EstadoJogo e; 
    memset(&e,0,sizeof(EstadoJogo)); 
    return e; 
}


void place(EstadoJogo * e, CorPiece cor, Pieces piece, int square){
    e->tabuleirojogo[cor][piece] |= (1ULL << square);
    if(cor==brancas) e->bitboard_brancas |= (1ULL<<square);
    else e->bitboard_pretas |= (1ULL<<square);
    e->bitboard_todas_pieces |= (1ULL<<square);
}


GameStruct base_game(void){
    GameStruct g; memset(&g,0,sizeof(GameStruct));
    g.estadoJogo = empty_state(); g.turnoJogador = brancas;
    for(int i=0;i<2;i++){ g.estadoJogo.canCastle[i][0]=0; g.estadoJogo.canCastle[i][1]=0; }
    return g;
}

void run_benchmark(const char * label, GameStruct * g, CorPiece turn){
    printf("\n===== %s =====\n", label);
    total_nodes_searched = 0;
    double t0 = SDL_GetTicks();
    Jogada m = get_best_move(g, turn, 1);
    double elapsed_s = (SDL_GetTicks() - t0) / 1000.0;
    if(elapsed_s <= 0) elapsed_s = 0.001;
    double nps = total_nodes_searched / elapsed_s;
    printf("nodes=%lu  tempo=%.2fs  NPS=%.0f  (jogada: peca=%d %d->%d)\n",
        total_nodes_searched, elapsed_s, nps, m.peca_movida, m.origem, m.destino);
}

int main(void){
    SDL_Init(SDL_INIT_TIMER);
    init_zobrist();
    tt_init();
    init_lmrLT_table();

    // 1) Posicao inicial (abertura) -- monta o tabuleiro padrao diretamente (sem depender de assets/SDL).
    {
        GameStruct g = base_game();
        initTabuleiro(&(g.estadoJogo.tabuleirojogo[0][0]),0);
        initTabuleiro(&(g.estadoJogo.tabuleirojogo[1][0]),56);
        init_other_bitboards(&g.estadoJogo);
        for(int i=0;i<2;i++){ g.estadoJogo.canCastle[i][0]=1; g.estadoJogo.canCastle[i][1]=1; }
        run_benchmark("Abertura (posicao inicial)", &g, brancas);
    }

    // 2) Meio-jogo: uma posicao com bastante material e mobilidade dos dois lados.
    {
        GameStruct g = base_game();
        // Brancas
        place(&g.estadoJogo, brancas, King, G1);
        place(&g.estadoJogo, brancas, Queen, D1);
        place(&g.estadoJogo, brancas, Rook, A1);
        place(&g.estadoJogo, brancas, Rook, F1);
        place(&g.estadoJogo, brancas, Bishop, E3);
        place(&g.estadoJogo, brancas, Bishop, G2);
        place(&g.estadoJogo, brancas, Horse, C3);
        place(&g.estadoJogo, brancas, Horse, F3);
        place(&g.estadoJogo, brancas, Pawn, A2);
        place(&g.estadoJogo, brancas, Pawn, B2);
        place(&g.estadoJogo, brancas, Pawn, C2);
        place(&g.estadoJogo, brancas, Pawn, D4);
        place(&g.estadoJogo, brancas, Pawn, E4);
        place(&g.estadoJogo, brancas, Pawn, F2);
        place(&g.estadoJogo, brancas, Pawn, G3);
        place(&g.estadoJogo, brancas, Pawn, H2);
        // Pretas
        place(&g.estadoJogo, pretas, King, G8);
        place(&g.estadoJogo, pretas, Queen, D8);
        place(&g.estadoJogo, pretas, Rook, A8);
        place(&g.estadoJogo, pretas, Rook, F8);
        place(&g.estadoJogo, pretas, Bishop, E6);
        place(&g.estadoJogo, pretas, Bishop, G7);
        place(&g.estadoJogo, pretas, Horse, C6);
        place(&g.estadoJogo, pretas, Horse, F6);
        place(&g.estadoJogo, pretas, Pawn, A7);
        place(&g.estadoJogo, pretas, Pawn, B7);
        place(&g.estadoJogo, pretas, Pawn, C7);
        place(&g.estadoJogo, pretas, Pawn, D6);
        place(&g.estadoJogo, pretas, Pawn, E5);
        place(&g.estadoJogo, pretas, Pawn, F7);
        place(&g.estadoJogo, pretas, Pawn, G6);
        place(&g.estadoJogo, pretas, Pawn, H7);
        run_benchmark("Meio-jogo (posicao tipo Ruy Lopez fechada)", &g, brancas);
    }

    // 3) Final: K+R+P vs K+R (final classico, poucas pecas, mobilidade alta do rei)
    {
        GameStruct g = base_game();
        place(&g.estadoJogo, brancas, King, E1);
        place(&g.estadoJogo, brancas, Rook, D1);
        place(&g.estadoJogo, brancas, Pawn, A4);
        place(&g.estadoJogo, brancas, Pawn, F2);
        place(&g.estadoJogo, brancas, Pawn, G2);
        place(&g.estadoJogo, brancas, Pawn, H2);
        place(&g.estadoJogo, pretas, King, E8);
        place(&g.estadoJogo, pretas, Rook, D8);
        place(&g.estadoJogo, pretas, Pawn, F7);
        place(&g.estadoJogo, pretas, Pawn, G7);
        place(&g.estadoJogo, pretas, Pawn, H7);
        run_benchmark("Final (Torre + peoes)", &g, brancas);
    }

    return 0;
}
