#include <engine.h>

//alpha usually starts at -99999 , int alpha = -99999; --Current best white evaluation
//beta usually starts at 99999 , int beta = 99999; --Current best black evaluation
//alpha - white eval , beta - black eval

uint64_bit get_best_move(GameStruct * game , CorPiece turn){
    int depth = MAX_DEPTH_SEARCH , ai_level = CHESS_AI_DIFFICULTY;
    int pieces_eval[2][NUMBER_PIECES] = {};
    initializeStructs(pieces_eval,NUMBER_PIECES);
    evaluate(game,turn,ai_level,pieces_eval);
    Moves best_move = move_algorithm(game,turn,depth,ai_level,-99999,99999,0,0,pieces_eval);
    return (best_move.move);
}


Moves move_algorithm(GameStruct * game , CorPiece turn , int depth , int ai_level , int alpha , int beta , int weval , int beval , int evals[2][NUMBER_PIECES]){
    uint64_bit positional_best_move = 0; //No move to start with
    SearchInfo search = {.ai_level = ai_level , .alpha = alpha , .beta = beta, .depth = depth , .bot_colour = turn , .turn = turn};
    for(int i=0;i<NUMBER_PIECES;i++){
        int counter=0;
        uint64_bit piece_bitboard = game->estadoJogo.tabuleirojogo[turn][i];
        Pieces piece = (Pieces)i;
        search.piece_type = piece;
        while(piece_bitboard != 0){
            if(piece_bitboard & 1ULL != 0){
                uint64_bit current_pos = 1ULL<<counter,
                           current_attacks = get_piece_attacks(current_pos,piece,game,turn) & ~get_same_colour_bitboard(&game->estadoJogo,turn);
                Moves best_searched = search_algorithm(current_pos,current_attacks,current_pos,game,evals,&search);
                if(best_searched.move_evaluation > search.alpha && turn == brancas){
                    search.alpha = best_searched.move_evaluation;
                    positional_best_move = best_searched.move;
                }
                else if(best_searched.move_evaluation < search.beta && turn == pretas){
                    search.beta = best_searched.move_evaluation;
                    positional_best_move = best_searched.move;
                }
            }
            counter++;
            piece_bitboard>>=1;
            if(search.alpha>=search.beta) break;
        }
    }
    int ret_eval = (turn==brancas) ? search.alpha : search.beta;
    Moves ret = {.move = positional_best_move , .move_evaluation = ret_eval};
    return ret;
}