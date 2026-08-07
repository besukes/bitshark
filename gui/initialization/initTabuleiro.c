#include "engine/chess_lib/engine.h"



void initPieces(uint64_bit pt[6],int index,int tipo,casas_board pos,int difPos){
    pt[index] = 0;
    pt[index] |= (1ULL << (tipo + pos));
    pt[index] |= (1ULL << (tipo + pos + difPos));
}


void initTabPawns(uint64_bit * pt,int tipo){
    pt[Pawn] = 0;
    if(tipo==0){
        pt[Pawn] |= (1ULL << A2);
        pt[Pawn] |= (1ULL << B2);
        pt[Pawn] |= (1ULL << C2);
        pt[Pawn] |= (1ULL << D2);
        pt[Pawn] |= (1ULL << E2);
        pt[Pawn] |= (1ULL << F2);
        pt[Pawn] |= (1ULL << G2);
        pt[Pawn] |= (1ULL << H2);
    }
    else{
        pt[Pawn] |= (1ULL << A7);
        pt[Pawn] |= (1ULL << B7);
        pt[Pawn] |= (1ULL << C7);
        pt[Pawn] |= (1ULL << D7);
        pt[Pawn] |= (1ULL << E7);
        pt[Pawn] |= (1ULL << F7);
        pt[Pawn] |= (1ULL << G7);
        pt[Pawn] |= (1ULL << H7);
    }
}

void initTabuleiro(uint64_bit pt[6], int additor){
    initTabPawns(pt,additor);
    initPieces(pt,Rook,additor,A1,7); //rook
    initPieces(pt,Horse,additor,B1,5); //horse
    initPieces(pt,Bishop,additor,C1,3); //bishop
    initPieces(pt,Queen,additor,D1,0); //queen
    initPieces(pt,King,additor,E1,0); //king
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