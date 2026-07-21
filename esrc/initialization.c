#include <engine.h>


void initializeStructs(PieceEvaluation array[2][NUMBER_PIECES],int indx){
    for(int i=0;i<indx;i++){
        array[0][i].eval = 0;
        array[1][i].eval = 0;
    }
}