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
    int white_rooks = __builtin_popcountll(estado->tabuleirojogo[brancas][Rook]);
    int black_rooks = __builtin_popcountll(estado->tabuleirojogo[pretas][Rook]);

    Boolean white_ok = (white_queens == 0 && white_minors <= 1) || (white_minors <= 1 && white_rooks == 0);
    Boolean black_ok = (black_queens == 0 && black_minors <= 1) || (black_minors <= 1 && black_rooks == 0);

    return (white_ok && black_ok);
}


int mopup_eval(GameStruct * game , CorPiece op_turn){
    if(is_end_game(&game->estadoJogo)){
        CorPiece turn = (op_turn == brancas) ? pretas : brancas;
        int turn_king_pos = posTabuleiro(game->estadoJogo.tabuleirojogo[turn][King]),
            op_turn_king_pos = posTabuleiro(game->estadoJogo.tabuleirojogo[op_turn][King]);
        if(op_turn_king_pos < 0 || turn_king_pos < 0) return 0; // segurança: sem rei (não deve acontecer)

        // King Manhattan Distance (linha/coluna) ao centro do tabuleiro
        int op_rank = op_turn_king_pos / 8, op_file = op_turn_king_pos % 8;
        int dist_rank = (op_rank <= 3) ? (3 - op_rank) : (op_rank - 4);
        int dist_file = (op_file <= 3) ? (3 - op_file) : (op_file - 4);
        int op_king_manhattan_dist_to_center = dist_rank + dist_file;

        // Distância entre os dois reis: sem aproximar o próprio rei do rei adversário, a torre
        // sozinha não consegue fechar o mate (precisa do apoio do rei para cortar as casas de
        // fuga) — sem este termo, o motor "empurra" o rei adversário para a borda mas nunca
        // convergir para o fechar, ficando a oscilar indefinidamente perto do mate sem o concluir.
        int my_rank = turn_king_pos/8, my_file = turn_king_pos%8;
        int dr = (my_rank > op_rank) ? (my_rank - op_rank) : (op_rank - my_rank);
        int df = (my_file > op_file) ? (my_file - op_file) : (op_file - my_file);
        int kings_chebyshev = (dr > df) ? dr : df;
 
        return 65*op_king_manhattan_dist_to_center + 25*(7 - kings_chebyshev);
    }
    return 0;
}



int is_defended_piece(uint64_bit pos , Pieces type , CorPiece turn , GameStruct * game , int piece_score){
    for(int i=0;i<NUMBER_PIECES;i++){
        uint64_bit tab = game->estadoJogo.tabuleirojogo[turn][i];
        while(tab){
            uint64_bit single = tab & (-tab); // isola o bit mais baixo
            uint64_bit atks = get_magic_piece_attacks(single,(Pieces)i,game,turn,1,game->estadoJogo.bitboard_todas_pieces);
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
            if(is_end_game(&game->estadoJogo)){
                position_score = pawn_evals_black_endgame[indx];
            }
            else position_score = pawn_evals_black[indx];
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


/*  Basically casts an ray from position single to target , according to the piece type , while also having in mind occupied squares , all in order to 
check whether a piece can take that square , in a way cheaper than casting all the possible rays for the piece.
    Whether this currently works or not is not totally understood by me yet , but tests are being ran.*/
uint64_bit get_SEE_ray(uint64_bit single , Pieces piece , uint64_bit occupied , CorPiece turn , uint64_bit target){
    uint64_bit piece_attks = 0;
    int target_indx = posTabuleiro(target),
        single_indx = posTabuleiro(single);
    
    int ln_target = target_indx / 8 , col_target = target_indx % 8,
        ln_single = single_indx / 8 , col_single = single_indx % 8;
    int maxDist = 0,
        shift = 0;
    uint64_bit (*shifter)(uint64_bit,int) = &shiftl;

    if(piece == Pawn){
        return get_pawn_attacks(single,turn);
    }
    else if(piece == Horse){
        return get_knight_attacks(single);
    }
    else{ //We now check what ray are we actually casting for improved efficiency
        //Points North
        if(ln_target > ln_single){
            //Points NorthEast
            if(col_target > col_single){shift = 9;maxDist = minimum(7-ln_single,7-col_single);}
            //Points North
            else if(col_target == col_single){shift = 8;maxDist = 7-ln_single;}
            //Points NorthWest
            else{shift = 7;maxDist = minimum(7-ln_single,col_single);}
        }
        //Points either West or East
        else if(ln_target == ln_single){
            shift = 1;
            //Points East
            if(col_target > col_single) maxDist = 7-col_single;
            //Invalid pointing
            else if(col_target == col_single) return 0;
            //Points West
            else{maxDist = col_single;shifter = &shiftr;}
        }
        //Points South
        else{
            shifter = &shiftr;
            //Points SouthEast
            if(col_target > col_single){shift = 7;maxDist = minimum(ln_single,7-col_single);}
            //Points South
            else if(col_target == col_single){shift = 8;maxDist = ln_single;}
            //Points SouthWest
            else{shift = 9;maxDist = minimum(ln_single,col_single);}
        }
    //If shift doesnt match rook or bishop movements , we dont even need to cast an ray , it wont reach target
        if(piece == Bishop){
           if(shift == 7 || shift == 9) get_attacks(maxDist,shifter,occupied,single,shift,&piece_attks);
        }
        if(piece == Rook){
            if(shift == 8 || shift == 1) get_attacks(maxDist,shifter,occupied,single,shift,&piece_attks);
        }
        if(piece == Queen){
            get_attacks(maxDist,shifter,occupied,single,shift,&piece_attks);
        }
    }
    return piece_attks;
}


/* Verifies whether the king can take a certain square without being capture by another piece.
    This Function needs to be fixed!! */
int is_king_last_capture(EstadoJogo * estado , CorPiece cur_turn , uint64_bit pos_cap){
    uint64_bit king_pos = estado->tabuleirojogo[cur_turn][King];
    uint64_bit king_moves = get_king_moves(king_pos);
    if(king_moves & pos_cap){
        int op_turn = (cur_turn == brancas) ? pretas : brancas;
        int hipotetical_king_indx = posTabuleiro(pos_cap),
            op_turn_king_indx = posTabuleiro(estado->tabuleirojogo[op_turn][King]);
        int x1 = hipotetical_king_indx % 8 , x2 = op_turn_king_indx % 8 ,
            y1 = hipotetical_king_indx / 8 , y2 = op_turn_king_indx / 8;
        int dx = (x1-x2) , dy = (y1-y2);
        dx = (dx<0) ? -dx : dx;
        dy = (dy<0) ? -dy : dy;
        return (!(dx <= 1 && dy <= 1));
    }
    return 0;
}


/*  This function works by checking 1 by 1 every piece from Pawn to Queen whether they can or not take a certain square and whether that may or may not
be a good trade.
    This Works by calculating the relative pieces value throughout a series of madeup captures to check how favorable is this sequence for *TURN*.
    We then check from indx to 1 the possible captures and how favorable are them.*/
int static_exchange_eval(GameStruct * game , Jogada * jogada , CorPiece turn){
    CorPiece cur_turn = (turn==brancas) ? pretas : brancas;
    uint64_bit pos_cap = 1ULL<<jogada->destino;
    uint64_bit occupied = game->estadoJogo.bitboard_todas_pieces & ~(pos_cap | (1ULL<<jogada->origem));
    int see[32]; see[0] = pieces_value[jogada->peca_capturada];
    uint64_bit pieces_checked[2][NUMBER_PIECES] = {0};
    int indx = 0;
    int current_piece_value = pieces_value[jogada->peca_movida];
    int king_has_taken = 0 , end_see = 0; // For tests it could be turned to 1

    while(!end_see){
        uint64_bit attacker_bit = 0;
        int attacker_type = 0;
        for(int i=1;i<NUMBER_PIECES;i++){
            uint64_bit positions_piece = game->estadoJogo.tabuleirojogo[cur_turn][i] & (~pieces_checked[cur_turn][i]);
            while(positions_piece != 0){
                uint64_bit single_pos = positions_piece & (-positions_piece);
                if(get_magic_piece_attacks(single_pos,(Pieces)i,game,cur_turn,0,occupied) & pos_cap){
                    attacker_type = i;
                    attacker_bit = single_pos;
                    break;
                }
                positions_piece &= (positions_piece - 1); // Remove esse bit
            }
            if(attacker_bit) break;
        }
        if(!attacker_bit){
            //At this point we know there's no piece from cur_turn that can take
            //We need to check if we can take with the king and still be safe
            if(is_king_last_capture(&game->estadoJogo,cur_turn,pos_cap) && !king_has_taken){
                indx++;
                see[indx] = current_piece_value - see[indx-1];
                king_has_taken = 1;
            }
            end_see = 1;
        }
        else{
            indx++;
            see[indx] = current_piece_value - see[indx-1];
            pieces_checked[cur_turn][attacker_type] |= attacker_bit;
            occupied &= ~attacker_bit;
            current_piece_value = pieces_value[attacker_type];
            cur_turn = (cur_turn==brancas) ? pretas : brancas;
        }
    }
    // NegaMax backpropagation (standing pat option)
    /* This works by checking if (indx-1) player will rather recapture on each instance or not , as if -see[indx] is inferior too see[indx-1] , 
    that would mean that player (indx-1) is losing material in that capture , because see[indx] would then be greater than see[indx-1] , indicating
    material win.*/
    for(int i=indx; i > 0 ; i--){
        if (-see[i] < see[i - 1]) {
            see[i - 1] = -see[i];
        }
    }
    return see[0];
}
