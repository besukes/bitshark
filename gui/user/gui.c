#include <engine/chess_lib/engine.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdio.h>
#include <math.h>




void desenhaBitshark(GUISettings * settings){
    roundedBoxRGBA(settings->gameRenderer,250,8,820,95,5,0,0,0, 200);
    SDL_Rect logo = {264,16,70,70};
    SDL_RenderCopy(settings->gameRenderer,settings->textures.logo,NULL,&logo);
    SDL_Color white = {.r = 255 , .b = 255 , .g = 255 ,.a = 255};
    renderTextoCentradoSombra(settings->gameRenderer,settings->fonteJogoTitles,"Bitshark",white,435,6,1.5);
    renderTextoCentradoBasico(settings->gameRenderer,settings->fonteJogoSmallerTitles , "2500 elo" , white , 394 , 60 , 0.55);
}


void desenhaEvalBar(float eval , GUISettings * settings){
    //Evaluation Bar
    roundedBoxRGBA(settings->gameRenderer,180,100,220,800,0,0,0,0, 200);

    int eval_size = eval + 0.5;
    if(eval <= (-850.0)) eval_size = -12;
    else if(eval >= 850.0) eval_size = 12;
    int starting_white_y = 450 - 29*eval_size;
    starting_white_y = (starting_white_y <= 100) ? 105 : starting_white_y;
    starting_white_y = (starting_white_y > 800) ? 800 : starting_white_y;
    roundedBoxRGBA(settings->gameRenderer,180, starting_white_y ,220,800,0,255,255,255, 200);
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
    desenhaEvalBar(game->position_eval,settings);
    desenhaMoved(game,settings);
    desenhaCheck(game,settings);

    if(CHECKMATE_BENCHMARK){
        desenhaTipoPiece(game->estadoJogo.tabuleirojogo[0][Queen],Queen,settings,game,0);
        desenhaTipoPiece(game->estadoJogo.tabuleirojogo[0][King],King,settings,game,0);

        desenhaTipoPiece(game->estadoJogo.tabuleirojogo[1][King],King,settings,game,6);
    }
    else{
        for(int i = 0 ; i < 6 ; i++){
            desenhaTipoPiece(game->estadoJogo.tabuleirojogo[0][i],(Pieces)i,settings,game,0);
        }
        for( int i = 0; i < 6; i++){
            desenhaTipoPiece(game->estadoJogo.tabuleirojogo[1][i],(Pieces)(i),settings,game,6);
        }
    }

    if(game->promoted.pawnPromoted) desenhaPromotion(game,settings);

    desenhaArrows(game,settings->gameRenderer,settings->textures.arrow);
}