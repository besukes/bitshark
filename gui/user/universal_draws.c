#include "engine/chess_lib/engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdio.h>


void drawLevel(int nivel,SDL_Renderer * renderer , TTF_Font* f,int x , int y, float scale){
    char str[256];
    SDL_Color branco = {255, 255 , 255 , 255};
    SDL_Color red = {255, 0 , 0 , 255} , suave = {150,150,150,255}; 
    renderTextoCentradoBasico(renderer,f,"Level",branco,x,y,scale);
    if(nivel==10){
        sprintf(str,"%d",nivel);
        renderTextoCentradoBasico(renderer,f,str,red,x+66,y,scale);
    }
    else{
        sprintf(str,"%d",nivel);
        renderTextoCentradoBasico(renderer,f,str,suave,x+66,y,scale);
    }
    renderTextoCentradoBasico(renderer,f,"/10",branco,x+101,y,scale);
}


void drawScore(int score_game,SDL_Renderer * renderer , TTF_Font* f,int x , int y , float scale){
    char str[256];
    SDL_Color branco = {255, 255 , 255 , 255};
    SDL_Color red = {255, 0 , 0 , 255} , green = {0, 255, 0, 255};
    if(score_game<0){
        renderTextoCentradoBasico(renderer,f,"Score",branco,x,y,scale);
        sprintf(str,"%d",score_game);
        renderTextoCentradoBasico(renderer,f,str,red,x+77,y,scale);
    }
    else{
        renderTextoCentradoBasico(renderer,f,"Score",branco,x,y,scale);
        sprintf(str,"+%d",score_game);
        renderTextoCentradoBasico(renderer,f,str,green,x+73,y,scale);
    }
}


void drawTurns(CorPiece turno ,int turns_game,SDL_Renderer * renderer , TTF_Font* f,int x , int y , float scale){
    char str[256];
    SDL_Color branco = {255, 255 , 255 , 255} ,
              laranja = {255, 165 , 0 , 255} , azul = {0, 255 , 255 , 255};
    renderTextoCentradoBasico(renderer,f,"Turns",branco,x,y,scale);
    sprintf(str,"%d",turns_game);
    
    if(turno==brancas) renderTextoCentradoBasico(renderer,f,str,azul,x+77,y,scale);
    else renderTextoCentradoBasico(renderer,f,str,laranja,x+77,y,scale);
}



void desenhaCheck(GameStruct * game , GUISettings * settings){
    int type = (-1);
    if(game->estadoJogo.king_in_check[0]) type = 0;
    else if(game->estadoJogo.king_in_check[1]) type = 1;
    else return;

    int pos_tab = posTabuleiro(game->estadoJogo.tabuleirojogo[type][King]);

    int coluna = pos_tab % 8 , linha = pos_tab / 8;
    SDL_SetRenderDrawColor(settings->gameRenderer, 255 , 0 , 0, 150); 
    SDL_Rect check = {100*coluna+210, 1080 - (100 * linha + 240),100,100};
    SDL_RenderFillRect(settings->gameRenderer, &check);
}


void desenharPieceAttacks(GUISettings * settings , uint64_bit passant , uint64_bit attacks , uint64_bit cor_oposta){
    int counter = 0;
    uint64_bit casa_atual = 1ULL;
    while(attacks != 0){
        if(casa_atual & attacks){
            int linha = counter/8 , coluna = counter % 8;
            if(1ULL<<counter & (cor_oposta | passant)){
                filledCircleRGBA(settings->gameRenderer, 100*coluna+260, 1080 - (100 * linha + 192) , 40 , 0 , 0, 0, 150);
                aacircleRGBA(settings->gameRenderer, 100*coluna+260, 1080 - (100 * linha + 192) , 40 , 0 , 0, 0, 150);
            }
            else {
                filledCircleRGBA(settings->gameRenderer, 100*coluna+260, 1080 - (100 * linha + 192) , 10 , 0 , 0, 0, 150);
                aacircleRGBA(settings->gameRenderer, 100*coluna+260, 1080 - (100 * linha + 192) , 10 , 0 , 0, 0, 150);
            }
        }
        attacks = (attacks>>1);
        counter++;
    }
}


void desenharPiece(Pieces tipoPiece , int linha , int coluna , GUISettings * settings, int offset){
    SDL_Rect posicaoPeca = {100*coluna+210, 1080 - (100 * linha + 246),100,100};
    SDL_RenderCopy(settings->gameRenderer,settings->textures.chessPieces[tipoPiece + offset],NULL,&posicaoPeca);
}


void desenharPieceDrag(Pieces tipoPiece , int mouseX , int mouseY , GUISettings * settings , int offset)
{
    SDL_Rect centro = {mouseX-51,mouseY-51,100,100};
    SDL_RenderCopyEx(settings->gameRenderer,settings->textures.chessPieces[tipoPiece + offset], NULL, &centro, 0, NULL, SDL_FLIP_NONE);
}


void desenhaTipoPiece(uint64_bit pos_pieces,Pieces tipoPiece , GUISettings * settings, GameStruct * game, int offset){
    int counter=0 , linha , coluna;
    uint64_bit casaAtual=1ULL;
    while(pos_pieces!=0){
        if(casaAtual & pos_pieces){
            linha = counter/8; coluna = counter%8;
            Boolean is_selected_piece = game->pieceSelecionada == tipoPiece && game->pieceCoords == (1ULL<<counter);
            if( game->isKeyPressedDown && is_selected_piece && !game->promoted.pawnPromoted)
                desenharPieceDrag(tipoPiece,settings->posMouseX,settings->posMouseY,settings,offset);
            else
            desenharPiece(tipoPiece,linha,coluna,settings, offset);
        }
        pos_pieces = (pos_pieces>>1);
        counter++;
    }
}



void desenhaPromotion(GameStruct * game , GUISettings * settings){
    int pos_tab = posTabuleiro(game->promoted.promoted_square);
    int offsetY = ( (pos_tab / 8 ) < 1) ? 800 : 0,
        offsetX = pos_tab%8;
    int offset_textura = (game->turnoJogador == brancas) ? 0 : 6;
    SDL_Rect promotion_sq = {250 + 110*offsetX,100 + offsetY,175,175};
    SDL_RenderCopy(settings->gameRenderer,settings->textures.miscTextures[3],NULL,&promotion_sq);

    SDL_Rect queen = {255 + 110*offsetX,105 + offsetY,70,70};
    SDL_Rect rook = {345 + 110*offsetX,105 + offsetY,70,70};
    SDL_Rect bishop = {255 + 110*offsetX,190 + offsetY,70,70};
    SDL_Rect knight = {345 + 110*offsetX,190 + offsetY,70,70};

    SDL_RenderCopy(settings->gameRenderer,settings->textures.chessPieces[Queen + offset_textura],NULL,&queen);
    SDL_RenderCopy(settings->gameRenderer,settings->textures.chessPieces[Rook + offset_textura],NULL,&rook);
    SDL_RenderCopy(settings->gameRenderer,settings->textures.chessPieces[Bishop + offset_textura],NULL,&bishop);
    SDL_RenderCopy(settings->gameRenderer,settings->textures.chessPieces[Horse + offset_textura],NULL,&knight);
}



void desenhaFundo(GUISettings * settings , SDL_Texture * texture){
    SDL_Rect fundo = {0,0,1920,1080};
    SDL_RenderCopy(settings->gameRenderer,texture,NULL,&fundo);
}



void renderTextoCentradoSombra(SDL_Renderer* r, TTF_Font* f, const char* txt, SDL_Color cor, int x , int y, float escala){
    SDL_Surface *s = TTF_RenderText_Blended(f, txt, cor);
    SDL_Texture *tx = SDL_CreateTextureFromSurface(r, s);
    float w = s->w * escala, h = s->h * escala;
    SDL_FreeSurface(s);
    SDL_Rect sombra = {x - (int)(w / 2 + 5), y + 5, (int)w, (int)h};
    SDL_Rect rect = {x - (int)(w / 2), y, w, h};
    SDL_SetTextureColorMod(tx, 60, 45, 0);
    SDL_RenderCopy(r, tx, NULL, &sombra);
    SDL_SetTextureColorMod(tx, cor.r, cor.g, cor.b);
    SDL_RenderCopy(r, tx, NULL, &rect);
    SDL_DestroyTexture(tx);
}



void renderTextoCentradoBasico(SDL_Renderer* r, TTF_Font* f, const char* txt, SDL_Color cor, int x , int y, float escala){
    SDL_Surface *s = TTF_RenderText_Blended(f, txt, cor);
    SDL_Texture *tx = SDL_CreateTextureFromSurface(r, s);
    float w = s->w * escala, h = s->h * escala;
    SDL_FreeSurface(s);
    SDL_Rect rect = {x - (int)(w / 2), y, (int)w, (int)h};
    SDL_RenderCopy(r, tx, NULL, &rect);
    SDL_DestroyTexture(tx);
}





void drawSingleArrow(int (*vector)[2] , SDL_Renderer * renderer , SDL_Texture * arrow[2]){
    int pos_draw_inicial = (*vector)[0] , pos_draw_final = (*vector)[1];
    if(pos_draw_inicial > 64 || pos_draw_inicial < 0 || pos_draw_final > 64 || pos_draw_final < 0) return;
    int xi = pos_draw_inicial % 8 , xf = pos_draw_final % 8 ,
        yi = pos_draw_inicial / 8 , yf = pos_draw_final / 8;

    int centro_xi = 100*xi+260 , centro_xf = 100*xf + 260,
        centro_yi = 1080 - (100 * yi + 246) + 40, centro_yf = 1080 - (100 * yf + 246) + 40;

    double dx = centro_xf - centro_xi , dy = (-1)*(centro_yf - centro_yi);
    int tam_arrow = (int)(sqrt(dx*dx + dy*dy)) - 57,
        largura_arrow = 24;

    double angulo_rad = (-1) * atan2(dy, dx);
    double angulo_graus =  angulo_rad * (180.0 / M_PI) - 90;
    double angulo_graus_tip =  angulo_rad * (180.0 / M_PI) + 90;
    
    int offset = (dy > 0) ? 0 : 30;

    SDL_Rect arrow_rect = {
        .x = (int)(centro_xi - largura_arrow/2),
        .y = (int)(centro_yi) + offset,
        .w = largura_arrow,
        .h = tam_arrow
    };
    SDL_Rect arrow_tip = {
        .x = (int)(centro_xf - 30) ,
        .y = (int)(centro_yf) + offset,
        .w = 60 , 
        .h = 60
    };


    SDL_Point pivot1 = {largura_arrow / 2,0},
              pivot2 = {30,0};
    SDL_RenderCopyEx(renderer , arrow[0] , NULL, &arrow_rect , angulo_graus , &pivot1 , SDL_FLIP_NONE);
    SDL_RenderCopyEx(renderer , arrow[1] , NULL, &arrow_tip , angulo_graus_tip , &pivot2 , SDL_FLIP_NONE);
}


void desenhaArrows(GameStruct * game , SDL_Renderer * renderer , SDL_Texture * generic_orange[2]){
    ArrowsGame arrows = game->arrows;
    int indx = arrows.indx_drawable_arrows;
    if(arrows.arrows_vector == NULL) return;
    for(int i=0;i<indx;i++){
        int (*vector_atual)[2] = (arrows.arrows_vector + i);
        drawSingleArrow(vector_atual, renderer , generic_orange);
    }
}