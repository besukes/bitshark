#include "engine/chess_lib/engine.h"


void undoPieceComida(GameStruct * game , uint64_bit * bb_cor_piece_comida, uint64_bit click,CorPiece op_turn,Pieces capturada){
    game->estadoJogo.tabuleirojogo[op_turn][capturada] |= click;
    *bb_cor_piece_comida |= click;
    game->estadoJogo.bitboard_todas_pieces |= click;
}


void restauraCastle(uint64_bit * rei_tab,uint64_bit * mesma_cor, uint64_bit click , GameStruct * game , CorPiece turno){
    int offset = 0;
    if(turno==pretas) offset = 7;
    uint64_bit nova_rook , antiga_pos_rook;
    //Short castle
    if(posTabuleiro(click)%8 > 4){
        nova_rook = (1ULL<< (8*offset + F1));
        antiga_pos_rook = (1ULL<<(8*offset + H1));
    }
    //Long castle
    else{
        nova_rook = (1ULL<< (8*offset + D1));
        antiga_pos_rook = (1ULL<<(8*offset + A1));
    }
    uint64_bit rei_atual = *rei_tab;
    *rei_tab = (1ULL<<(8*offset + E1));
    *mesma_cor = (*mesma_cor & ~nova_rook & ~rei_atual) | antiga_pos_rook | *rei_tab;
    game->estadoJogo.tabuleirojogo[turno][Rook] &= ~nova_rook;
    game->estadoJogo.tabuleirojogo[turno][Rook] |= antiga_pos_rook;

    uint64_bit cor_oposta = (turno == brancas) ? game->estadoJogo.bitboard_pretas : game->estadoJogo.bitboard_brancas;
    game->estadoJogo.bitboard_todas_pieces = *mesma_cor | cor_oposta;
}


void undoPiece_move(GameStruct * game , uint64_bit * sameCor,uint64_bit * opCor, uint8_t new_pos,uint8_t ant_pos ,uint8_t special ,Pieces piece , CorPiece turn){
    //ant_pos e piece tem de ser mudada no jogo original
    uint64_bit * piece_tab = &(game->estadoJogo.tabuleirojogo[turn][piece]),
               antiga_pos = (1ULL<<ant_pos),
               nova_pos = (1ULL<<new_pos);
    if(special == FLAG_CASTLE){
        restauraCastle(piece_tab,sameCor,nova_pos,game,turn);
    }
    else{
        *piece_tab = ((*piece_tab & ~nova_pos) | antiga_pos);
        *sameCor = ((*sameCor & ~nova_pos) | antiga_pos);
    }
    game->estadoJogo.bitboard_todas_pieces = *sameCor | *opCor;
}



void undoMove(GameStruct * game , Jogada * jogada , CorPiece turn){
    uint64_bit * mesma_cor = &(game->estadoJogo.bitboard_brancas), 
               * cor_oposta = &(game->estadoJogo.bitboard_pretas);
    CorPiece op_turn = pretas;
    if(turn == pretas){
        mesma_cor = &(game->estadoJogo.bitboard_pretas);
        cor_oposta = &(game->estadoJogo.bitboard_brancas);
        op_turn = brancas;
    }
    updateZobrist(game,jogada,turn,op_turn);
    if(jogada->promocao){
        game->estadoJogo.tabuleirojogo[turn][jogada->promocao] &= ~(1ULL<<(jogada->destino));
    }
    undoPiece_move(game,mesma_cor,cor_oposta,jogada->destino,jogada->origem,jogada->especial
                    ,(Pieces)(jogada->peca_movida),turn);
    if(jogada->peca_capturada != Empty){
         uint64_bit captured_pos = (jogada->especial == FLAG_ENPASSANT)
            ? ((turn==brancas) ? (1ULL<<jogada->destino)>>8 : (1ULL<<jogada->destino)<<8)
            : (1ULL<<jogada->destino);
        undoPieceComida(game,cor_oposta,captured_pos,op_turn,jogada->peca_capturada);
    }
    int pos_passant = jogada->prev_enpassant;
    game->estadoJogo.canCastle[brancas][Short] = jogada->prev_castlerights[brancas][Short];
    game->estadoJogo.canCastle[brancas][Long] = jogada->prev_castlerights[brancas][Long];
    game->estadoJogo.canCastle[pretas][Short] = jogada->prev_castlerights[pretas][Short];
    game->estadoJogo.canCastle[pretas][Long] = jogada->prev_castlerights[pretas][Long];
    game->estadoJogo.enpassant = (pos_passant != 255) ? (1ULL<<pos_passant) : 0;
    game->is_end_game = is_end_game(&game->estadoJogo);
    if(jogada->especial == FLAG_CASTLE) game->estadoJogo.is_castled[turn] = 0;
}
