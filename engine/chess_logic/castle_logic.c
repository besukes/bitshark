#include "engine/chess_lib/engine.h"



int is_open_castle_path(uint64_bit bitboard_todas_pieces,uint64_bit path , uint64_bit extraPositions){
    uint64_bit relevant_path = path & ~extraPositions;
    return ( (bitboard_todas_pieces & relevant_path )== 0);
}



int is_castelling_king(GameStruct * game , CorPiece cor, uint64_bit drop){
    uint64_bit destino_short = (cor == brancas) ? (1ULL << 6 | 1ULL<<7)  : (1ULL << 62 | 1ULL<<63),
               destino_long  = (cor == brancas) ? (1ULL << 2 | 1ULL <<1 | 1ULL)  : (1ULL << 58 | 1ULL << 57 | 1ULL << 56);
    uint64_bit path_short = (cor == brancas) ? (1ULL << 6 | 1ULL<<7 | 1ULL<<5)  : ( 1ULL << 61 | 1ULL << 62 | 1ULL<<63),
               path_long  = (cor == brancas) ? ( 1ULL << 3 | 1ULL << 2 | 1ULL <<1 | 1ULL)  : (1ULL << 59 | 1ULL << 58 | 1ULL << 57 | 1ULL << 56);
    uint64_bit extra_pos_short = (cor == brancas) ? ( (1ULL << 7) & game->estadoJogo.tabuleirojogo[cor][Rook] )
                                              : ( (1ULL << 63) & game->estadoJogo.tabuleirojogo[cor][Rook] ),
               extra_pos_long = (cor == brancas) ? ( 1ULL & game->estadoJogo.tabuleirojogo[cor][Rook] )
                                              : ( (1ULL << 56) & game->estadoJogo.tabuleirojogo[cor][Rook] );
    Boolean castelShort = ( (destino_short & drop) != 0) && (extra_pos_short != 0) &&
                is_open_castle_path(game->estadoJogo.bitboard_todas_pieces,path_short,extra_pos_short),
            castelLong = ( (destino_long & drop) != 0) && (extra_pos_long != 0) &&
                is_open_castle_path(game->estadoJogo.bitboard_todas_pieces,path_long,extra_pos_long);
    return (                                      !game->estadoJogo.king_in_check[cor] &&
            ( (castelShort && game->estadoJogo.canCastle[cor][Short]) || (castelLong && game->estadoJogo.canCastle[cor][Long]) ) );
}



int invalidCastle(GameStruct * game , uint8_t pos , CorPiece turno){
    int offset = 0;
    unsigned int indx = 0;
    if(pos == 64) return 0;
    casas_board casa_rook = (pos%8 > 4) ? F1 : D1;
    if(turno == pretas){
        offset = 1; indx = 7;
    }
    uint64_bit rook_castle = game->estadoJogo.tabuleirojogo[offset][Rook] & (1ULL << (casa_rook + 8*indx)),
               king_pos = game->estadoJogo.tabuleirojogo[offset][King];
    return( is_in_check(&game->estadoJogo,king_pos,turno)
            || is_in_check(&game->estadoJogo,rook_castle,turno)
            || ( (king_pos & game->estadoJogo.tabuleirojogo[offset][Rook] )!= 0)
    );
}


void verifica_direito_castle(GameStruct * game , Jogada * jogada , CorPiece turn){
    Pieces piece = jogada->peca_movida;
    uint64_bit cur_pos = jogada->origem;
    if(piece == King){
        game->estadoJogo.canCastle[turn][Short] = 0;
        game->estadoJogo.canCastle[turn][Long] = 0;
    }
    else if(piece==Rook){
        if(turn==brancas){
            if(cur_pos == (1ULL<<H1)) game->estadoJogo.canCastle[turn][Short] = 0;
            if(cur_pos == (1ULL)) game->estadoJogo.canCastle[turn][Long] = 0;
        }
        else{
            if(cur_pos == (1ULL<<H8)) game->estadoJogo.canCastle[turn][Short] = 0;
            if(cur_pos == (1ULL<<A8)) game->estadoJogo.canCastle[turn][Long] = 0;
        }
    }
}



void castle_King(GameStruct * game , uint64_bit click , int square, uint64_bit * mesmaCor , CorPiece turno){
    int pos = posTabuleiro(click) , offset = 0 , shiftam = 3;
    if(pos == (-1)) return;
    uint64_bit (*funcRook)(uint64_bit,int) = &shiftl,
               (*funcKing)(uint64_bit,int) = &shiftr;
    if(pos%8 > 4){
        offset=7;funcRook = &shiftr; shiftam = 2; funcKing = &shiftl;
    }
    uint64_bit rooks = game->estadoJogo.tabuleirojogo[turno][Rook],
               rook_de_castle = 1ULL<<(square + offset),
               rook_shifted = funcRook(rook_de_castle,shiftam);

    *mesmaCor = *mesmaCor & ~rooks & ~game->estadoJogo.tabuleirojogo[turno][King];
    game->estadoJogo.bitboard_todas_pieces &= ~rooks & ~game->estadoJogo.tabuleirojogo[turno][King];

    rooks = rook_shifted | (rooks & ~rook_de_castle);
    game->estadoJogo.tabuleirojogo[turno][Rook] = rooks;
    uint64_bit click_shifted = funcKing(rook_shifted,1);

    *mesmaCor |= (rooks | click_shifted);
    game->estadoJogo.bitboard_todas_pieces |= (rooks | click_shifted);
    game->estadoJogo.tabuleirojogo[turno][King] = click_shifted;
}