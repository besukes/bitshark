#include "engine/chess_lib/engine.h"
#include <stdint.h>


void get_attacks(int max , uint64_bit (*func)(uint64_bit,int),uint64_bit pos_limites,uint64_bit pos_piece,int shift ,uint64_bit * atk){
    *atk = 0;
    for(int i=1;i<=max ;i++){
        uint64_bit casa_atual = func(pos_piece,shift*i);
        *(atk) |= casa_atual;
        if(pos_limites & casa_atual) break;
    }
}



uint64_bit get_cross_attacks(uint64_bit piece_pos , uint64_bit pos_limites){
    int indx_tab = posTabuleiro(piece_pos);
    int linha = indx_tab/8 , coluna = indx_tab % 8,
        maxDistNorte = 7-linha,
        maxDistSul= linha,
        maxDistOeste = coluna,
        maxDistEste = 7-coluna;
    uint64_bit atkN,atkS,atkO,atkE ,atk= 0;
    get_attacks(maxDistNorte,&shiftl,pos_limites,piece_pos,8,&atkN);
    get_attacks(maxDistSul,&shiftr,pos_limites,piece_pos,8,&atkS);
    get_attacks(maxDistOeste,&shiftr,pos_limites,piece_pos,1,&atkO);
    get_attacks(maxDistEste,&shiftl,pos_limites,piece_pos,1,&atkE);
    atk = atkN | atkS | atkO | atkE;
    return atk;
}



uint64_bit get_sliding_attacks(uint64_bit piece_pos, uint64_bit pos_limites){
    int indx_tab = posTabuleiro(piece_pos);
    int linha = indx_tab/8 , coluna = indx_tab % 8,
        maxDistNordeste = minimum(7-linha,7-coluna),
        maxDistNoroeste = minimum(7-linha,coluna),
        maxDistSudeste = minimum(linha,7-coluna),
        maxDistSudoeste = minimum(linha,coluna);
    uint64_bit atkNo,atkNe,atkSude ,atkSudo,atk;
    get_attacks(maxDistNordeste,&shiftl,pos_limites,piece_pos,9,&atkNe);
    get_attacks(maxDistNoroeste,&shiftl,pos_limites,piece_pos,7,&atkNo);
    get_attacks(maxDistSudeste,&shiftr,pos_limites,piece_pos,7,&atkSude);
    get_attacks(maxDistSudoeste,&shiftr,pos_limites,piece_pos,9,&atkSudo);
    atk = atkNe | atkNo | atkSude | atkSudo;
    return atk;
}



uint64_bit get_pawn_attacks(uint64_bit piece_pos,CorPiece cor){
    uint64_bit at = 0 , colunaA = 0 , colunaH = 0;
    getColunasAH(&colunaA,&colunaH);
    if(cor==brancas){
        at |= ((piece_pos<<9) & ~colunaA);
        at |= ((piece_pos<<7) & ~colunaH);
    }
    else{
        at |= ((piece_pos>>9) & ~colunaH);
        at |= ((piece_pos>>7) & ~colunaA);
    }
    return at;
}



uint64_bit get_knight_attacks(uint64_bit piece_pos){
    uint64_bit at = 0 , colunaA = COLUNA_A , colunaH = COLUNA_H,
               colunaB = COLUNA_B , colunaG = COLUNA_G;
    at |= ((piece_pos & ~colunaA & ~colunaB )<<6);
    at |= ((piece_pos & ~colunaA)<<15);
    at |= ((piece_pos & ~colunaH & ~colunaG )<<10);
    at |= ((piece_pos & ~colunaH)<<17);
    at |= ((piece_pos & ~colunaG & ~colunaH)>>6);
    at |= ((piece_pos& ~colunaH)>>15);
    at |= ((piece_pos & ~colunaA & ~colunaB)>>10);
    at |= ((piece_pos& ~colunaA)>>17);
    return at;
}


void king_line_dependant_moves(uint64_bit * atk ,uint64_bit (*func)(uint64_bit,int),uint64_bit pos_rei , uint64_bit colunaA , uint64_bit colunaH){
    uint64_bit coluna1,coluna2;
    if(func==&shiftl){
        coluna1 = colunaA;
        coluna2 = colunaH;
    }else{
        coluna1 = colunaH;
        coluna2 = colunaA;
    }
    *atk |= func(pos_rei,8);
    *atk |= func(pos_rei&~coluna1,7);
    *atk |= func(pos_rei&~coluna2,9);
}


uint64_bit get_castle_moves(uint64_bit pos , uint64_bit bb_pieces , GameStruct * game , CorPiece turn){
    uint64_bit castle = 0;
    uint64_bit short_castle = (turn==brancas) ? (1ULL<<6) : (1ULL<<62);
    uint64_bit long_castle = (turn==brancas) ? (1ULL<<2) : (1ULL << 58);

    if(is_castelling_king(game,turn,short_castle)) (castle|=short_castle);
    if(is_castelling_king(game,turn,long_castle))  (castle|=long_castle);

    return castle;
}


uint64_bit get_king_moves(uint64_bit pos){
    int posTab = posTabuleiro(pos);
    uint64_bit at = 0 , colunaA = COLUNA_A , colunaH = COLUNA_H;
    if(0<=posTab && posTab<8) king_line_dependant_moves(&at,&shiftl,pos,colunaA,colunaH);
    else if(56<=posTab && posTab<64) king_line_dependant_moves(&at,&shiftr,pos,colunaA,colunaH);
    else {
        king_line_dependant_moves(&at,&shiftl,pos,colunaA,colunaH);
        king_line_dependant_moves(&at,&shiftr,pos,colunaA,colunaH);
    }
    at |= ((pos & ~colunaA)>>1);
    at |= ((pos & ~colunaH)<<1);
    return at;
}


uint64_bit get_enpassant_move(GameStruct * game ,uint64_bit pos , uint64_bit pos_enpassant , CorPiece turn){
    Jogada jogada = {.destino = posTabuleiro(pos_enpassant) , .origem = posTabuleiro(pos) , .peca_movida = Pawn};
    if(can_en_passant(game,&jogada,turn)) return pos_enpassant;
    return 0;
}


uint64_bit get_possible_pawn_moves(uint64_bit pos,uint64_bit bitboard_pieces,CorPiece turno,uint64_bit (*func)(uint64_bit,int),GameStruct * game){
    uint64_bit oposto = (turno==brancas) ? game->estadoJogo.bitboard_pretas : game->estadoJogo.bitboard_brancas;
    uint64_bit fst_step = func(pos,8) & ~bitboard_pieces;
    uint64_bit pawn_attacks = get_pawn_attacks(pos,turno);
    if(pawnFirstRank(pos,turno)){
        uint64_bit snd_step = (fst_step) ? ( func(pos,16) & ~bitboard_pieces ) : 0;
        return ( fst_step | snd_step | ( pawn_attacks & oposto));
    }
    else{
        return ( fst_step | ( pawn_attacks & oposto) | get_enpassant_move(game,pos,game->estadoJogo.enpassant,turno));
    }
}



uint64_bit get_piece_attacks(uint64_bit pos,Pieces piece,GameStruct * game , CorPiece cor_turno , int only_captures){
    uint64_bit bitboardPieces = game->estadoJogo.bitboard_todas_pieces;
    switch(piece){
        case Pawn :
            uint64_bit (*func)(uint64_bit,int) = (cor_turno==brancas) ? &shiftl : &shiftr;
            return get_possible_pawn_moves(pos,bitboardPieces,cor_turno,func,game);
        break;
        case Rook :
            return get_cross_attacks(pos,bitboardPieces);
        break;
        case Horse :
            return get_knight_attacks(pos);
        break;
        case Bishop :
            return get_sliding_attacks(pos,bitboardPieces);
        break;
        case Queen :
            return (get_sliding_attacks(pos,bitboardPieces) | get_cross_attacks(pos,bitboardPieces));
        break;
        case King :
            return ((only_captures) ? get_king_moves(pos) : (get_king_moves(pos)| get_castle_moves(pos,bitboardPieces,game,cor_turno)));
        break;
        default :
            return 0ULL;
        break;
    }
}

int isPromotionRank(uint64_bit pos_piece , CorPiece turn){
    int postab = posTabuleiro(pos_piece);
    if(turn == brancas && 48 <= postab && postab<  56) return 1;
    else if(turn == pretas && 8 <= postab && postab < 16) return 1;
    return 0;
}

int pawnPromoting(uint64_bit pos,CorPiece cor){
   int posTab = posTabuleiro(pos);
   if(cor==brancas && posTab>=56 && posTab<64) return 1;
   else if(cor==pretas && posTab>=0 && posTab<8) return 1;
   else return 0;
}


int isPseudoValidMove(GameStruct * game, Jogada * jogada , CorPiece cor , uint64_bit pos_attacks){
    if(jogada->peca_movida == Empty || jogada->destino == 64 || jogada->origem == 64) return 0;
    uint64_bit pos_dest = 1ULL<<jogada->destino;
    Pieces piece = jogada->peca_movida;
    uint64_bit pos_mesma_cor = get_same_colour_bitboard(&(game->estadoJogo),cor);
    uint64_bit move = (~pos_mesma_cor & (pos_attacks & pos_dest));
    jogada->especial = piece == King  && is_castelling_king(game,cor,pos_dest);
    if(jogada->especial) jogada->especial = FLAG_CASTLE;
    else jogada->especial = 0;
    int tries_to_enpassant = (piece == Pawn) && (pos_dest & game->estadoJogo.enpassant);
    int can_enpassant = can_en_passant(game,jogada,cor);
    if(tries_to_enpassant){
        if(can_enpassant){
            jogada->especial = FLAG_ENPASSANT;
            jogada->peca_capturada = Pawn;
        }
        else return 0;
    }
    return (move != 0 || jogada->especial == FLAG_CASTLE || jogada->especial == FLAG_ENPASSANT);
}

void adicionaPromotion(Jogada* jogadas , int indx , Pieces piece , Jogada j){
    j.promocao = piece;
    jogadas[indx] = j;
}

int gerar_jogadas_legais(GameStruct* game, Jogada* jogadas , CorPiece cor , int only_captures){
    uint64_bit occupied = game->estadoJogo.bitboard_todas_pieces;
    uint64_bit * oposto = (cor==brancas) ? &(game->estadoJogo.bitboard_pretas) : &(game->estadoJogo.bitboard_brancas);
    CorPiece op_cor = (cor==brancas) ? pretas : brancas;
    int num_jogadas = 0;
    uint64_bit enpassant_pos = game->estadoJogo.enpassant;
    int canCastleShortB = game->estadoJogo.canCastle[brancas][Short];
    int canCastleLongB = game->estadoJogo.canCastle[brancas][Long];
    int canCastleShortP = game->estadoJogo.canCastle[pretas][Short];
    int canCastleLongP = game->estadoJogo.canCastle[pretas][Long];
    for (int i = NUMBER_PIECES - 1; i >= 0; i--) {
        Pieces piece = (Pieces)i;   
        uint64_bit bitboard = game->estadoJogo.tabuleirojogo[cor][piece];
        while (bitboard) {
            uint64_bit single_piece = bitboard & (-bitboard); // Isola o bit mais baixo
            uint64_bit attacks = get_magic_piece_attacks(single_piece, piece,game, cor,only_captures,occupied);
            if(only_captures) attacks &= *oposto; // Se for apenas capturas, filtra os ataques para incluir apenas as peças do oponente{
            while (attacks) {
                uint64_bit single_attack = attacks & (-attacks);
                Pieces p_cap = comparePiece(&game->estadoJogo,op_cor,single_attack);
                Jogada jogada = {.origem = (uint8_t)posTabuleiro(single_piece), .destino = (uint8_t)posTabuleiro(single_attack), .peca_movida = piece, 
                                .peca_capturada = p_cap, .promocao = 0, .especial = 0 , .score = 0 , 
                                .prev_castlerights[brancas][Short] = canCastleShortB , .prev_castlerights[brancas][Long] = canCastleLongB ,
                                .prev_castlerights[pretas][Short] = canCastleShortP , .prev_castlerights[pretas][Long] = canCastleLongP ,
                                .prev_enpassant = posTabuleiro(enpassant_pos)};
                int is_promoting = (piece == Pawn) && isPromotionRank(single_piece,cor) && pawnPromoting(single_attack,cor);
                Boolean flags_are_respected = !only_captures || jogada.peca_capturada != Empty;
                if (flags_are_respected && isPseudoValidMove(game, &jogada, cor , single_attack)){
                    if(is_promoting){
                        adicionaPromotion(jogadas,num_jogadas,Queen,jogada);
                        num_jogadas++;
                        adicionaPromotion(jogadas,num_jogadas,Horse,jogada);
                        num_jogadas++;
                        adicionaPromotion(jogadas,num_jogadas,Rook,jogada);
                        num_jogadas++;
                        adicionaPromotion(jogadas,num_jogadas,Bishop,jogada);
                        num_jogadas++;
                    }
                    else jogadas[num_jogadas++] = jogada;
                }
                attacks &= attacks - 1; // Remove esse bit
            }
            bitboard &= bitboard - 1; // Remove esse bit
        }
    }
    return num_jogadas;
}


int jogo_terminou(EstadoJogo *estado) {
    if (estado->checkMate || estado->stalemate) {
        return 1; // O jogo terminou
    }
    return 0; // O jogo não terminou
}



int is_attacked_square(uint64_bit pos , CorPiece turn , GameStruct * game , uint64_bit occupancy){
    for(int i=0;i<NUMBER_PIECES;i++){
        uint64_bit tab = game->estadoJogo.tabuleirojogo[turn][i];
        while(tab){
            uint64_bit single = tab & (-tab); // isola o bit mais baixo
            uint64_bit atks = (i == Pawn) ? get_pawn_attacks(single,turn)
                                          : get_magic_piece_attacks(single,(Pieces)i,game,turn,1,occupancy);
            if((atks&pos) != 0) return 1;
            tab &= tab - 1; // remove esse bit
        }
    }
    return 0;
}