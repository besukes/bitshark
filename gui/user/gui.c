#include "engine/chess_lib/engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdio.h>
#include <math.h>

void desenhaBitshark(GUISettings * settings){

}

void desenhaMoved(GameStruct * game,GUISettings * settings){
    int sq = game->moved_to_square;
    if(sq == (-1)) return;
    int line = sq / 8 , column = sq % 8;

    SDL_SetRenderDrawColor(settings->gameRenderer, 0 , 0 , 140 , 40); 
    SDL_Rect moved = {88*column+248, 1080 - (88 * line + 366),89,89};
    SDL_RenderFillRect(settings->gameRenderer, &moved);
}



void desenhaInterfaceJogo(GameStruct * game ,GUISettings * settings){
    desenhaFundo(settings,settings->textures.fundo);
    SDL_Rect tabuleiro = {250,100,700,700};
    SDL_RenderCopy(settings->gameRenderer,settings->textures.tabTexture,NULL,&tabuleiro);
    if(game->selected_piece_attacks != 0){
        uint64_bit op = get_opposing_colour_bitboard(&game->estadoJogo,game->turnoJogador);
        desenharPieceAttacks(settings,game->estadoJogo.enpassant, game->selected_piece_attacks , op);
    }

    desenhaBitshark(settings);
    desenhaMoved(game,settings);
    desenhaCheck(game,settings);

    for(int i = 0 ; i < 6 ; i++){
        desenhaTipoPiece(game->estadoJogo.tabuleirojogo[0][i],(Pieces)i,settings,game,0);
    }
    for( int i = 0; i < 6; i++){
        desenhaTipoPiece(game->estadoJogo.tabuleirojogo[1][i],(Pieces)(i),settings,game,6);
    }

    if(game->promoted.pawnPromoted) desenhaPromotion(game,settings);

    desenhaArrows(game,settings->gameRenderer,settings->textures.arrow);
}