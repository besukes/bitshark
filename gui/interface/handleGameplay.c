#include "engine/chess_lib/engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#define ITERATIVE_DEEPENING 1

void softReset(GameStruct * game){
    game->pieceCoords = 0;
    game->pieceSelecionada = Empty;
    game->selected_piece_attacks = 0;
}

//teste temporario do bot , temos de mudar eventualmente
//&& game->turnoJogador == brancas
void handleJogadaChess(GameStruct* game , GUISettings * settings,SDL_Event event , Mix_Chunk * sfxarray[]){
    if(game->turnoJogador == pretas){
        GameStruct game_aux = *game;
        game_aux.indx_lastmoves = 0;
        //depois substituir por game_aux
        Jogada best_move = get_best_move(game,pretas,ITERATIVE_DEEPENING);
        if(best_move.peca_movida == Empty || best_move.destino >= 64 || best_move.origem >= 64){
            game->turnoJogador = brancas;
            return;
        }
        atualizaJogada(game,&best_move,pretas);
        if(best_move.promocao){
            promotePiece(game,Queen,1ULL<<best_move.destino,pretas);
        }
        TipoJogada t = check_move(game,&best_move,pretas);
        if(t == Checkmate ){
            settings->screen = WinScreen;
        }
        notInCheck(game);
        update_en_passant(game,&best_move,pretas);
        game->promoted.pawnPromoted = 0;
        updateScore(game);
        if(game->indx_lastmoves > 0) capturepiece_sfx(sfxarray);
        else if(game->estadoJogo.king_in_check[brancas]) check_sfx(sfxarray);
        else movepiece_sfx(sfxarray);
        game->indx_lastmoves = 0;
        game->turnoJogador = brancas;
        game->turns++;
        game->moved_to_square = best_move.destino;
    }
    else if(game->turnoJogador == brancas){
        GameStruct game_aux = *game;
        game_aux.indx_lastmoves = 0;
        //depois substituir por game_aux
        Jogada best_move = get_best_move(game,brancas,ITERATIVE_DEEPENING);
        if(best_move.peca_movida == Empty || best_move.destino >= 64 || best_move.origem >= 64){
            game->turnoJogador = pretas;
            return;
        }
        atualizaJogada(game,&best_move,brancas);
        if(best_move.promocao){
            promotePiece(game,Queen,1ULL<<best_move.destino,brancas);
        }
        TipoJogada t = check_move(game,&best_move,brancas);
        if(t == Checkmate ){
            settings->screen = WinScreen;
        }
        notInCheck(game);
        update_en_passant(game,&best_move,brancas);
        game->promoted.pawnPromoted = 0;
        updateScore(game);
        if(game->indx_lastmoves > 0) capturepiece_sfx(sfxarray);
        else if(game->estadoJogo.king_in_check[pretas]) check_sfx(sfxarray);
        else movepiece_sfx(sfxarray);
        game->indx_lastmoves = 0;
        game->turnoJogador = pretas;
        game->turns++;
        game->moved_to_square = best_move.destino;
    }
    else if(event.type == SDL_MOUSEBUTTONDOWN && game->turnoJogador == brancas){
        if(event.button.button == SDL_BUTTON_LEFT && game->isKeyPressedDown ==0){
            game->isKeyPressedDown = 1;
            cleanArrowEvent(game);
            efetuaEventoClique(game,settings,&event);
            game->jogada = Valid;
        }
        else if( (event.button.button == SDL_BUTTON_RIGHT) && game->arrows.is_drawing_arrows == 0){
            efetuaEventoClickArrows(game,event);
        }
    }
    else if(event.type == SDL_MOUSEBUTTONUP && game->turnoJogador == brancas){
        if(event.button.button == SDL_BUTTON_LEFT){
            game->isKeyPressedDown = 0;
            efetuaEventoSoltar(game,settings,event,sfxarray);
            if(game->jogada != Invalid && !game->promoted.pawnPromoted){
                game->turnoJogador = (game->turnoJogador == brancas) ? pretas : brancas;
                if(game->turnoJogador == brancas) game->turns++;
            }
            softReset(game);
        }
        else if( (event.button.button == SDL_BUTTON_RIGHT) && game->arrows.is_drawing_arrows){
           efetuaEventoSoltarArrows(game,event);
        }
    }
}



