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


int atks_stronger_piece(uint64_bit pos , Pieces type , CorPiece turn , CorPiece op_turn , GameStruct * game , int piece_score){
    uint64_bit piece_atks = get_piece_attacks(pos,type,game,turn,1);
    for(int i=0;i<NUMBER_PIECES;i++){
        uint64_bit tab = game->estadoJogo.tabuleirojogo[op_turn][i];
        int p_value = pieces_value[i];
        if(p_value > piece_score && (tab&piece_atks) !=0) return p_value; 
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

int get_mobility_score_piece(Pieces type , uint64_bit pos , CorPiece turn , GameStruct * game , int piece_score){
    int m_score = 0;
    CorPiece op_turn = (turn==brancas) ? pretas : brancas;
    int is_attacked = is_attacked_piece(pos,type,op_turn,game,piece_score);
    int is_defended = is_defended_piece(pos,type,turn,game,piece_score);
    if(is_attacked && !is_defended){
        m_score-=piece_score;
    }
    else{
        int can_take_greater_score = atks_stronger_piece(pos,type,turn,op_turn,game,piece_score);
        if(can_take_greater_score && is_defended){
            m_score += (4*can_take_greater_score)/3;
        }
    }
    return (m_score);
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


/*int evaluate_pos(GameStruct * game ){
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

    if(0){}
    //if(isCheckMate(game,other_turn)) *cur_player_eval = 99999*who2Move;
    else{
        int new_piece_eval = evaluate_piece_type(game->estadoJogo.tabuleirojogo[cur_turn][piece],piece,
                                                cur_turn,game,search->ai_level);
        *cur_player_eval += (new_piece_eval - pieces_evals[cur_turn][piece])*who2Move;
    }
    return (*cur_player_eval);
}*/


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