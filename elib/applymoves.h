#include <engine.h>


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




void fetch_change_board(GameStruct * game,uint64_bit click,uint64_bit * mesmaCor , uint64_bit * corOposta , MoveInfo * mov){
    CorPiece turno = mov->turn;
    CorPiece cor_oposta = (turno == brancas) ? pretas : brancas;
    Pieces selected = mov->piece_moved;

    Pieces piece_comida = comparePiece(game->estadoJogo, cor_oposta, click);
    if(piece_comida == Empty)return;

    game->estadoJogo.tabuleirojogo[cor_oposta][piece_comida] &= ~click;
    *corOposta &= ~click;
    game->estadoJogo.bitboard_todas_pieces &= ~click;
    efetuaJogada(&(game->estadoJogo.tabuleirojogo[turno][selected]),
                 &(game->estadoJogo.bitboard_todas_pieces),mov->last_piece_pos,
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


void atualizaJogada(GameStruct * game , uint64_bit click,Boolean castles,Boolean enpassant , MoveInfo * mov){
    CorPiece turno = mov->turn;
    uint64_bit * bitboard_cor_oposta , * bitboard_cor_turno , (*ep)(uint64_bit,int);
    int square;
    checkTurno(turno,&bitboard_cor_oposta,&bitboard_cor_turno,&square,game,&ep);
    if(castles){
        castle_King(game,click,square,bitboard_cor_turno,turno);
    }
    else if(enpassant){
        enpassant_move(game,bitboard_cor_oposta,bitboard_cor_turno,ep);
    }
    else if( (*bitboard_cor_oposta) & click){
        fetch_change_board(game,click,bitboard_cor_turno,bitboard_cor_oposta,mov);
    }
    else{
        uint64_bit * bit_piece = &(game->estadoJogo.tabuleirojogo[turno][mov->piece_moved]),
                   * bit_global = &(game->estadoJogo.bitboard_todas_pieces);
        efetuaJogada(bit_piece,bit_global,mov->piece_moved,click,bitboard_cor_turno);
    } 
}

