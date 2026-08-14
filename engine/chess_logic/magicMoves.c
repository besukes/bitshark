#include <engine/chess_lib/engine.h>
#include <engine/chess_lib/PreComputed_magics.h>

// Max sizes for lookup tables
#define MAX_ROOK_ATTACKS 4096 // 2^12
#define MAX_BISHOP_ATTACKS 512 // 2^9

// Precomputed tables stored in memory
uint64_bit rook_attack_table[64][MAX_ROOK_ATTACKS];
uint64_bit bishop_attack_table[64][MAX_BISHOP_ATTACKS];
uint64_bit knight_attack_table[64];

// Arrays holding your precomputed magic numbers and masks for each square (0..63)
uint64_bit rook_masks[64];
uint64_bit bishop_masks[64];

const uint64_bit chess_border = 0xFF818181818181FFULL;



void init_sliding_attack_table(void) {
    for (int sq = 0; sq < 64; sq++) {
        uint64_bit mask = bishop_masks[sq];
        uint64_bit occ = 0ULL;
        // Loop through all 2^n occupancy subsets (Carry-Rippler)
        do {
            // 1. Calculate actual attacks using classical ray-casting
            uint64_bit attacks = (get_sliding_attacks)(1ULL<<sq,occ);
            // 2. Compute the magic index for this specific occupancy
            int index = (int)((occ * BishopMagics[sq]) >> BishopShifts[sq]);
            // 3. Store the attack bitboard into the lookup table
            bishop_attack_table[sq][index] = attacks;
            // Next occupancy subset
            occ = (occ - mask) & mask;
        } while (occ != 0ULL);
    }
}


void init_cross_attack_table(void) {
    for (int sq = 0; sq < 64; sq++) {
        uint64_bit mask = rook_masks[sq];
        uint64_bit occ = 0ULL;
        // Loop through all 2^n occupancy subsets (Carry-Rippler)
        do {
            // 1. Calculate actual attacks using classical ray-casting
            uint64_bit attacks = (get_cross_attacks)(1ULL<<sq,occ);
            // 2. Compute the magic index for this specific occupancy
            int index = (int)((occ * RookMagics[sq]) >> RookShifts[sq]);
            // 3. Store the attack bitboard into the lookup table
            rook_attack_table[sq][index] = attacks;
            // Next occupancy subset
            occ = (occ - mask) & mask;
        } while (occ != 0ULL);
    }
}


void init_Knight_table(void){
    for(int i=0;i<64;i++){
        knight_attack_table[i] = get_knight_attacks(1ULL<<i);
    }
}


void init_AttacksLookUpTable(void){
    init_cross_attack_table();
    init_sliding_attack_table();
    init_Knight_table();
}


uint64_bit generate_mask(Pieces piece , casas_board sq){
    if(piece == Rook){
        uint64_bit possible_moves = get_cross_attacks(1ULL<<sq,0);
        return (possible_moves & ~chess_border);
    }
    else if(piece == Bishop){
        uint64_bit possible_moves = get_sliding_attacks(1ULL<<sq,0);
        return (possible_moves & ~chess_border);
    }
    printf("No mask generated...\n");
    return 0;
}


void init_SlidingNdCross_Masks(void){
    for(int i=0;i<64;i++){
        uint64_bit mask = generate_mask(Rook,i);
        rook_masks[i] = mask;
    }
    for(int i=0;i<64;i++){
        uint64_bit mask = generate_mask(Bishop,i);
        bishop_masks[i] = mask;
    }
}


uint64_bit get_magic_cross_attacks(int square, uint64_bit occupancy){
    uint64_bit occ = occupancy & rook_masks[square];
    int index = (occ * RookMagics[square]) >> (RookShifts[square]);
    return (rook_attack_table[square][index]);
}


uint64_bit get_magic_sliding_attacks(int square , uint64_bit occupancy){
    uint64_bit occ = occupancy & bishop_masks[square];
    int index = (occ * BishopMagics[square]) >> (BishopShifts[square]);
    return (bishop_attack_table[square][index]);
}



uint64_bit get_magic_piece_attacks(uint64_bit pos,Pieces piece,GameStruct * game , CorPiece cor_turno , int only_captures){
    uint64_bit bitboardPieces = game->estadoJogo.bitboard_todas_pieces;
    int pos_tab = posTabuleiro(pos);
    if(pos_tab == (-1)) return 0;
    switch(piece){
        case Pawn :
            uint64_bit (*func)(uint64_bit,int) = (cor_turno==brancas) ? &shiftl : &shiftr;
            return (get_possible_pawn_moves(pos,bitboardPieces,cor_turno,func,game));
        break;
        case Rook :
            return (get_magic_cross_attacks(pos_tab,bitboardPieces));
        break;
        case Horse :
            return (knight_attack_table[pos_tab]);
        break;
        case Bishop :
            return (get_magic_sliding_attacks(pos_tab,bitboardPieces));
        break;
        case Queen :
            return (get_magic_sliding_attacks(pos_tab,bitboardPieces) | get_magic_cross_attacks(pos_tab,bitboardPieces));
        break;
        case King :
            return ((only_captures) ? get_king_moves(pos) : (get_king_moves(pos)| get_castle_moves(pos,bitboardPieces,game,cor_turno)));
        break;
        default :
            return 0ULL;
        break;
    }
}


void initMagicMoveGeneration(void){
    init_SlidingNdCross_Masks();
    init_AttacksLookUpTable();
}