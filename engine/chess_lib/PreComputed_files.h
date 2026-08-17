
#define COLUNA_A 0x0101010101010101ULL
#define COLUNA_B 0x0202020202020202ULL
#define COLUNA_C 0x0404040404040404ULL
#define COLUNA_D 0x0808080808080808ULL
#define COLUNA_E 0x1010101010101010ULL
#define COLUNA_F 0x2020202020202020ULL
#define COLUNA_G 0x4040404040404040ULL
#define COLUNA_H 0x8080808080808080ULL


static const unsigned long long rook_files[] = {
    COLUNA_A,COLUNA_B,
    COLUNA_C,COLUNA_D,
    COLUNA_E,COLUNA_F,
    COLUNA_G,COLUNA_H
};


static const int passed_pawn_bonus[] = {
    0, // Linha 0
    5, // Linha 1
    10, // Linha 2
    15, // Linha 3
    30, // Linha 4
    70, // Linha 5
    150, // Linha 6
    200, // Linha 7 (em teoria nunca chega , mas para evitar bugs)
};