#include <engine.h>


int pawnFirstRank(uint64_bit pos,CorPiece cor){
    int postab = __builtin_ctzll(pos);
    if(cor==brancas){
        return(8 <= postab && postab < 16);
    }
    else{
        return( 48 <= postab && postab < 56);
    }
}


uint64_bit get_same_colour_bitboard(EstadoJogo * estado , CorPiece cor){
    if(cor==brancas) return estado->bitboard_brancas;
    else return estado->bitboard_pretas;
}


uint64_bit get_opposing_colour_bitboard(EstadoJogo * estado , CorPiece cor){
    if(cor==brancas) return estado->bitboard_pretas;
    else return estado->bitboard_brancas;
}


Pieces comparePiece(EstadoJogo estado , CorPiece cor , uint64_bit posclique){
    int i;
    for(i=0;i<6;i++){
        if(estado.tabuleirojogo[cor][i] & posclique){
            return((Pieces)i);
        }
    }
    return Empty;
}


int posTabuleiro(uint64_bit bitboard){
    if(bitboard==0) return (-1);
    return __builtin_ctzll(bitboard);
}


void getColunasAH(uint64_bit * colunaA , uint64_bit * colunaH){
    *colunaA = 0ULL;
    *colunaH = 0ULL;
    for(int i=0;i<8;i++){
        *colunaA |= (1ULL<< (8*i));
        *colunaH |= (1ULL<< (8*i + 7));
    }
}