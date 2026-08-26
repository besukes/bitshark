#include "engine/chess_lib/engine.h"



void initPieces(uint64_bit * pt,int tipo,casas_board pos,int difPos){
    *pt = 0;
    *pt |= (1ULL << (tipo + pos));
    *pt |= (1ULL << (tipo + pos + difPos));
}


void initTabPawns(uint64_bit * pt,int tipo){
    *pt = 0;
    if(tipo==0){
        *pt |= (1ULL << A2);
        *pt |= (1ULL << B2);
        *pt |= (1ULL << C2);
        *pt |= (1ULL << D2);
        *pt |= (1ULL << E2);
        *pt |= (1ULL << F2);
        *pt |= (1ULL << G2);
        *pt |= (1ULL << H2);
    }
    else{
        *pt |= (1ULL << A7);
        *pt |= (1ULL << B7);
        *pt |= (1ULL << C7);
        *pt |= (1ULL << D7);
        *pt |= (1ULL << E7);
        *pt |= (1ULL << F7);
        *pt |= (1ULL << G7);
        *pt |= (1ULL << H7);
    }
}

void initTabuleiro(uint64_bit pt[2][NUMBER_PIECES], int additor){
    CorPiece turn = (additor == 56) ? pretas : brancas;
    initTabPawns(&pt[turn][Pawn],additor);
    initPieces(&pt[turn][Rook],additor,A1,7); //rook
    initPieces(&pt[turn][Horse],additor,B1,5); //horse
    initPieces(&pt[turn][Bishop],additor,C1,3); //bishop
    initPieces(&pt[turn][Queen],additor,D1,0); //queen
    initPieces(&pt[turn][King],additor,E1,0); //king
}

void init_other_bitboards(EstadoJogo * es){
    es->bitboard_brancas = 0;
    es->bitboard_pretas = 0;
    for(int i = 0 ; i < 6 ; i++){
        es->bitboard_brancas |= es->tabuleirojogo[0][i];
        es->bitboard_pretas  |= es->tabuleirojogo[1][i];
    }
    es->bitboard_todas_pieces = es->bitboard_brancas | es->bitboard_pretas;
}



void initTabuleiroBENCHMARK(EstadoJogo * state){
    Pieces test_piece = BENCHMARK_TESTED , non_tested_piece = BENCHMARK_NOT_TESTED;
    initPieces(&state->tabuleirojogo[brancas][test_piece],0,D1,0); //white queen/rook
    initPieces(&state->tabuleirojogo[brancas][King],0,E1,0); //white king

    state->tabuleirojogo[brancas][Pawn] = 0;
    state->tabuleirojogo[brancas][Horse] = 0;
    state->tabuleirojogo[brancas][Bishop] = 0;
    state->tabuleirojogo[brancas][non_tested_piece] = 0;

    initPieces(&state->tabuleirojogo[pretas][King],0,E8,0); //black king

    state->tabuleirojogo[pretas][Pawn] = 0;
    state->tabuleirojogo[pretas][Horse] = 0;
    state->tabuleirojogo[pretas][Bishop] = 0;
    state->tabuleirojogo[pretas][Rook] = 0;
    state->tabuleirojogo[pretas][Queen] = 0;

    state->bitboard_brancas = state->tabuleirojogo[brancas][test_piece] | state->tabuleirojogo[brancas][King];
    state->bitboard_pretas = state->tabuleirojogo[pretas][King];

    state->bitboard_todas_pieces = state->bitboard_brancas | state->bitboard_pretas;
}