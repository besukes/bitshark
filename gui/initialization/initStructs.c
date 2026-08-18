#include "engine/chess_lib/engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>




GUISettings initGUISettings(SDL_Renderer * sdl_renderer , SDL_Window * window){
    GUISettings settings;
    settings.gameRenderer = sdl_renderer;
    settings.window = window;
    settings.posMouseX = 0;
    settings.posMouseY = 0;
    settings.ticks = 0;
    settings.screen = GameScreen;
    loadAssets(&settings,sdl_renderer);
    TTF_Init();
    settings.fonteJogoTitles = TTF_OpenFont("assets/ttf/m6x11plus.ttf", 35);
    settings.fonteJogoSmallerTitles = TTF_OpenFont("assets/ttf/m6x11plus.ttf", 48);
    return settings;
}

EstadoJogo initEstadoJogo(void){
    EstadoJogo es;
    es.checkMate = 0;
    es.king_in_check[brancas] = 0;
    es.king_in_check[pretas] = 0;
    es.enpassant = 0;
    es.stalemate = 0;
    es.is_castled[0] = 0;
    es.is_castled[1] = 0;
    for(int i=0;i<2;i++){
        es.canCastle[i][0] = 1;
        es.canCastle[i][1] = 1;
    }
    initTabuleiro(&(es.tabuleirojogo[0][0]),0);
    initTabuleiro(&(es.tabuleirojogo[1][0]),56);
    init_other_bitboards(&es);
    return es;
}


void initArrowsStruct(GameStruct * game){
    game->arrows.is_drawing_arrows = 0;
    game->arrows.indx_drawable_arrows=0;
    game->arrows.arrows_vector = NULL; 
}


GameStruct initGameStruct(void){
    GameStruct game;
    game.game_needs_initialization = 1;
    game.isKeyPressedDown = 0;
    game.jogada = Valid;
    game.pieceSelecionada = Empty;
    game.turnoJogador = brancas;
    game.pieceCoords = 0;
    game.moved_to_square = (-1);
    game.promoted.pawnPromoted = 0;
    game.promoted.promotedSucessfully = 0;
    game.selected_piece_attacks = 0;
    game.promoted.promoted_square = 0;
    game.score_game = 0;
    game.turns=0;
    game.trying_to_leave = 0;
    game.cur_pos_key = 0;
    game.position_eval = 0;
    game.is_end_game = 0;
    initArrowsStruct(&game);
    return game;
}



void initializeGame(GameStruct * game){
    game->estadoJogo = initEstadoJogo();
    game->game_needs_initialization = 0;
    game->isKeyPressedDown = 0;
    game->jogada = Valid;
    game->pieceSelecionada = Empty;
    game->turnoJogador = brancas;
    game->pieceCoords = 0;
    game->moved_to_square = (-1);
    game->promoted.pawnPromoted = 0;
    game->selected_piece_attacks = 0;
    game->promoted.promoted_square = 0;
    game->score_game = 0;
    game->turns=0;
    game->trying_to_leave = 0;
    game->cur_pos_key = compute_zobrist(game,game->turnoJogador);
    game->position_eval = 0;
    game->is_end_game = 0;
    initArrowsStruct(game);
}


void initializeStructs(int matrix[2][NUMBER_PIECES],int indx){
    for(int i=0;i<indx;i++){
        matrix[0][i] = 0;
        matrix[1][i] = 0;
    }
}