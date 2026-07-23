#include <engine.h>
#include <undoMove.h>
#include <stdlib.h>


Boolean is_in_check(EstadoJogo * estado , uint64_bit kingpos , CorPiece cor){
    uint64_bit todas_pieces =  estado->bitboard_todas_pieces;
    CorPiece oponente = (cor==brancas) ? pretas : brancas;
    uint64_bit op_knight = estado->tabuleirojogo[oponente][2],
               op_pawns = estado->tabuleirojogo[oponente][0],
               op_rooks = estado->tabuleirojogo[oponente][1],
               op_bishops = estado->tabuleirojogo[oponente][3],
               op_queen = estado->tabuleirojogo[oponente][4],
               op_king = estado->tabuleirojogo[oponente][5];
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
    game_aux->lastmoves = NULL;
    game_aux->pieceCoords = pos_piece;
}


int isCheckMate(GameStruct * game , CorPiece cor){
    //return 1; // Testes de checkmate , importante remover depois
    int in_check = 1;
    CorPiece cor_atual = game->turnoJogador;
    game->turnoJogador = cor;
    uint64_bit same_colour = get_same_colour_bitboard(&(game->estadoJogo),cor);
    for(int i=0;i<6 && in_check;i++){
        uint64_bit tab_piece = game->estadoJogo.tabuleirojogo[cor][i];
        Pieces piece_atual = (Pieces)i;
        while(tab_piece !=0 && in_check){
            uint64_bit pos_piece = (1ULL<<(__builtin_ctzll(tab_piece) ));
            uint64_bit pieces_move = get_piece_attacks(pos_piece,piece_atual,game,cor);
            uint64_bit tries = pieces_move & ~same_colour;
            while( tries !=0 && in_check){
                Boolean castles = 0 , enpassant = 0 , promotion = 0;
                int casa_destino = __builtin_ctzll(tries);
                uint64_bit drop = 1ULL<<casa_destino;
                if(isPseudoValidMove(game,drop,&castles,&enpassant,&promotion)){
                    atualizaJogada(game,drop,castles,enpassant,cor);
                    in_check = is_in_check(&(game->estadoJogo),(game->estadoJogo.tabuleirojogo[cor][King]),cor);
                    undoMove(game,drop,pos_piece,0,piece_atual,cor);
                }
                tries &= (tries-1);
            }
            tab_piece &= (tab_piece-1);
        }
    }
    game->turnoJogador = cor_atual;
    return (in_check);
}




int check_move(GameStruct * game, Boolean castles , uint64_bit click){
    CorPiece turno = game->turnoJogador;
    CorPiece turno_op = (turno == pretas) ? brancas : pretas;
    uint64_bit pos_king_op = game->estadoJogo.tabuleirojogo[turno_op][King];
    if(is_in_check(&(game->estadoJogo),game->estadoJogo.tabuleirojogo[turno][King],turno)){
        return 1;
    }
    else if( castles && invalidCastle(game,click)){
        return 1;
    }
    else{
        verifica_direito_castle(game,turno);
    }

    Boolean cant_move_opp_king = isCheckMate(game,turno_op);
    if(is_in_check(&(game->estadoJogo),pos_king_op,turno_op)){
        if(cant_move_opp_king){
            return 2;
        }
        game->estadoJogo.king_in_check[turno_op] = 1;
    }
    else if(cant_move_opp_king) return 3;
    return 0;
}
