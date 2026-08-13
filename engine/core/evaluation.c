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

        // Distância de Chebyshev (linha/coluna) ao centro do tabuleiro
        int op_rank = op_turn_king_pos / 8, op_file = op_turn_king_pos % 8;
        int dist_rank = (op_rank <= 3) ? (3 - op_rank) : (op_rank - 4);
        int dist_file = (op_file <= 3) ? (3 - op_file) : (op_file - 4);
        int op_king_dist_to_center = (dist_rank > dist_file) ? dist_rank : dist_file;

        // Distância entre os dois reis: sem aproximar o próprio rei do rei adversário, a torre
        // sozinha não consegue fechar o mate (precisa do apoio do rei para cortar as casas de
        // fuga) — sem este termo, o motor "empurra" o rei adversário para a borda mas nunca
        // convergir para o fechar, ficando a oscilar indefinidamente perto do mate sem o concluir.
        int mr = turn_king_pos/8, mf = turn_king_pos%8;
        int dr = mr>op_rank ? mr-op_rank : op_rank-mr;
        int df = mf>op_file ? mf-op_file : op_file-mf;
        int kings_distance = (dr > df) ? dr : df;
 
        return 65*op_king_dist_to_center + 25*(7 - kings_distance);
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

//Needs to be done
uint64_bit get_see_piece_attacks(uint64_bit single , Pieces piece , uint64_bit occupied , CorPiece turn){
    uint64_bit piece_attks = 0;
    return piece_attks;
}


int static_exchange_eval(GameStruct * game , Jogada * jogada , CorPiece turn){
    CorPiece cur_turn = (turn==brancas) ? pretas : brancas;
    uint64_bit pos_cap = 1ULL<<jogada->destino;
    uint64_bit occupied_sq = game->estadoJogo.bitboard_todas_pieces & ~(pos_cap | (1ULL<<jogada->origem));
    int see[32]; see[0] = pieces_value[jogada->peca_capturada];
    int indx = 0;
    int current_piece_value = pieces_value[jogada->peca_movida];
    int end_see = 0; // For tests it could be turned to 1

    while(!end_see){
        indx++;
        see[indx] = current_piece_value - gain[indx-1];
        uint64_bit attacker_bit = 0;
        int attacker_type = 0;
        for(int i=1;i<NUMBER_PIECES;i++){
            uint64_bit positions_piece = game->estadoJogo.tabuleirojogo[cur_turn][i];
            int exist_attackers = attackers[cur_turn][i] > 0;
            while(positions_piece != 0){
                uint64_bit single_pos = positions_piece & (-positions_piece);
                if(get_see_piece_attacks(single_pos,(Pieces)i,occupied_sq,cur_turn) & pos_cap){
                    attacker_type = i;
                    attacker_bit = single_pos;
                }
                positions_piece &= (positions_piece - 1); // Remove esse bit
                if(attacker_bit) break;
            }
        }
        if(!attacker_bit) end_see = 1;
        else{
            occupied_sq &= ~attacker_bit;
            current_piece_value = pieces_value[attacker_type];
            cur_turn = (cur_turn==brancas) ? pretas : brancas;
        }
    }
    // Minimax backpropagation (standing pat option)
    while (--d > 0) {
        if (see[d - 1] < -see[d]) {
            see[d - 1] = -see[d];
        }
    }
    return see[0];
}
