#include <engine.h>



uint64_bit move_algorithm(GameStruct * game , CorPiece turn , int depth , int ai_level){
    uint64_bit best_move = 0;
    PieceEvaluation pieces_eval[2][NUMBER_PIECES];
    initializeStructs(pieces_eval,NUMBER_PIECES);
    evaluate(game,turn,ai_level,pieces_eval);
    int alpha = -99999; //Current best evaluation
    uint64_bit positional_best_move = 0; //No move to start with
    for(int i=0;i<NUMBER_PIECES;i++){
        int counter=0;
        uint64_bit piece_bitboard = game->estadoJogo.tabuleirojogo[turn][i];
        Pieces piece = (Pieces)i;
        while(piece_bitboard != 0){
            if(piece_bitboard & 1ULL != 0){
                uint64_bit current_pos = 1ULL<<counter,
                           current_attacks = get_piece_attacks(current_pos,piece,game,turn);
                Moves best_searched = search_algorithm(current_attacks,game,depth,ai_level,piece,alpha,pieces_eval,turn);
                if(best_searched.move_evaluation > alpha){
                    alpha = best_searched.move_evaluation;
                    positional_best_move = best_searched.move;
                }
            }
            counter++;
            piece_bitboard>>1;
        }
    }
    return best_move;
}