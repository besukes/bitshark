#include <engine.h>



void update_en_passant(GameStruct * game , uint64_bit click){
    CorPiece turno = game->turnoJogador;
    if(pawnFirstRank(game->pieceCoords,turno) && game->pieceSelecionada == Pawn){
        if(turno==brancas && (click == (game->pieceCoords << 16))) game->estadoJogo.enpassant = (game->pieceCoords << 8);
        else if(turno==pretas && (click == (game->pieceCoords >> 16))) game->estadoJogo.enpassant = (game->pieceCoords >> 8);
    }
    else game->estadoJogo.enpassant = 0;
}



Boolean can_en_passant(GameStruct * game , uint64_bit drop,CorPiece cor){
    if(game->pieceSelecionada != Pawn || game->estadoJogo.enpassant == 0) return 0;
    uint64_bit passant = game->estadoJogo.enpassant;
    int pos_tab_drop = posTabuleiro(drop) , pos_tab_piece = posTabuleiro(game->pieceCoords);
    int pos_passant = posTabuleiro(passant) , pos1 = 0 , pos2 = 0;
    if(cor==brancas){
        pos1 = pos_passant - 7;
        pos2 = pos_passant - 9;
        Boolean is_6_line = 40<=pos_tab_drop && pos_tab_drop < 48 ,
               pawn_in_pos = pos_tab_piece == pos1 || pos_tab_piece == pos2;
        return (is_6_line && pawn_in_pos && ((game->estadoJogo.enpassant & drop) != 0));
    }
    else{
        pos1 = pos_passant + 7;
        pos2 = pos_passant + 9;
        Boolean is_3_line = 16<=pos_tab_drop && pos_tab_drop < 24,
                pawn_in_pos = pos_tab_piece == pos1 || pos_tab_piece == pos2;
        return (is_3_line && pawn_in_pos && ((game->estadoJogo.enpassant & drop) != 0));
    }
}


void enpassant_move(GameStruct * game , uint64_bit * cor_oposta , uint64_bit * mesma_cor,ShiftFunction ep_shift){
    uint64_bit peao_removido = ep_shift(game->estadoJogo.enpassant,8);
    (*cor_oposta) &= ~peao_removido;
    (*mesma_cor) = ((*mesma_cor & ~game->pieceCoords) | game->estadoJogo.enpassant);
    game->estadoJogo.bitboard_todas_pieces = *mesma_cor | *cor_oposta;

    CorPiece turno = game->turnoJogador , op = (turno==brancas) ? pretas : brancas;
    uint64_bit peoes_turno = game->estadoJogo.tabuleirojogo[turno][0],
               peoes_opostos = game->estadoJogo.tabuleirojogo[op][0];
    peoes_turno = (peoes_turno & ~game->pieceCoords) | game->estadoJogo.enpassant;
    peoes_opostos &= ~peao_removido;

    game->estadoJogo.tabuleirojogo[turno][0] = peoes_turno;
    game->estadoJogo.tabuleirojogo[op][0] = peoes_opostos;
    game->estadoJogo.enpassant = 0;
}

