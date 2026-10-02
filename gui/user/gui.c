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
    
    
    SDL_Color cor = {255, 255 , 255 , 200};
    int switch_case = 0;
    char res[128];
    int eval_size = eval + 0.5 , n_mate = 0;
    int starting_white_y = 450 - 29*eval_size;
    starting_white_y = (starting_white_y <= 100) ? 105 : starting_white_y;
    starting_white_y = (starting_white_y > 800) ? 800 : starting_white_y;
    roundedBoxRGBA(settings->gameRenderer,170,100,220,800,0,60,60,60, 255);
    roundedBoxRGBA(settings->gameRenderer,170, starting_white_y ,220,800,0,255,255,255, 255);
    if(eval <= (-850.0)){
        eval_size = -12;
        n_mate = (99999 - eval*(-100));
        snprintf(res,128,"M%d",n_mate);
    }
    else if(eval >= 850.0){
        eval_size = 12;
        n_mate = (99999 - eval*100);
        snprintf(res,128,"M%d",n_mate);
    }
    else snprintf(res,128,"%.1f",(eval<0)?-eval:eval);
    if(eval < 0) 
    {
        cor.r = 255, cor.g = 255, cor.b = 255;
        switch_case = 0;
    }
    else{
        cor.r = 0, cor.g = 0, cor.b = 0;
        switch_case = 1;
    }  

    renderTextoCentradoBasico(settings->gameRenderer,settings->fonteJogoSmallerTitles,res,cor,197,110+(switch_case*640),0.75);
}


void desenhaMoved(GameStruct * game,GUISettings * settings){
    int sq = game->moved_to_square;
    if(sq == (-1)) return;
    int line = sq / 8 , column = sq % 8;

    SDL_SetRenderDrawColor(settings->gameRenderer, 0 , 0 , 140 , 40); 
    SDL_Rect moved = {88*column+248, 1080 - (88 * line + 366),89,89};
    SDL_RenderFillRect(settings->gameRenderer, &moved);
}

void desenhaWinScreen (GameStruct * game , GUISettings * settings){
    SDL_Color white = {.r = 255 , .b = 255 , .g = 255 ,.a = 255};
    char result[128];
    if(settings->winner == brancas) snprintf(result,sizeof(result),"Brancas Venceram em : %d",game->turns);
    else if(settings->winner == pretas) snprintf(result,sizeof(result),"Pretas Venceram em : %d",game->turns);
    else snprintf(result,sizeof(result),"Empate em : %d",game->turns);
    renderTextoCentradoSombra(settings->gameRenderer,settings->fonteJogoTitles,result,white,600,800,2);
    renderTextoCentradoSombra(settings->gameRenderer,settings->fonteJogoTitles,"Retry",white,1000,800,2);
    renderTextoCentradoSombra(settings->gameRenderer,settings->fonteJogoTitles,"Exit",white,200,800,2);
}

void desenhaChoosingScreen (GameStruct * game , GUISettings * settings){
    SDL_Rect bg = {0,0,0,0};
    SDL_SetRenderDrawColor(settings->gameRenderer, 125, 125, 255, 255);
    SDL_RenderDrawRect(settings->gameRenderer, &bg);
    SDL_Color white = {.r = 255 , .b = 255 , .g = 255 ,.a = 255};
    renderTextoCentradoSombra(settings->gameRenderer,settings->fonteJogoTitles,"Escolha o tipo de jogador",white,600,400,2);
    renderTextoCentradoSombra(settings->gameRenderer,settings->fonteJogoTitles,"Player",white,500,800,2);
    renderTextoCentradoSombra(settings->gameRenderer,settings->fonteJogoTitles,"Bot",white,700,800,2);
    
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
        Pieces tested_piece = BENCHMARK_TESTED;
        desenhaTipoPiece(game->estadoJogo.tabuleirojogo[0][tested_piece],tested_piece,settings,game,0);
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