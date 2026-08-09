#include "engine/chess_lib/engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

Jogada killer_moves[MAX_DEPTH_SEARCH][2] = {0};
int history_table[NUMBER_PIECES*2][NUM_SQUARES] = {0};



void interfaceCChess(GameStruct * game ,GUISettings * settings , Mix_Chunk * sfxarray[]){
    SDL_Event event;
    SDL_PollEvent(&event);
    //enquanto o utilizador nao clicar no botao para sair ele continua no jogo
    while(event.type != SDL_QUIT && game->jogada!= Leave){
        SDL_PollEvent(&event);
        SDL_RenderClear(settings->gameRenderer);
        settings->ticks = SDL_GetTicks();
        if(event.type == SDL_MOUSEMOTION){
            settings->posMouseX = event.motion.x;
            settings->posMouseY = event.motion.y;
            SDL_Event tmp;
            while(SDL_PeepEvents(&tmp,1,SDL_GETEVENT,SDL_MOUSEMOTION,SDL_MOUSEMOTION) > 0){
                settings->posMouseX = tmp.motion.x;
                settings->posMouseY = tmp.motion.y;
            }
        }
        if(settings->screen == GameScreen){
            if(game->game_needs_initialization) initializeGame(game);
            handleJogadaChess(game,settings,event , sfxarray);
            desenhaInterfaceJogo(game,settings);
        }
        else{
            printf("Numero de moves : %d",game->turns);
            while(event.type != SDL_QUIT ){
                SDL_RenderClear(settings->gameRenderer);
                SDL_PollEvent(&event);
                desenhaInterfaceJogo(game,settings);
                SDL_RenderPresent(settings->gameRenderer);
            }
            
        } //Por fazer endscreen
        SDL_RenderPresent(settings->gameRenderer);
    }
}

int main(void){
    SDL_Initializators init = sdl_initializer();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    GUISettings settings = initGUISettings(init.renderer,init.window);
    GameStruct game = initGameStruct();
    Mix_Chunk * sfxarray[10];
    initsfx(sfxarray);
    init_zobrist();
    tt_init();
    interfaceCChess(&game,&settings , sfxarray);
    free_allocated_memory(&game,&settings , sfxarray);
    return 0;
}