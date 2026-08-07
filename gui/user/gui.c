#include "engine/chess_lib/engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdio.h>
#include <math.h>





void desenhaInterfaceJogo(GameStruct * game ,GUISettings * settings){
    desenhaFundo(settings,settings->textures.fundo);
    SDL_Rect tabuleiro = {210,140,800,800};
    SDL_RenderCopy(settings->gameRenderer,settings->textures.tabTexture,NULL,&tabuleiro);
    if(game->selected_piece_attacks != 0){
        uint64_bit op = get_opposing_colour_bitboard(&game->estadoJogo,game->turnoJogador);
        desenharPieceAttacks(settings,game->estadoJogo.enpassant, game->selected_piece_attacks , op);
    }
    desenhaCheck(game,settings);
    for(int i = 0 ; i < 6 ; i++){
        desenhaTipoPiece(game->estadoJogo.tabuleirojogo[0][i],(Pieces)i,settings,game,0);
    }
    for( int i = 0; i < 6; i++){
        desenhaTipoPiece(game->estadoJogo.tabuleirojogo[1][i],(Pieces)(i),settings,game,6);
    }

    if(game->promoted.pawnPromoted) desenhaPromotion(game,settings);

    SDL_Rect turn = {250,(-40),700,210};
    SDL_RenderCopy(settings->gameRenderer,settings->textures.miscTextures[game->turnoJogador],NULL,&turn);
    desenhaArrows(game,settings->gameRenderer,settings->textures.arrow);
}