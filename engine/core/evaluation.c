#include "engine/chess_lib/engine.h"
#include <engine/chess_lib/evals.h>

//Evals are now looking better but still a bit shy to a strong engine
//Evaluation functions need to do a incremental evaluation to only check the new piece moved evaluation and compare it
//Changing alpha and beta depending to that comparation

int is_end_game(EstadoJogo * estado){
    int white_queens = __builtin_popcountll(estado->tabuleirojogo[brancas][Queen]);
    int black_queens = __builtin_popcountll(estado->tabuleirojogo[pretas][Queen]);

    if(white_queens == 0 && black_queens == 0) return 1;

    int white_minors = __builtin_popcountll(estado->tabuleirojogo[brancas][Horse])
                      + __builtin_popcountll(estado->tabuleirojogo[brancas][Bishop]);
    int black_minors = __builtin_popcountll(estado->tabuleirojogo[pretas][Horse])
                      + __builtin_popcountll(estado->tabuleirojogo[pretas][Bishop]);

    Boolean white_ok = (white_queens == 0) || (white_minors <= 1);
    Boolean black_ok = (black_queens == 0) || (black_minors <= 1);

    return (white_ok && black_ok);
}


int mopup_eval(GameStruct * game , CorPiece op_turn){
    if(is_end_game(&game->estadoJogo)){
        int king_pos = posTabuleiro(game->estadoJogo.tabuleirojogo[op_turn][King]);
        int king_dist_to_center = abs(28 - king_pos);
        int force_king_to_corner_endgame = 4*king_dist_to_center;
        return force_king_to_corner_endgame;
    }
    return 0;
}



int is_defended_piece(uint64_bit pos , Pieces type , CorPiece turn , GameStruct * game , int piece_score){
    for(int i=0;i<NUMBER_PIECES;i++){
        uint64_bit tab = game->estadoJogo.tabuleirojogo[turn][i];
        while(tab){
            uint64_bit single = tab & (-tab); // isola o bit mais baixo
            uint64_bit atks = get_piece_attacks(single,(Pieces)i,game,turn,1);
            if((atks&pos) != 0) return 1;
            tab &= tab - 1; // remove esse bit
        }
    }
    return 0;
}




int evaluate_piece(uint64_bit piece_pos , Pieces piece_type , CorPiece turn , GameStruct * game){
    //piece evaluation is based on the position of the piece , its mobility , what pieces it attacks
    //and how it coordinates with other pieces
    /*  
        piece_eval = piece_score + position_score + mobility_score;
    */
    int position_score = 0 , piece_score = pieces_value[piece_type];
    //int mobility_score = 0;
    int pos = posTabuleiro(piece_pos);
    if(pos==(-1)) return 0;
    int line = pos/8 , column = pos%8 , indx = (turn==brancas) ? ((7-line)*8 + column) : pos;
    switch(piece_type){
        case Pawn :
            position_score = pawn_evals_black[indx];
        break;
        case Rook:
            position_score = black_rook_evals[indx];
        break;
        case Horse:
            position_score = knight_evals[indx];
        break;
        case Bishop:
            position_score = black_bishop_evals[indx];
        break;
        case Queen:
            position_score = black_queen_evals[indx];
        break;
        case King:
            if(is_end_game(&game->estadoJogo)){
                position_score = black_king_endGame_evals[indx];
            }
            else position_score = black_king_middleGame_evals[indx];
        break;
        default:break;
    }
    ///mobility_score = get_mobility_score_piece(piece_type,piece_pos,turn,game,piece_score);
    return (piece_score + position_score);
}



int evaluate_piece_type(uint64_bit bitboard , Pieces piece_type, CorPiece turn, GameStruct * game){
    int score = 0;
    uint64_bit bb = bitboard;
    while(bb){
        uint64_bit single = bb & (-bb); // isola o bit mais baixo
        score += evaluate_piece(single, piece_type, turn, game); // avalia só essa peça
        bb &= bb - 1; // remove esse bit
    }
    return score;
}




int evaluate(GameStruct * game , CorPiece turno){
    int total_white_eval = 0 , total_black_eval = 0;
    int eval = 0;
    for(int i=0;i<NUMBER_PIECES;i++){
        Pieces piece = (Pieces)i;
        int white_eval = evaluate_piece_type(game->estadoJogo.tabuleirojogo[brancas][piece],piece,brancas,game);
        int black_eval = evaluate_piece_type(game->estadoJogo.tabuleirojogo[pretas][piece],piece,pretas,game);

        total_white_eval+= white_eval;

        total_black_eval += black_eval;
 
    }
    eval+= total_white_eval - total_black_eval;
    eval = (turno==brancas) ? eval : (-eval);
    return eval;
}