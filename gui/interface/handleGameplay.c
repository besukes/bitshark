#include "engine/chess_lib/engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#define ITERATIVE_DEEPENING 1
#define BOT_PLAYS_BLACK 1
#define BOT_PLAYS_WHITE 1



void softReset(GameStruct *game){
    game->pieceCoords = 0;
    game->pieceSelecionada = Empty;
    game->selected_piece_attacks = 0;
}

void playBotMove(GameStruct *game, GUISettings *settings, Mix_Chunk *sfxarray[], CorPiece turn){
    GameStruct game_aux = *game;
    CorPiece op_turn = (turn == brancas) ? pretas : brancas;
    // depois substituir por game_aux
    Jogada best_move = get_best_move(&game_aux, turn , ITERATIVE_DEEPENING);
    if (best_move.peca_movida == Empty || best_move.destino >= 64 || best_move.origem >= 64){
        game->turnoJogador = op_turn;
        return;
    }
    atualizaJogada(game, &best_move, turn);
    TipoJogada t = check_move(game, &best_move, turn);
    if (t == Checkmate){
        settings->screen = WinScreen;
        game->estadoJogo.checkMate = 1;
    }
    game->estadoJogo.king_in_check[turn] = 0;
    game->promoted.pawnPromoted = 0;
    updateScore(game, &best_move, turn);
    if (best_move.peca_capturada != Empty)
        capturepiece_sfx(sfxarray);
    else if (game->estadoJogo.king_in_check[op_turn])
        check_sfx(sfxarray);
    else
        movepiece_sfx(sfxarray);
    game->turnoJogador = op_turn;
    game->turns++;
    game->moved_to_square = best_move.destino;
    uint64_bit key = compute_zobrist(game, turn);
    if (hash_stack_indx >= 2000)
        settings->screen = WinScreen;
    else hash_key_stack[hash_stack_indx++] = key;
    game->position_eval = game_aux.position_eval;
}


// teste temporario do bot , temos de mudar eventualmente
//&& game->turnoJogador == brancas
void handleJogadaChess(GameStruct *game, GUISettings *settings, SDL_Event event, Mix_Chunk *sfxarray[]){
    if(game->turnoJogador == pretas && BOT_PLAYS_BLACK) playBotMove(game, settings,sfxarray,pretas);
    else if(game->turnoJogador == brancas && BOT_PLAYS_WHITE) playBotMove(game,settings,sfxarray,brancas);
    else if(event.type == SDL_MOUSEBUTTONDOWN){
        if(event.button.button == SDL_BUTTON_LEFT && game->isKeyPressedDown == 0){
            game->isKeyPressedDown = 1;
            cleanArrowEvent(game);
            efetuaEventoClique(game, settings, &event);
            game->jogada = Valid;
        }
        else if ((event.button.button == SDL_BUTTON_RIGHT) && game->arrows.is_drawing_arrows == 0){
            efetuaEventoClickArrows(game, event);
        }
    }
    else if (event.type == SDL_MOUSEBUTTONUP){
        if (event.button.button == SDL_BUTTON_LEFT){
            game->isKeyPressedDown = 0;
            efetuaEventoSoltar(game, settings, event, sfxarray);
            if (game->jogada != Invalid && !game->promoted.pawnPromoted){
                game->turnoJogador = (game->turnoJogador == brancas) ? pretas : brancas;
                if (game->turnoJogador == brancas)
                    game->turns++;
            }
            softReset(game);
            uint64_bit key = compute_zobrist(game,game->turnoJogador);
            printf("[DEBUG] Current key : %llu , Calculated key : %llu\n",game->cur_pos_key,key);
            printf("Turno Jogador : %d\n",game->turnoJogador);
        }
        else if ((event.button.button == SDL_BUTTON_RIGHT) && game->arrows.is_drawing_arrows){
            efetuaEventoSoltarArrows(game, event);
        }
    }
}
