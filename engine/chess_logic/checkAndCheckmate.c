#include "engine/chess_lib/engine.h"
#include <stdio.h>



Boolean is_in_check(EstadoJogo * estado , uint64_bit kingpos , CorPiece cor){
    uint64_bit todas_pieces =  estado->bitboard_todas_pieces;
    CorPiece oponente = (cor==brancas) ? pretas : brancas;
    uint64_bit op_knight = estado->tabuleirojogo[oponente][Horse],
               op_pawns = estado->tabuleirojogo[oponente][Pawn],
               op_rooks = estado->tabuleirojogo[oponente][Rook],
               op_bishops = estado->tabuleirojogo[oponente][Bishop],
               op_queen = estado->tabuleirojogo[oponente][Queen],
               op_king = estado->tabuleirojogo[oponente][King];
    uint64_bit check_knights = get_knight_attacks(kingpos) & op_knight,
               check_pawns = get_pawn_attacks(kingpos,cor) & op_pawns,
               check_diagonals = get_sliding_attacks(kingpos,todas_pieces) & (op_bishops | op_queen),
               check_cross = get_cross_attacks(kingpos,todas_pieces) & (op_rooks | op_queen),
               check_king = get_king_moves(kingpos) & op_king;
    return ( (check_knights | check_pawns | check_diagonals | check_cross | check_king) != 0);
}



void notInCheck(GameStruct * game){
    CorPiece turno = game->turnoJogador;
    game->estadoJogo.king_in_check[turno] = 0;
}


void initgame_aux(GameStruct * game_aux , CorPiece cor_pieces_teste , Pieces piece_atual ,uint64_bit pos_piece){
    game_aux->turnoJogador = cor_pieces_teste;
    game_aux->pieceSelecionada = piece_atual;
    game_aux->indx_lastmoves = 0;
    game_aux->pieceCoords = pos_piece;
}


int isCheckMate(GameStruct * game , CorPiece cor){
    //return 1; // Testes de checkmate , importante remover depois
    Jogada j = {.peca_movida = Empty , .origem = 0 , .destino =  0, .promocao = 0 , .especial = 0};
    int in_check = 1;
    CorPiece cor_atual = game->turnoJogador , op_colour = (cor==brancas) ? pretas : brancas;
    game->turnoJogador = cor;
    uint64_bit same_colour = get_same_colour_bitboard(&(game->estadoJogo),cor);
    for(int i=0;i<6 && in_check;i++){
        Pieces piece_atual = (Pieces)i;
        uint64_bit tab_piece = game->estadoJogo.tabuleirojogo[cor][piece_atual];
        j.peca_movida = piece_atual;
        while(tab_piece !=0 && in_check){
            uint64_bit pos_piece = (1ULL<<(__builtin_ctzll(tab_piece) ));
            uint64_bit pieces_move = get_piece_attacks(pos_piece,piece_atual,game,cor,0);
            uint64_bit tries = pieces_move & ~same_colour;
            j.origem = __builtin_ctzll(pos_piece);
            while( tries !=0 && in_check){
                j.destino = __builtin_ctzll(tries);
                if(isPseudoValidMove(game,&j,cor,pieces_move)){
                    j.peca_capturada = comparePiece(&game->estadoJogo,op_colour,1ULL<<j.destino);
                    atualizaJogada(game,&j,cor);
                    in_check = is_in_check(&(game->estadoJogo),(game->estadoJogo.tabuleirojogo[cor][King]),cor);
                    undoMove(game,&j,cor);
                }
                tries &= (tries-1);
            }
            tab_piece &= (tab_piece-1);
        }
    }
    game->turnoJogador = cor_atual;
    return (in_check);
}




TipoJogada check_move(GameStruct * game, Jogada * jogada , CorPiece turno){
    CorPiece turno_op = (turno == pretas) ? brancas : pretas;
    uint64_bit pos_king_op = game->estadoJogo.tabuleirojogo[turno_op][King];
    if(is_in_check(&(game->estadoJogo),game->estadoJogo.tabuleirojogo[turno][King],turno)){
        return Invalid;
    }
    else if( jogada->especial == FLAG_CASTLE && invalidCastle(game,jogada->destino,turno)){
        return Invalid;
    }
    else{
        verifica_direito_castle(game,jogada,turno);
    }

    GameStruct game_aux = *game;
    game_aux.indx_lastmoves = 0;
    Boolean cant_move_opp_king = isCheckMate(&game_aux,turno_op);
    if(is_in_check(&(game->estadoJogo),pos_king_op,turno_op)){
        if(cant_move_opp_king){
            return Checkmate;
        }
        game->estadoJogo.king_in_check[turno_op] = 1;
    }
    else if(cant_move_opp_king) return Stalemate;
    return Valid;
}