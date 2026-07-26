#include <engine.h>
#include <evals.h>

//Still havent done any of the evals , work in progress
//Evaluation functions need to do a incremental evaluation to only check the new piece moved evaluation and compare it
//Changing alpha and beta depending to that comparation


int evaluate_piece(uint64_bit piece_pos , Pieces piece_type , CorPiece turn , int ai_lvl , EstadoJogo * estado , Boolean is_middle_game){
    //piece evaluation is based on the position of the piece , its mobility , what pieces it attacks
    //and how it coordinates with other pieces
    /*  piece_eval = piece_score + position_score + mobility_score + attacks_score 
                    + same_colour_coordination - op_colour_coordination;
    */
    int position_score = 0 , mobility_score = 0 , attacks_score = 0 ,
       same_colour_coord = 0 ,  op_colour_coord = 0 , piece_score = piece_value(piece_type);
    int pos = posTabuleiro(piece_pos);
    int line = pos/8 , column = pos%8 , indx = (turn==brancas) ? ((7-line)*8 + column) : (line*8 + column);
    switch(piece_type){
        case Pawn :
            position_score = pawn_evals_black[indx];
        break;
        case Rook:
            position_score = black_rook_evals[indx];
        break;
        case Horse:
            position_score = knight_evals[line*8 + column];
        break;
        case Bishop:
            position_score = black_bishop_evals[indx];
        break;
        case Queen:
            position_score = black_queen_evals[indx];
        break;
        case King:
            if(is_middle_game){
                position_score = black_king_middleGame_evals[indx];
            }
            else position_score = black_king_endGame_evals[indx];
        break;
        default:break;
    }
    return (piece_score + position_score + mobility_score + attacks_score + same_colour_coord - op_colour_coord);
}


int evaluate_pos(GameStruct * game , SearchInfo * search , int * w_eval , int * b_eval , int pieces_evals[2][NUMBER_PIECES] , int cur_piece_eval){
    int * cur_player_eval , who2Move ;
    CorPiece other_turn , cur_turn;
    Pieces piece = search->piece_type;
    if(search->turn == brancas){
        cur_player_eval = w_eval;
        other_turn = pretas; cur_turn = brancas;
        who2Move = 1;
    }
    else{
        cur_player_eval = b_eval;
        other_turn = brancas; cur_turn = pretas;
        who2Move = (-1);
    }

    if(isCheckMate(game,other_turn)) *cur_player_eval = (search->turn == brancas) ? 99999 : (-99999);
    else{
        int new_piece_eval = evaluate_piece(game->estadoJogo.tabuleirojogo[cur_turn][piece],piece,
                                            cur_turn,search->ai_level,&game->estadoJogo,0);
        *cur_player_eval += (new_piece_eval - pieces_evals[cur_turn][piece]);
        *cur_player_eval *= who2Move;
    }
    return (*cur_player_eval);
}


int evaluate(GameStruct * game , CorPiece turno , int ai_level , int pieces_evals[2][NUMBER_PIECES] , int * white_eval , int * black_eval){
    CorPiece other_turn = pretas;
    int * turn_eval = white_eval , * other_turn_eval = black_eval;
    int eval = 0 , who2Move =  1 , whoNot2Move = (-1) ;
    if(turno == pretas){
        other_turn = brancas;
        who2Move = (-1);
        whoNot2Move = 1;
        turn_eval = black_eval;
        other_turn_eval = white_eval;
    }
    for(int i=0;i<NUMBER_PIECES;i++){
        int piece_eval_turn = evaluate_piece(game->estadoJogo.tabuleirojogo[turno][i],(Pieces)i,turno,
                                        ai_level,&game->estadoJogo,0);
        int piece_eval_other_turn = evaluate_piece(game->estadoJogo.tabuleirojogo[other_turn][i],(Pieces)i,
                                        other_turn,ai_level,&game->estadoJogo,0);

        pieces_evals[turno][i] = piece_eval_turn;
        *turn_eval+= (piece_eval_turn*who2Move);

        pieces_evals[other_turn][i] = piece_eval_other_turn;
        *other_turn_eval += (piece_eval_other_turn*whoNot2Move);

        eval+= piece_eval_turn*who2Move + piece_eval_other_turn*whoNot2Move;
    }
    return eval;
}