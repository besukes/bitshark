#include "engine/chess_lib/engine.h"



void initsfx(Mix_Chunk * sfxarray[]){
    sfxarray[0] =  Mix_LoadWAV("sfx/move-self.mp3");
    sfxarray[1] =  Mix_LoadWAV("sfx/capture.mp3");
    sfxarray[2] =  Mix_LoadWAV("sfx/promote.mp3");
    sfxarray[3] =  Mix_LoadWAV("sfx/move-check.mp3");
    sfxarray[4] =  Mix_LoadWAV("sfx/game-end.mp3");
}

void loadTexturasPieces(SDL_Texture * chessPieces[12],SDL_Renderer * sdl_renderer){
    for(int i=0;i<12;i++){
        char str[30];
        sprintf(str,"assets/pieces/png/%d.png",i+1);
        chessPieces[i] = IMG_LoadTexture(sdl_renderer,str);
    }
}


void loadAssets(GUISettings * settings , SDL_Renderer * sdl_renderer){
    loadTexturasPieces(settings->textures.chessPieces,sdl_renderer);
    settings->textures.logo = IMG_LoadTexture(sdl_renderer,"assets/logo/bitshark");
    settings->textures.tabTexture = IMG_LoadTexture(sdl_renderer,"assets/tabuleiro/tab_2.png");
    settings->textures.arrow[0] = IMG_LoadTexture(sdl_renderer,"assets/misc/arrow_texture.png");
    settings->textures.arrow[1] = IMG_LoadTexture(sdl_renderer,"assets/misc/triangle_orange.png");
    settings->textures.fundo = IMG_LoadTexture(sdl_renderer,"assets/misc/forest.jpg");

    settings->textures.miscTextures[0] = IMG_LoadTexture(sdl_renderer,"assets/misc/white_moves.png");
    settings->textures.miscTextures[1] = IMG_LoadTexture(sdl_renderer,"assets/misc/black_moves.png");
}