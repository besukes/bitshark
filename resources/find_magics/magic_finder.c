#include <engine/chess_lib/engine.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static uint64_bit chess_border = 0xFF818181818181FFULL;

//I want to find my own magic numbers , but for now i will use some that are not mine , for the sake of it 
//When i get better i do intend to get back to this and try it for myself and perhaps even use my own!



uint64_t random_uint64(void) {
    srand(time(NULL));
    uint64_t r = 0;
    // Assumes RAND_MAX >= 32767 (at least 15 bits)
    // Combines 5 calls to rand() to cover 64 bits
    for (int i = 0; i < 5; i++) {
        r = (r << 15) | (rand() & 0x7FFF);
    }
    return r;
}



uint64_bit generate_random_magic(void){
    uint64_bit magic = random_uint64() & random_uint64() & random_uint64();
    return magic;
}


int generate_occupancies(uint64_bit mask , uint64_bit * occupancies){
    uint64_bit occ = 0ULL;
    int count = 0;
    // Cycle through all subsets of the mask
    do {
        occupancies[count++] = occ;
        // Carry-Rippler trick to get the next subset of 'mask'
        occ = (occ - mask) & mask;
    } while (occ != 0ULL);

    return count;
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


uint64_bit attempt_find_magic(Pieces piece , casas_board casa){
    uint64_bit used_attacks[4096];
    uint64_bit square_occupancies[4096] = {0};
    uint64_bit relevant_mask_sq = generate_mask(piece,casa);
    int loop_indx = generate_occupancies(relevant_mask_sq,square_occupancies);
    uint64_bit magic = generate_random_magic();
    for(int i=0;i<1000000;i++){
        if(count_bits((relevant_mask_sq * magic) & 0xFF00000000000000ULL) < 6) continue;

    }
    return (0ULL);
}
