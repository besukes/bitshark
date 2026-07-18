#include <engine.h>


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
    uint64_bit at = 0 , colunaA = 0 , colunaH = 0 ,
               colunaB = 0 , colunaG = 0;
    for(int i=0;i<8;i++){
        colunaA |= (1ULL<< (8*i));
        colunaB |= (1ULL<< (8*i + 1));
        colunaH |= (1ULL<< (8*i + 7));
        colunaG |= (1ULL<< (8*i + 6));
    }
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


uint64_bit get_king_moves(uint64_bit pos){
    int posTab = posTabuleiro(pos);
    uint64_bit at = 0 , colunaA = 0 , colunaH = 0;
    getColunasAH(&colunaA,&colunaH);
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



uint64_bit get_possible_pawn_moves(uint64_bit pos,uint64_bit bitboard_pieces,CorPiece turno,uint64_bit (*func)(uint64_bit,int),GameStruct * game){
    uint64_bit oposto = (turno==brancas) ? game->estadoJogo.bitboard_pretas : game->estadoJogo.bitboard_brancas;
    uint64_bit fst_step = func(pos,8) & ~bitboard_pieces;
    if(pawnFirstRank(pos,turno)){
        uint64_bit snd_step = (fst_step) ? ( func(pos,16) & ~bitboard_pieces ) : 0;
        return ( fst_step | snd_step | ( get_pawn_attacks(pos,turno) & oposto) );
    }
    else{
        return ( fst_step | ( get_pawn_attacks(pos,turno) & oposto) );
    }
}



uint64_bit get_piece_attacks(uint64_bit pos,Pieces piece,GameStruct * game , CorPiece cor_turno){
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
            return get_king_moves(pos);
        break;
        default :
            return 0ULL;
        break;
    }
}


int pawnPromoting(uint64_bit pos,CorPiece cor){
   int posTab = posTabuleiro(pos);
   if(cor==brancas && posTab>=56 && posTab<64) return 1;
   else if(cor==pretas && posTab>=0 && posTab<8) return 1;
   else return 0;
}


int isPseudoValidMove(GameStruct * game, uint64_bit drop , Boolean * castle , Boolean * enpassant , Boolean * promote){
    Pieces piece = game->pieceSelecionada;
    CorPiece cor = game->turnoJogador;
    uint64_bit pos_piece = game->pieceCoords,
               pos_atacks = get_piece_attacks(pos_piece,piece,game,cor),
               pos_mesma_cor = get_same_colour_bitboard(&(game->estadoJogo),cor);
    uint64_bit jogada = (~pos_mesma_cor & (pos_atacks & drop));
    *promote = ( (jogada != 0) && (piece == Pawn) && pawnPromoting(drop,cor) );
    *castle = game->pieceSelecionada == King  && is_castelling_king(game,cor,drop);
    *enpassant = can_en_passant(game,drop,cor);
    return (jogada != 0 || *castle || *enpassant);
}
