#include "engine/chess_lib/engine.h"




void update_en_passant(GameStruct * game , Jogada * jogada , CorPiece turno){
    if(jogada->peca_movida != Pawn) return;
    uint64_bit coords = 1ULL<<jogada->origem;
    uint64_bit click = 1ULL<<jogada->destino;
    if(pawnFirstRank(coords,turno)){
        if(turno==brancas && (click == (coords << 16))) game->estadoJogo.enpassant = (coords << 8);
        else if(turno==pretas && (click == (coords >> 16))) game->estadoJogo.enpassant = (coords >> 8);
    }
    else game->estadoJogo.enpassant = 0;
}



Boolean can_en_passant(GameStruct * game , Jogada * j, CorPiece turn){
    Pieces piece = j->peca_movida;
    if(piece != Pawn || game->estadoJogo.enpassant == 0 || j->destino >= 64 || j->origem >= 64) return 0;
    uint64_bit drop = 1ULL<<j->destino;
    uint64_bit cur_pos = 1ULL<<j->origem;
    uint64_bit passant = game->estadoJogo.enpassant;
    int pos_tab_drop = posTabuleiro(drop) , pos_tab_piece = posTabuleiro(cur_pos);
    int pos_passant = posTabuleiro(passant) , pos1 = 0 , pos2 = 0;
    if(turn==brancas){
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


void enpassant_move(GameStruct * game , uint64_bit * cor_oposta , uint64_bit * mesma_cor,ShiftFunction ep_shift, CorPiece turno, uint64_bit origem){
    uint64_bit peao_removido = ep_shift(game->estadoJogo.enpassant,8);
    (*cor_oposta) &= ~peao_removido;
    (*mesma_cor) = ((*mesma_cor & ~origem) | game->estadoJogo.enpassant);
    game->estadoJogo.bitboard_todas_pieces = *mesma_cor | *cor_oposta;

    CorPiece op = (turno==brancas) ? pretas : brancas;
    uint64_bit peoes_turno = game->estadoJogo.tabuleirojogo[turno][Pawn],
               peoes_opostos = game->estadoJogo.tabuleirojogo[op][Pawn];
    peoes_turno = (peoes_turno & ~origem) | game->estadoJogo.enpassant;
    peoes_opostos &= ~peao_removido;

    game->estadoJogo.tabuleirojogo[turno][Pawn] = peoes_turno;
    game->estadoJogo.tabuleirojogo[op][Pawn] = peoes_opostos;
    game->estadoJogo.enpassant = 0;
}

