#define CHESS_AI_DIFFICULTY 5
#define MAX_DEPTH_SEARCH 30
#define NUMBER_PIECES 6


typedef struct Moves{
    uint64_bit move;
    int move_evaluation;
}Moves;

typedef struct PieceEvaluation{
    int eval;
}PieceEvaluation;

typedef struct SearchInfo{
    int depth;
    int alpha;
    int beta;
    int ai_level;
    Pieces piece_type;
    CorPiece bot_colour;
    CorPiece turn;
}SearchInfo;


typedef int Boolean; //Forma mais intuitiva de perceber quando as variáveis são usadas como valores lógicos.

/*Struct que define a posicao de uma peca no tabuleiro de xadrez usando um long 64 bit*/
typedef unsigned long long uint64_bit;

typedef uint64_bit (*ShiftFunction)(uint64_bit,int); //Tipo que define um endereço de memória de uma função que recebe um unsigned long long de 64 bits e um int normal

/*Enum que guarda todas as peças possíveis de xadrez*/
typedef enum {
    Pawn, //The piece is a PAWN
    Rook, //The piece is the ROOK
    Horse, //The piece is the HORSE
    Bishop, //The piece is the BISHOP
    Queen, //The piece is the QUEEN
    King, //The piece is the KING
    Empty, //There is no piece
} Pieces;


/*Bitboard que cada indice representa o numero de left shifts que temos de fazer para chegar a essa posição no tabuleiro */
typedef enum casas_board{
    A1,B1,C1,D1,E1,F1,G1,H1,
    A2,B2,C2,D2,E2,F2,G2,H2,
    A3,B3,C3,D3,E3,F3,G3,H3,
    A4,B4,C4,D4,E4,F4,G4,H4,
    A5,B5,C5,D5,E5,F5,G5,H5,
    A6,B6,C6,D6,E6,F6,G6,H6,
    A7,B7,C7,D7,E7,F7,G7,H7,
    A8,B8,C8,D8,E8,F8,G8,H8
} casas_board;

//Representa a cor de uma dada peça
typedef enum { brancas , pretas } CorPiece;

//Representa os diferentes castle types
typedef enum { Short , Long } CastleTypes;

/*Struct que define um estado de um jogo de xadrez.
Guarda as posições de todas as peças , bem como as posições afetadas por elas.
Guarda também informações sobre se um dado king está em check/checkmate ou não*/
typedef struct EstadoJogo{
    int checkMate; //Informa se um rei está em checkmate (game over)
    int stalemate; //Informa se o jogo acabou em staleMate
    int king_in_check[2]; //Informa se o reis estão em check
    int canCastle[2][2]; //Matriz de possibilidades de dar castle
    uint64_bit enpassant; //Guarda a posição possível de se fazer enpassant
    uint64_bit tabuleirojogo[2][6]; //Guarda as informações do tabuleiro das peças regulares de xadrez
    uint64_bit bitboard_brancas; // Bitboard das peças brancas
    uint64_bit bitboard_pretas; // Bitboard das peças pretas
    uint64_bit bitboard_todas_pieces; //Bitboard que guarda as posições ocupadas por todas as peças no jogo
}EstadoJogo;


/*Linked List que guarda a piece que foi comida na jogada anterior , para depois desfazer a jogada , caso seja necessário*/
typedef struct PecasComidas{
    uint64_bit pos_de_piece;
    Pieces tipo_piece;
    CorPiece cor_piece;
    struct PecasComidas * prox;
} * PecasComidasLL;

/*Struct responsável por guardar as informações sobre os peões se estão ou não a ser promovidos , se o peão foi promovido de forma correta
e o quadrado onde a promoção está a acontecer*/
typedef struct PromotedInformation{
    uint64_bit promoted_square; //Guarda o promotion square do pawn promovido
    Boolean pawnPromoted; //Guarda se o peão está a ser promovido
    Boolean promotedSucessfully; //Guarda se o peão foi promovido num clique
}PromotedInformation;

/*Guarda a quantidade de material que cada lado tem durante o jogo*/
typedef struct SidesMaterial{
    int white_material; //Quantidade de material das brancas
    int black_material; //Quantidade de material das pretas
}SidesMaterial;

/*Struct que guarda o estado do jogo , tal como o turno do jogador , a peça que está a ser segurada(caso esteja a ser premida a tecla ,
que é da responsabilidade do bool isKeyPressedDown) e também a jogada do utilizador */
typedef struct GameStruct{
    EstadoJogo estadoJogo; //Estado atual do jogo
    Boolean isKeyPressedDown; //Verifica se o utilizador está a premir a tecla
    Pieces pieceSelecionada; //Guarda a peça que o utilizador está a ser segurada , caso esteja
    uint64_bit pieceCoords; //Guarda a posição de onde a peça que está a ser segurada veio , caso esteja
    CorPiece turnoJogador; //Guarda o turno do utilizador 
    PecasComidasLL lastmoves; //Guarda a peça que foi comida na jogada anterior , para depois desfazer a jogada , caso seja necessário
    PromotedInformation promoted; //Guarda informações sobre se uma promotion aconteceu e onde ela ocorreu
}GameStruct;

/*Struct que serve como uma lookup table das melhores posições de uma dada peça de uma dada cor no tabuleiro de xadrez.
Para este projeto a "colour_key" será apenas utilizada a cor preta e as lookup tables de todas as peças pretas*/
typedef struct LookupTable{
    CorPiece colour_key;
    Pieces piece_key;
    int evaluation_multiplier[64];
} * ChessLT;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int piece_value(Pieces piece){
    switch(piece){
        case 0: return 100;
        case 1: return 500;
        case 2: return 320;
        case 3: return 330;
        case 4: return 900;
        case 5: return 20000;
        default: 
            printf("[ERROR] Pieces index out of range\n");
            return 0;
        break;
    }
}

uint64_bit shiftr(uint64_bit pos,int shift){return (pos>>shift);}

uint64_bit shiftl(uint64_bit pos,int shift){return (pos<<shift);}

int minimum(int n1,int n2){return ((n1<n2)? n1 : n2);}



/// aux_engine ///////////////////////////////

int pawnFirstRank(uint64_bit pos,CorPiece cor);

uint64_bit get_same_colour_bitboard(EstadoJogo * estado , CorPiece cor);

uint64_bit get_opposing_colour_bitboard(EstadoJogo * estado , CorPiece cor);

Pieces comparePiece(EstadoJogo estado , CorPiece cor , uint64_bit posclique);

int posTabuleiro(uint64_bit bitboard);

void getColunasAH(uint64_bit * colunaA , uint64_bit * colunaH);


/// applymoves /////////////////////////////

void promotePiece(GameStruct * game , Pieces piece, uint64_bit promotion_square);

void efetuaJogada(uint64_bit * selected_piece , uint64_bit * todas_pieces , uint64_bit original_coords , uint64_bit click , uint64_bit * mesmacor);

void fetch_change_board(GameStruct * game,uint64_bit click,uint64_bit * mesmaCor , uint64_bit * corOposta);

void checkTurno(CorPiece turno , uint64_bit * * oposta , uint64_bit * * mesma_cor,int * sq , GameStruct * game , uint64_bit (**ep)(uint64_bit,int));

void atualizaJogada(GameStruct * game , uint64_bit click,Boolean castles,Boolean enpassant);


/// castle_logic ////////////////////////////

int is_open_castle_path(uint64_bit bitboard_todas_pieces,uint64_bit path , uint64_bit extraPositions);

int is_castelling_king(GameStruct * game , CorPiece cor, uint64_bit drop);

int invalidCastle(GameStruct * game , uint64_bit click);

void verifica_direito_castle(GameStruct * game ,CorPiece turno);

void castle_King(GameStruct * game , uint64_bit click , int square, uint64_bit * mesmaCor);



/// checkmate /////////////////////////////

Boolean is_in_check(EstadoJogo * estado , uint64_bit kingpos , CorPiece cor);
void notInCheck(GameStruct * game);
int isCheckMate(GameStruct * game , CorPiece cor);
int check_move(GameStruct * game, Boolean castles , uint64_bit click);



/// en_passant /////////////////////////////

void update_en_passant(GameStruct * game , uint64_bit click);
Boolean can_en_passant(GameStruct * game , uint64_bit drop,CorPiece cor);
void enpassant_move(GameStruct * game , uint64_bit * cor_oposta , uint64_bit * mesma_cor,ShiftFunction ep_shift);


/// possibleMoves.h /////////////////////////////

void get_attacks(int max , uint64_bit (*func)(uint64_bit,int),uint64_bit pos_limites,uint64_bit pos_piece,int shift ,uint64_bit * atk);
uint64_bit get_cross_attacks(uint64_bit piece_pos , uint64_bit pos_limites);
uint64_bit get_sliding_attacks(uint64_bit piece_pos, uint64_bit pos_limites);
uint64_bit get_pawn_attacks(uint64_bit piece_pos,CorPiece cor);
uint64_bit get_knight_attacks(uint64_bit piece_pos);
void king_line_dependant_moves(uint64_bit * atk ,uint64_bit (*func)(uint64_bit,int),uint64_bit pos_rei , uint64_bit colunaA , uint64_bit colunaH);
uint64_bit get_king_moves(uint64_bit pos);
uint64_bit get_possible_pawn_moves(uint64_bit pos,uint64_bit bitboard_pieces,CorPiece turno,uint64_bit (*func)(uint64_bit,int),GameStruct * game);
uint64_bit get_piece_attacks(uint64_bit pos,Pieces piece,GameStruct * game , CorPiece cor_turno);
int pawnPromoting(uint64_bit pos,CorPiece cor);
int isPseudoValidMove(GameStruct * game, uint64_bit drop , Boolean * castle , Boolean * enpassant , Boolean * promote);


/// initialization ////////////////////////////

void initializeMovesStruct(PieceEvaluation array2[2][NUMBER_PIECES],int indx);


/// depth_search /////////////////////////////

Moves search_algorithm (uint64_bit atks ,uint64_bit pos, GameStruct * game ,PieceEvaluation evals[2][NUMBER_PIECES] , SearchInfo search_info);

/// evaluation //////////////////////////////

int evaluate(GameStruct * game , CorPiece turno , int ai_level , PieceEvaluation pieces_eval[2][NUMBER_PIECES]);

/// engine /////////////////////////////////

Moves move_algorithm(GameStruct * game , CorPiece turn , int depth , int ai_level , int alpha);
