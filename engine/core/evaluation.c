#include "engine/chess_lib/engine.h"
#include <engine/chess_lib/evals.h>
#include <engine/chess_lib/PreComputed_files.h>

extern unsigned long long passed_pawn_mask[2][64];


// Usada pelo Null Move Pruning: evita aplicar a poda em finais so com reis e peoes,
// onde o risco de zugzwang (em que "passar a vez" seria de facto a melhor jogada) e alto
// e a poda pode causar erros de avaliacao graves.
int has_non_pawn_material(GameStruct * game, CorPiece turn){
    return (game->estadoJogo.tabuleirojogo[turn][Horse] || game->estadoJogo.tabuleirojogo[turn][Bishop] ||
            game->estadoJogo.tabuleirojogo[turn][Rook]  || game->estadoJogo.tabuleirojogo[turn][Queen]);
}




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



int calculate_stronger_side(CorPiece * weak , CorPiece * strong , EstadoJogo * estado){
    int white_queens = __builtin_popcountll(estado->tabuleirojogo[brancas][Queen]);
    int black_queens = __builtin_popcountll(estado->tabuleirojogo[pretas][Queen]);

    int white_minors = __builtin_popcountll(estado->tabuleirojogo[brancas][Horse])
                      + __builtin_popcountll(estado->tabuleirojogo[brancas][Bishop]);
    int black_minors = __builtin_popcountll(estado->tabuleirojogo[pretas][Horse])
                      + __builtin_popcountll(estado->tabuleirojogo[pretas][Bishop]);

    int white_rooks = __builtin_popcountll(estado->tabuleirojogo[brancas][Rook]);
    int black_rooks = __builtin_popcountll(estado->tabuleirojogo[pretas][Rook]);

    int white_material = white_queens*800 + white_minors*300 + white_rooks*500;
    int black_material = black_queens*800 + black_minors*300 + black_rooks*500;

    if((white_material > black_material + 1000) || (white_material >= 500 && black_material <= 300)) {
        *weak = pretas;
        *strong = brancas;
        return 1;
    }
    else if((black_material > white_material + 1000) || (black_material >= 500 && white_material <= 300)){
        *weak = brancas;
        *strong = pretas;
        return 1;
    }
    return 0;
}


int mopup_eval(GameStruct * game){
    CorPiece weak = pretas, strong = brancas;
    int exists_stronger = calculate_stronger_side(&weak,&strong,&game->estadoJogo);
    int safe2apply_mopup = game->is_end_game || exists_stronger;
    if(safe2apply_mopup){
        int strong_king_pos = posTabuleiro(game->estadoJogo.tabuleirojogo[strong][King]),
            weak_turn_king_pos = posTabuleiro(game->estadoJogo.tabuleirojogo[weak][King]);
        if(weak_turn_king_pos < 0 || strong_king_pos < 0) return 0; // segurança: sem rei (não deve acontecer)

        // King Manhattan Distance (linha/coluna) ao centro do tabuleiro
        int weak_rank = weak_turn_king_pos / 8, weak_file = weak_turn_king_pos % 8;
        int dist_rank = (weak_rank <= 3) ? (3 - weak_rank) : (weak_rank - 4);
        int dist_file = (weak_file <= 3) ? (3 - weak_file) : (weak_file - 4);
        int weak_king_manhattan_dist_to_center = dist_rank + dist_file;

        // Distância entre os dois reis: sem aproximar o próprio rei do rei adversário, a torre
        // sozinha não consegue fechar o mate (precisa do apoio do rei para cortar as casas de
        // fuga) — sem este termo, o motor "empurra" o rei adversário para a borda mas nunca
        // convergir para o fechar, ficando a oscilar indefinidamente perto do mate sem o concluir.
        int strong_rank = strong_king_pos/8, strong_file = strong_king_pos%8;
        int dr = (strong_rank > weak_rank) ? (strong_rank - weak_rank) : (weak_rank - strong_rank);
        int df = (strong_file > weak_file) ? (strong_file - weak_file) : (weak_file - strong_file);
        int kings_chebyshev = (dr > df) ? dr : df;
 
        int mopup = 65*weak_king_manhattan_dist_to_center + 25*(7 - kings_chebyshev);
        return ((strong == brancas) ? mopup : -mopup);
    }
    return 0;
}


int rookOpenFilesBonus(EstadoJogo * state , CorPiece turn){
   int bonus = 0;
    uint64_bit rooks = state->tabuleirojogo[turn][Rook];
    uint64_bit occupancy = state->bitboard_todas_pieces;
    while(rooks != 0){
        uint64_bit single = rooks & (-rooks);
        int column = posTabuleiro(single) % 8;
        if(!(rook_files[column] & (occupancy & ~single)))
            bonus += 15;
        rooks &= (rooks - 1);
    }
    return bonus;
}


int mobilityScore(GameStruct * game , Pieces piece , uint64_bit piece_pos , CorPiece turn){
    int pawn_structure_bonus = 10;
    uint64_bit attks = get_magic_piece_attacks(piece_pos,piece,game,turn,0,game->estadoJogo.bitboard_todas_pieces);
    int number_attks = __builtin_popcountll(attks);
    if(piece == Bishop || piece == Rook) return (number_attks*2); // Really good to have active bishops and rooks
    else if(piece == Queen) return (number_attks/2); // How many squares a queen see is not as relevant
    else if(piece == Horse) return (number_attks); // An active knight is good
    else if(piece == Pawn && (attks & game->estadoJogo.tabuleirojogo[turn][Pawn])) 
        return (pawn_structure_bonus); // Benefit pawn structures
    return 0; // King mobility score will have to be implemented later with deeper thought
}


int passedPawnBonus(uint64_bit piece_pos , GameStruct * game , CorPiece turn){
    CorPiece op_turn = (turn == brancas) ? pretas : brancas;
    int pos_tab = posTabuleiro(piece_pos);
    int bonus = 0;
    int indx = (turn == brancas) ? pos_tab/8 : (7-pos_tab/8);
    //SE nao houver peoes da outra cor na passed_pawn_mask , entao o peao é um passed pawn
    if(!(passed_pawn_mask[turn][pos_tab] & game->estadoJogo.tabuleirojogo[op_turn][Pawn]))
        bonus += passed_pawn_bonus[indx];
    return bonus;
}


int kingSafetyBonus(int position , int line , int col , GameStruct* game , CorPiece turn){
    int used_line = (turn == brancas) ? (line + 1) : (line - 1);
    if(line<0 || line > 7) return 0;
    int left_side = (col>0) ? (col - 1) : col;
    int right_side = (col<7) ? (col + 1) : col;
    uint64_bit mask = 1ULL<<(8*used_line + left_side) | 1ULL<<(8*used_line + col) | 1ULL<<(8*used_line + right_side);
    uint64_bit intersect = mask & game->estadoJogo.tabuleirojogo[turn][Pawn];
    int number_pawns = __builtin_popcountll(intersect);
    Boolean is_castled = game->estadoJogo.is_castled[turn];
    if(is_castled) return (-20 + number_pawns*10);
    else if(!is_castled && !game->estadoJogo.canCastle[turn][Short] && !game->estadoJogo.canCastle[turn][Long])
        return (-10); //Penalize king for not being able to castle early/middle game
    return 0;
}


int evaluate_piece(uint64_bit piece_pos , Pieces piece_type , CorPiece turn , GameStruct * game){
    //piece evaluation is based on the position of the piece , its mobility , what pieces it attacks
    //and how it coordinates with other pieces
    /*  
        piece_eval = piece_score + position_score + mobility_score;
    */
    int position_score = 0 , piece_score = pieces_value[piece_type];
    int pos = posTabuleiro(piece_pos);
    if(pos==(-1)) return 0;
    int line = pos/8 , column = pos%8 , indx = (turn==brancas) ? ((7-line)*8 + column) : pos;
    switch(piece_type){
        case Pawn :
            if(game->is_end_game){
                position_score = pawn_evals_black_endgame[indx] + passedPawnBonus(piece_pos,game,turn);
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
            if(game->is_end_game){
                position_score = black_king_endGame_evals[indx];
            }
            else{
                position_score = black_king_middleGame_evals[indx] + kingSafetyBonus(pos,line,column,game,turn);
                if(game->estadoJogo.is_castled[turn]) position_score += 50;
            }
        break;
        default:break;
    }
    int mobility_score = mobilityScore(game,piece_type,piece_pos,turn);
    return (piece_score + position_score + mobility_score);
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


/*  This function is responsible for getting each piece attacks for SEE logic.
    Still needs to be done.*/
uint64_bit get_SEE_captures(GameStruct * game ,uint64_bit single , Pieces piece , uint64_bit occupied , CorPiece turn){
    int pos_tab = posTabuleiro(single);
    if(pos_tab == (-1)) return 0;
    switch(piece){
        case Pawn :
            return (get_pawn_attacks(single,turn));
        break;
        case Rook :
            return (get_magic_cross_attacks(pos_tab,occupied));
        break;
        case Horse :
            return (get_magic_knight_attacks(pos_tab));
        break;
        case Bishop :
            return (get_magic_sliding_attacks(pos_tab,occupied));
        break;
        case Queen :
            return (get_magic_sliding_attacks(pos_tab,occupied) | get_magic_cross_attacks(pos_tab,occupied));
        break;
        case King :
            return (get_king_moves(single));
        break;
        default :
            return 0ULL;
        break;
    }
}


/*  Verifies whether the king can take a certain square without being capture by another piece.
    This is important , as he can be the last one to take on SEE but not if he can be captured after.*/
int is_king_last_capture(GameStruct * game , CorPiece cur_turn , uint64_bit pos_cap , uint64_bit occupancy){
    uint64_bit king_pos = game->estadoJogo.tabuleirojogo[cur_turn][King];
    uint64_bit king_moves = get_king_moves(king_pos);
    if(king_moves & pos_cap){
        int op_turn = (cur_turn == brancas) ? pretas : brancas;
        occupancy = (occupancy & ~king_pos) | pos_cap;
        return (!is_attacked_square(pos_cap,op_turn,game,occupancy));
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
    uint64_bit origem_bit = 1ULL<<jogada->origem;
    uint64_bit occupied = game->estadoJogo.bitboard_todas_pieces & ~(pos_cap | origem_bit);
    int see[32]; see[0] = pieces_value[jogada->peca_capturada];
    uint64_bit pieces_checked[2][NUMBER_PIECES] = {0};
    pieces_checked[turn][jogada->peca_movida] |= origem_bit;
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
                if(get_SEE_captures(game,single_pos,(Pieces)i,occupied,cur_turn) & pos_cap){
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
            if(is_king_last_capture(game,cur_turn,pos_cap,occupied) && !king_has_taken){
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
