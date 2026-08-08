#include "engine/chess_lib/engine.h"
#include <engine/chess_lib/evals.h>


uint64_bit shiftr(uint64_bit pos,int shift){
    return (pos>>shift);
}



uint64_bit shiftl(uint64_bit pos,int shift){
    return (pos<<shift);
}


int pawnFirstRank(uint64_bit pos,CorPiece cor){
    int postab = __builtin_ctzll(pos);
    if(cor==brancas){
        return(8 <= postab && postab < 16);
    }
    else{
        return( 48 <= postab && postab < 56);
    }
}



uint64_bit initQuadrado(void){
    uint64_bit quadrado = 0;
    quadrado|= (1ULL << A8);quadrado|= (1ULL << B8);quadrado|= (1ULL << C8);quadrado|= (1ULL << D8);quadrado|= (1ULL << E8);quadrado|= (1ULL << F8);
                                        quadrado|= (1ULL << G8);quadrado|= (1ULL << H8);
    quadrado|= (1ULL << A7);                                                                                                quadrado|= (1ULL << H7); 
    quadrado|= (1ULL << A6);                                                                                                quadrado|= (1ULL << H6); 
    quadrado|= (1ULL << A5);                                                                                                quadrado|= (1ULL << H5); 
    quadrado|= (1ULL << A4);                                                                                                quadrado|= (1ULL << H4);                                  
    quadrado|= (1ULL << A3);                                                                                                quadrado|= (1ULL << H3); 
    quadrado|= (1ULL << A2);                                                                                                quadrado|= (1ULL << H2); 
    quadrado|= (1ULL << A1);quadrado|= (1ULL << B1);quadrado|= (1ULL << C1);quadrado|= (1ULL << D1);quadrado|= (1ULL << E1);quadrado|= (1ULL << F1);
                                        quadrado|= (1ULL << G1);quadrado|= (1ULL << H1);
    return quadrado;
}




uint64_bit get_same_colour_bitboard(EstadoJogo * estado , CorPiece cor){
    if(cor==brancas) return estado->bitboard_brancas;
    else return estado->bitboard_pretas;
}


uint64_bit get_opposing_colour_bitboard(EstadoJogo * estado , CorPiece cor){
    if(cor==brancas) return estado->bitboard_pretas;
    else return estado->bitboard_brancas;
}


uint64_bit get_selected_piece_attacks(GameStruct * game , uint64_bit click , Pieces piece , CorPiece turno){
    uint64_bit atk = 0;
    uint64_bit passant = game->estadoJogo.enpassant;
    Jogada j = {.origem = (-1) , .peca_movida = piece};
    if(can_en_passant(game,&j,turno)) atk = passant | get_piece_attacks(click,piece,game,turno);
    else atk = get_piece_attacks(click,piece,game,turno);
    atk = ~get_same_colour_bitboard(&game->estadoJogo,turno) & atk;
    return atk;
}



int is_attacked_piece(uint64_bit is_attacked , Pieces attacked_piece , CorPiece turn , GameStruct * game , int piece_score){
    for(int i=0;i<NUMBER_PIECES;i++){
        uint64_bit attacker = game->estadoJogo.tabuleirojogo[turn][i];
        if(attacker!=0 && pieces_value[i] < piece_score){
            uint64_bit atks = get_piece_attacks(is_attacked,attacked_piece,game,turn);
            if((atks & attacker)!=0) return 1;
        }
    }
    return 0;
}
