#include "engine/chess_lib/engine.h"
#include <stdio.h>




void promotePiece(GameStruct * game , Pieces piece, uint64_bit promotion_square , CorPiece turno){
    uint64_bit * bitboard_pawns = &(game->estadoJogo.tabuleirojogo[turno][Pawn]);
    uint64_bit * bitboard_nova_piece = &(game->estadoJogo.tabuleirojogo[turno][piece]);
    *bitboard_pawns &= ~promotion_square;
    *bitboard_nova_piece |= promotion_square;
}




void efetuaJogada(uint64_bit * selected_piece , uint64_bit * todas_pieces , uint64_bit original_coords , uint64_bit click , uint64_bit * mesmacor){
    *mesmacor = ( ( (*mesmacor) & (~original_coords) ) | click);
    *selected_piece = ( ( (*selected_piece) & (~original_coords) ) | click);
    *todas_pieces = ( ( (*todas_pieces) & (~original_coords)) | click);
}


void fetch_change_board(GameStruct * game,uint64_bit click,uint64_bit * mesmaCor , uint64_bit * corOposta, Jogada * jogada , CorPiece turno){
    CorPiece cor_oposta = (turno == brancas) ? pretas : brancas;
    Pieces selected = jogada->peca_movida;

    //sem esta verificacao ha problemas e pecas tornam se duplicadas
    Pieces piece_comida = comparePiece(&game->estadoJogo, cor_oposta, click);
    if(piece_comida == Empty || selected == Empty )return;

    jogada->peca_capturada = piece_comida;
    game->estadoJogo.tabuleirojogo[cor_oposta][piece_comida] &= ~click;

    *corOposta &= ~click;
    game->estadoJogo.bitboard_todas_pieces &= ~click;
    efetuaJogada(&(game->estadoJogo.tabuleirojogo[turno][selected]),
                 &(game->estadoJogo.bitboard_todas_pieces),1ULL<<jogada->origem,
                 click,mesmaCor
                );
}




void checkTurno(CorPiece turno , uint64_bit * * oposta , uint64_bit * * mesma_cor,int * sq , GameStruct * game , uint64_bit (**ep)(uint64_bit,int)){
    if(turno==brancas){
        *oposta = &(game->estadoJogo.bitboard_pretas);
        *mesma_cor = &(game->estadoJogo.bitboard_brancas);
        *sq=0;
        *ep = &shiftr;
    }
    else{
        *oposta = &(game->estadoJogo.bitboard_brancas);
        *mesma_cor = &(game->estadoJogo.bitboard_pretas);
        *sq=56;
        *ep = &shiftl;
    }
}


void atualizaJogada(GameStruct * game , Jogada * jogada , CorPiece turno){
    if(jogada->peca_movida == Empty) return;
    uint64_bit * bitboard_cor_oposta , * bitboard_cor_turno , (*ep)(uint64_bit,int);
    int square;
    checkTurno(turno,&bitboard_cor_oposta,&bitboard_cor_turno,&square,game,&ep);
    uint64_bit click = 1ULL<<jogada->destino;
    if(jogada->especial == FLAG_CASTLE){
        castle_King(game,click,square,bitboard_cor_turno,turno);
    }
    else if(jogada->especial == FLAG_ENPASSANT){
        enpassant_move(game,bitboard_cor_oposta,bitboard_cor_turno,ep,turno,1ULL<<jogada->origem);
    }
    else if( (click & *bitboard_cor_oposta) != 0){
        fetch_change_board(game,click,bitboard_cor_turno,bitboard_cor_oposta,jogada,turno);
    }
    else{
        uint64_bit * bit_piece = &(game->estadoJogo.tabuleirojogo[turno][jogada->peca_movida]),
                   * bit_global = &(game->estadoJogo.bitboard_todas_pieces);
        efetuaJogada(bit_piece,bit_global,1ULL<<jogada->origem,click,bitboard_cor_turno);
        if(jogada->promocao){
            promotePiece(game,Queen,click,turno);
        }
    } 
}