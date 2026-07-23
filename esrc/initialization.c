#include <engine.h>


void initializeStructs(int matrix[2][NUMBER_PIECES],int indx){
    for(int i=0;i<indx;i++){
        matrix[0][i] = 0;
        matrix[1][i] = 0;
    }
}