#include "engine/chess_lib/engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdlib.h>

int getOption(void){
    int o=0;
    printf("Insira a resolução desejada:\n"
            "(1)- 670x540\n"
            "(2)- 900x720\n"
            "(3)- 1200x900\n"
            "(DEFAULT) - FULLSCREEN\n");
    int u = scanf("%d",&o);
    if(u) return o;
    return 3;
}

/*Função que dado uma opção de resolução atribui os valores de largura e altura da janela*/
void setResolution(int * resX,int * resY,int optn){
    switch(optn){
        case 1:
            *resX=670;*resY=540;
        break;
        case 2:
            *resX=900;*resY=720;
        break;
        case 3:
            *resX=1200;*resY=900;
        break;
        //Resolução default será fullscreen
        default:
        break;
    }
}

SDL_Initializators sdl_initializer(void){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window;
    int op = getOption();
    int resX = 1200 , resY = 900;
    setResolution(&resX,&resY,op);
    window = SDL_CreateWindow("CChess", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, resX , resY , 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    //para não precisar de dar scale às imagens no ecrã
    SDL_RenderSetLogicalSize(renderer, 1200, 900);
    SDL_Initializators init = {.renderer = renderer , .window = window};
    return init;
}

void freesfxchunks(Mix_Chunk * sfxarray[]){
    for(int i = 0 ; i < 5 ; i++)
        Mix_FreeChunk(sfxarray[i]);
}



void free_allocated_memory(GameStruct * game , GUISettings * user , Mix_Chunk * sfxarray[]){
    for(int i = 0; i < 12; i++){
        SDL_DestroyTexture(user->textures.chessPieces[i]);
    }
    SDL_DestroyTexture(user->textures.tabTexture);
    SDL_DestroyTexture(user->textures.fundo);
    SDL_DestroyTexture(user->textures.logo);

    SDL_DestroyTexture(user->textures.arrow[0]);
    SDL_DestroyTexture(user->textures.arrow[1]);

    for(int i = 0; i < 2; i++){
        SDL_DestroyTexture(user->textures.miscTextures[i]);
    }
    SDL_DestroyRenderer(user->gameRenderer);
    freesfxchunks(sfxarray);
    TTF_CloseFont(user->fonteJogoTitles);
    TTF_CloseFont(user->fonteJogoSmallerTitles);
    IMG_Quit();
    SDL_Quit();
}