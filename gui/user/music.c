#include <SDL2/SDL_mixer.h>



void movepiece_sfx (Mix_Chunk * sfxarray[]){
   Mix_Chunk * move = sfxarray[0];
   Mix_VolumeChunk(move, 128);
   Mix_PlayChannel(1, move, 0);
}
void capturepiece_sfx (Mix_Chunk * sfxarray[]){
   Mix_Chunk * capture = sfxarray[1];
   Mix_VolumeChunk(capture, 128);
   Mix_PlayChannel(1, capture, 0);
}
void promote_sfx (Mix_Chunk * sfxarray[]){
   Mix_Chunk * promote = sfxarray[2];
   Mix_VolumeChunk(promote, 128);
   Mix_PlayChannel(1, promote, 0);
}
void check_sfx (Mix_Chunk * sfxarray[]){
   Mix_Chunk * check = sfxarray[3];
   Mix_VolumeChunk(check, 128);
   Mix_PlayChannel(1, check, 0);
}
void checkmate_sfx (Mix_Chunk * sfxarray[]){
   Mix_Chunk * checkmate = sfxarray[4];
   Mix_VolumeChunk(checkmate, 128);
   Mix_PlayChannel(1, checkmate, 0);
}