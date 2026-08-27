#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdint.h>


#define NUMBER_PIECES 6
#define MAX_DEPTH_SEARCH 21
#define FLAG_TIMEOUT -999999
#define VALOR_INFINITO 99999

#define CHECKMATE_BENCHMARK 0
//Need to define one to 5 and the other to 4 and 5 = Queen , 4 = Rook
#define BENCHMARK_TESTED 5
#define BENCHMARK_NOT_TESTED 4

#define ITERATIVE_DEEPENING 1
#define BOT_PLAYS_BLACK 1
#define BOT_PLAYS_WHITE 1


#define COLUNA_A 0x0101010101010101ULL
#define COLUNA_H 0x8080808080808080ULL
#define COLUNA_B 0x0202020202020202ULL
#define COLUNA_G 0x4040404040404040ULL

#define FLAG_CASTLE 1
#define FLAG_ENPASSANT 2

#define NUM_SQUARES 64
#define MAX_NUMBER_MOVES 256

/*Struct que define a posicao de uma peca no tabuleiro de xadrez usando um long 64 bit*/
typedef unsigned long long uint64_bit;

typedef uint64_bit (*ShiftFunction)(uint64_bit,int); //Tipo que define um endereço de memória de uma função que recebe um unsigned long long de 64 bits e um int normal


// Estrutura leve para Bitboards para guardar jogadas (12 bytes)
typedef struct Jogada{
    uint8_t origem; // Guarda a posição de onde veio a peça movida
    uint8_t destino; // Guarda a posição para onde foi a peça movida
    uint8_t peca_movida; // Guarda a informação de qual peça foi movida
    uint8_t peca_capturada; // Guarda a informação de qual peça foi capturada no square destino , caso tenha sido
    uint8_t promocao; // Informa se está ou não a haver uma promoção com este move , e guarda a peça à qual está a ser promovido o peão
    uint8_t especial; // Informa se o move é "especial" , ou seja , enpassant ou castle
    int score; // Guarda o score desta jogada em termos de relevância teórica
    uint8_t prev_enpassant; //Guarda o estado anterior do enpassant
    uint8_t prev_castlerights[2][2]; //Matriz de possibilidades de dar castle
} Jogada;

extern Jogada killer_moves[MAX_DEPTH_SEARCH][2]; //Armazena os killer moves para cada profundidade de busca
extern int history_table[NUMBER_PIECES*2][NUM_SQUARES]; //Armazena a tabela de histórico para cada peça e posição

extern uint64_bit zobrist_pieces[2][NUMBER_PIECES][64];
extern uint64_bit zobrist_castle[2][2];
extern uint64_bit zobrist_ep[64];
extern uint64_bit zobrist_turn;

extern int hash_stack_indx;
extern uint64_bit hash_key_stack[2048];

extern int lmr_lt[MAX_DEPTH_SEARCH][256];

extern unsigned long total_nodes_searched;

typedef int Boolean; //Forma mais intuitiva de perceber quando as variáveis são usadas como valores lógicos.

typedef struct Coordenadas{
    int x;
    int y;
}Coordenadas;

/*Struct que serve apenas no startAndCleanup.c para retornar os endereços de memória do nosso renderer e window*/
typedef struct SDL_Initializators{
    SDL_Renderer * renderer; //renderer do jogo
    SDL_Window * window; //window do jogo
}SDL_Initializators;

typedef enum { GameScreen , WinScreen } UserScreen;

typedef enum { Invalid , Leave , Valid , Checkmate , TooLarge , Stalemate} TipoJogada; //Define o tipo de jogada que o utilizador efetuou

// Transposition table: guarda resultados de posições já pesquisadas para evitar recalculá-las
// quando se chega lá outra vez por uma ordem de jogadas diferente (transposição).
#define TT_SIZE (1 << 20) // ~1M entradas (potência de 2 para indexar com & em vez de %)
 
typedef enum { TT_EMPTY, TT_EXACT, TT_LOWERBOUND, TT_UPPERBOUND } TTFlag;
 
typedef struct TTEntry{
    uint64_bit key;
    int depth;
    int score;
    TTFlag flag;
    Jogada best_move;
} TTEntry;
 
extern TTEntry * transposition_table;

/*Enum que guarda todas as peças possíveis do jogo*/
typedef enum {
    King, //The piece is the KING
    Pawn, //The piece is a PAWN
    Horse, //The piece is the Knight
    Bishop, //The piece is the Bishop
    Rook, //The piece is the Rook
    Queen, //The piece is the QUEEN
    TheDog, //CChess Dog piece that protects the 3 squares in front of him (Norte , Nordeste , Noroeste)
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

/*Struct responsável por guardar as informações necessárias para desenhar setas do utilizador*/
typedef struct ArrowsGame{
    Boolean is_drawing_arrows;//Informa se o utilizador está neste momento a desenhar uma arrow 
    int indx_drawable_arrows;//Guarda o número de quantas setas é suposto desenhar no jogo
    /*Guarda os vetores de cada seta para desenhar no jogo.
    Este tipo define um array dinamico em que cada elemento é um array de 2 elementos de Indices de posicao do tabuleiro , uma posição original e uma posição final.
    Inicializa-se com realloc(indx_drawable_arrows * sizeof(Coordenadas[2]));*/
    int (*arrows_vector)[2];
}ArrowsGame;

/*Struct que define um estado de um jogo de xadrez.
Guarda as posições de todas as peças , bem como as posições afetadas por elas.
Guarda também informações sobre se um dado king está em check/checkmate ou não*/
typedef struct EstadoJogo{
    int checkMate; //Informa se um rei está em checkmate (game over)
    int stalemate; //Informa se o jogo acabou em staleMate
    int king_in_check[2]; //Informa se o reis estão em check
    int canCastle[2][2]; //Matriz de possibilidades de dar castle
    int is_castled[2]; //Informa se o rei estão castled
    uint64_bit enpassant; //Guarda a posição possível de se fazer enpassant
    uint64_bit tabuleirojogo[2][6]; //Guarda as informações do tabuleiro das peças regulares de xadrez
    uint64_bit bitboard_brancas; // Bitboard das peças brancas
    uint64_bit bitboard_pretas; // Bitboard das peças pretas
    uint64_bit bitboard_todas_pieces; //Bitboard que guarda as posições ocupadas por todas as peças no jogo
}EstadoJogo;



/*Struct responsável por guardar as informações sobre os peões se estão ou não a ser promovidos , se o peão foi promovido de forma correta
e o quadrado onde a promoção está a acontecer*/
typedef struct PromotedInformation{
    uint64_bit promoted_square; //Guarda o promotion square do pawn promovido
    Boolean pawnPromoted; //Guarda se o peão está a ser promovido
    Boolean promotedSucessfully; //Guarda se o peão foi promovido num clique
}PromotedInformation;


/*Struct que guarda o estado do jogo , tal como o turno do jogador , a peça que está a ser segurada(caso esteja a ser premida a tecla ,
que é da responsabilidade do bool isKeyPressedDown) e também a jogada do utilizador */
typedef struct GameStruct{
    Boolean game_needs_initialization; //Indica se o jogo precisa ser inicializado consoante a tela do utilizador
    EstadoJogo estadoJogo; //Estado atual do jogo
    Boolean isKeyPressedDown; //Verifica se o utilizador está a premir a tecla
    Pieces pieceSelecionada; //Guarda a peça que o utilizador está a ser segurada , caso esteja
    uint64_bit pieceCoords; //Guarda a posição de onde a peça que está a ser segurada veio , caso esteja
    uint64_bit selected_piece_attacks; //Guarda as posições da peça que estão sobre ataque da mesma
    uint64_bit moved_to_square; //Guarda o square para o qual foi movida a piece
    CorPiece turnoJogador; //Guarda o turno do utilizador 
    TipoJogada jogada; //Guarda informacao sobre a jogada do utilizador
    PromotedInformation promoted; //Guarda informações sobre se uma promotion aconteceu e onde ela ocorreu
    ArrowsGame arrows; //Guarda informações sobre se existem ou não setas desenhadas pelo utilizador e onde elas se encontram desenhadas
    int score_game; //Guarda o score do jogo (diferenca de pecas comidas)
    int turns; //Guarda os turnos ja jogados no jogo
    Boolean trying_to_leave; //Informa se o utilizador clicou no botao de sair
    uint64_bit cur_pos_key; //Current zobrist key da posicao
    float position_eval; //Curent position evaluation for evaluation bar
    int is_end_game; //Informa se o jogo já está em endgame ou não
}GameStruct;

/*Guarda as texturas que o jogo utiliza no seu decorrer , tal como o tema das peças*/
typedef struct AssetsCChess{
    SDL_Texture * chessPieces[12]; //Textura das pieces
    SDL_Texture * tabTexture; //Textura do tabuleiro
    SDL_Texture * fundo; //Textura do fundo
    SDL_Texture * arrow[2]; //Drawable arrows texture
    SDL_Texture * logo;
    SDL_Texture * miscTextures[20]; //Texturas misc 
}AssetsCChess;



/*Struct que guarda as definições da gui da engine bitshark mais relevantes , como o renderer responsável para apresentar imagens,
as texturas do jogo , a fonte das letras do jogo , as posições verticais e horizontais do rato do utilizador , o nª de ticks
que já se passaram deste o começo do jogo , bem como a tela em que o utilizador se encontra*/
typedef struct GuiSettings{
    AssetsCChess textures; //Texturas da engine
    SDL_Renderer * gameRenderer; //Renderer responsável por guardar a janela e onde desenhamos os objetos
    SDL_Window * window; //Window atual do utilizador
    TTF_Font * fonteJogoTitles; //Fonte das letras do nosso jogo de titulos importantes
    TTF_Font * fonteJogoSmallerTitles; //Fonte das letras do nosso jogo de titulos mais pequenos
    int posMouseX; //Posição horizontal do rato do utilizador , em termos de píxeis
    int posMouseY; //Posição vertical do rato do utilizador , em termos de píxeis
    UserScreen screen; //Screen atual
    int ticks; //Número de ticks que já passaram desde o começo do jogo (importante para o timer)
}GUISettings;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////     MODULOS     ///////////////////////////////////////////////////////////////////////////


//Modulo initStructs.c

GUISettings initGUISettings(SDL_Renderer * sdl_renderer , SDL_Window * window);
GameStruct initGameStruct(void);
EstadoJogo initEstadoJogo(void);
void initializeGame(GameStruct * game);
void initializeStructs(int matrix[2][NUMBER_PIECES],int indx);




//Modulo initTabuleiro.c

void initTabuleiro(uint64_bit pt[2][NUMBER_PIECES], int additor);
void init_other_bitboards(EstadoJogo * es);
void initTabuleiroBENCHMARK(EstadoJogo * state);



//Modulo loadAssets.c

void initTexturasJogo(GUISettings * settings ,SDL_Renderer * sdl_renderer);
void initsfx(Mix_Chunk * sfxarray[]);




//Modulo handleGameplay.c

void handleJogadaChess(GameStruct* game , GUISettings * settings,SDL_Event * event , Mix_Chunk * sfxarray[]);




//Modulo startAndCleanup.c

SDL_Initializators sdl_initializer(void);
void free_allocated_memory(GameStruct * game , GUISettings * user, Mix_Chunk * sfxarray[]);
void loadAssets(GUISettings * settings , SDL_Renderer * sdl_renderer);




//Modulo corefunctions.c

int posTabuleiro(uint64_bit bitboard);
uint64_bit click_table_position(int mouseX , int mouseY);
int dentroDoBotao(int mx , int my , int inf_x , int sup_x , int inf_y , int sup_y);
int minimum(int n1,int n2);
int maximum(int n1,int n2);
Pieces comparePiece(EstadoJogo * estado , CorPiece cor , uint64_bit posclique);
void getColunasAH(uint64_bit * colunaA , uint64_bit * colunaH);
int mouseOver(GUISettings * settings, SDL_Rect r);




//Modulo events.c

void efetuaEventoClique(GameStruct * game , GUISettings * settings,SDL_Event * event);
void efetuaEventoSoltar(GameStruct * game , GUISettings * settings , SDL_Event event , Mix_Chunk * sfxarray[]);
void cleanArrowEvent(GameStruct * game);
void efetuaEventoClickArrows(GameStruct * game , SDL_Event event);
void efetuaEventoSoltarArrows(GameStruct * game , SDL_Event event);
void updateScore(GameStruct * game , Jogada* jogada,CorPiece turn);




//Modulo moveMaker.c

void atualizaJogada(GameStruct * game , Jogada * jogada , CorPiece turno);
void promotePiece(GameStruct * game , Pieces piece, uint64_bit promotion_square , CorPiece turno);



//Modulo possibleMoves.c

int isPseudoValidMove(GameStruct * game, Jogada * jogada , CorPiece turno , uint64_bit pos_attacks);
uint64_bit get_knight_attacks(uint64_bit piece_pos);
uint64_bit get_pawn_attacks(uint64_bit piece_pos,CorPiece cor);
uint64_bit get_sliding_attacks(uint64_bit piece_pos, uint64_bit pos_limites);
uint64_bit get_cross_attacks(uint64_bit piece_pos , uint64_bit pos_limites);
uint64_bit get_piece_attacks(uint64_bit pos,Pieces piece,GameStruct * game,CorPiece cor_turno , int only_captures);
uint64_bit get_king_moves(uint64_bit pos);
uint64_bit get_possible_pawn_moves(uint64_bit pos,uint64_bit bitboard_pieces,CorPiece turno,uint64_bit (*func)(uint64_bit,int),GameStruct * game);
void king_line_dependant_moves(uint64_bit * atk ,uint64_bit (*func)(uint64_bit,int),uint64_bit pos_rei , uint64_bit colunaA , uint64_bit colunaH);
int gerar_jogadas_legais(GameStruct *game, Jogada * jogadas , CorPiece cor , int only_captures);
void get_attacks(int max , uint64_bit (*func)(uint64_bit,int),uint64_bit pos_limites,uint64_bit pos_piece,int shift ,uint64_bit * atk);
uint64_bit get_castle_moves(uint64_bit pos , uint64_bit bb_pieces , GameStruct * game , CorPiece turn);
int isPromotionRank(uint64_bit pos_piece , CorPiece turn);
int is_attacked_square(uint64_bit pos , CorPiece turn , GameStruct * game , uint64_bit occupancy);
int verify_pawn_promotion(Pieces piece , uint64_bit origem , uint64_bit dest , CorPiece turn);



//Modulo userinterface.c

void desenhaInterfaceJogo(GameStruct * game ,GUISettings * settings);
void desenhaWinScreen(GameStruct * game ,GUISettings * settings,SDL_Event event);
void desenharPieceDrag(Pieces tipoPiece , int mouseX , int mouseY , GUISettings * settings , int offset);




//Modulo checkAndCheckmate.c

TipoJogada check_move(GameStruct * game, Jogada * jogada , CorPiece turno);
int isCheckMate(GameStruct * game, CorPiece cor);
void notInCheck(GameStruct * game);
Boolean is_in_check(EstadoJogo * estado , uint64_bit kingpos , CorPiece cor);





//Modulo en_passant.c

void update_en_passant(GameStruct * game , Jogada * jogada , CorPiece turno);
Boolean can_en_passant(GameStruct * game , Jogada * jogada , CorPiece turno);
void enpassant_move(GameStruct * game , uint64_bit * cor_oposta , uint64_bit * mesma_cor,ShiftFunction ep, CorPiece turno, uint64_bit origem);



//Modulo chess_important.c

int pawnFirstRank(uint64_bit pos,CorPiece cor);
uint64_bit shiftr(uint64_bit pos,int shift);
uint64_bit shiftl(uint64_bit pos,int shift);
uint64_bit get_same_colour_bitboard(EstadoJogo * estado , CorPiece cor);
uint64_bit initQuadrado(void);
uint64_bit get_opposing_colour_bitboard(EstadoJogo * estado , CorPiece cor);
uint64_bit get_selected_piece_attacks(GameStruct * game , uint64_bit click , Pieces piece , CorPiece turno);



//Modulo undoMove.c

void undoMove(GameStruct * game , Jogada * jogada , CorPiece turn);



//Modulo castle_logic.c

int is_castelling_king(GameStruct * game , CorPiece cor , uint64_bit drop);
int invalidCastle(GameStruct * game , uint8_t pos , CorPiece turno);
void verifica_direito_castle(GameStruct * game , Jogada * jogada , CorPiece turn);
void castle_King(GameStruct * game , uint64_bit click , int square, uint64_bit * mesmaCor , CorPiece turno);




//Modulo universal_draws.c

void desenharPiece(Pieces tipoPiece , int linha , int coluna , GUISettings * settings, int offset);
void desenhaTipoPiece(uint64_bit pos_pieces,Pieces tipoPiece , GUISettings * settings, GameStruct * game, int offset);
void desenharPieceDrag(Pieces tipoPiece , int mouseX , int mouseY , GUISettings * settings , int offset);
void desenharPieceAttacks(GUISettings * settings , uint64_bit passant , uint64_bit attacks , uint64_bit cor_oposta);
void desenhaCheck(GameStruct * game , GUISettings * settings);
void desenhaPromotion(GameStruct * game , GUISettings * settings);
void desenhaFundo(GUISettings * settings , SDL_Texture * texture);
void renderTextoCentradoSombra(SDL_Renderer* r, TTF_Font* f, const char* txt, SDL_Color cor, int x , int y, float escala);
void renderTextoCentradoBasico(SDL_Renderer* r, TTF_Font* f, const char* txt, SDL_Color cor, int x , int y, float escala);
void drawScore(int score_game,SDL_Renderer * renderer , TTF_Font* f,int x , int y , float scale);
void drawTurns(CorPiece turno ,int turns_game,SDL_Renderer * renderer , TTF_Font* f,int x , int y , float scale);
void desenhaArrows(GameStruct * game , SDL_Renderer * renderer , SDL_Texture * generic_orange[2]);



//Modulo music.c
void movepiece_sfx (Mix_Chunk * sfxarray[]);
void capturepiece_sfx (Mix_Chunk * sfxarray[]);
void promote_sfx (Mix_Chunk * sfxarray[]);
void check_sfx (Mix_Chunk * sfxarray[]);
void checkmate_sfx (Mix_Chunk * sfxarray[]);



/////////////////////////////////////////////////////////////////////CHESS BOT///////////////////////////////////////////////////////////////////////

/// search /////////////////////////////

int search(GameStruct * game, int depth, int alpha, int beta, int wb_eval , double initial_time, double time_limit , CorPiece turn , int ply , int allows_nmp);



/// evaluation //////////////////////////////

int evaluate(GameStruct * game , CorPiece turno);
int evaluate_piece(uint64_bit piece_pos , Pieces piece_type , CorPiece turn , GameStruct * game);
int is_end_game(EstadoJogo * estado);
int mopup_eval(GameStruct * game);
int static_exchange_eval(GameStruct * game , Jogada * jogada , CorPiece turn);
int has_non_pawn_material(GameStruct * game, CorPiece turn);
int rookOpenFilesBonus(EstadoJogo * state , CorPiece turn);
int calculate_stronger_side(CorPiece * weak , CorPiece * strong , EstadoJogo * estado);


/// engine /////////////////////////////////

Jogada get_best_move(GameStruct * game , CorPiece turn , int is_interative_deepening, SDL_Event * e);



/// moves /////////////////////////////////

int applyDeltaMove(GameStruct * game , Jogada * jogada , CorPiece turn , CorPiece op_turn);
void pick_best_move(Jogada * jogadas, int num_jogadas, int start_index);
void moveScoring(GameStruct * game ,Jogada * jogadas , int num_jogadas , Jogada * hash_move , int depth , CorPiece turn);
void moveScoringCaptures(GameStruct * game ,Jogada * jogadas , int num_jogadas , Jogada * hash_move , CorPiece turn);
int matches_killer_move(int depth, Jogada * jogada, int index);
int calculate_extension_depth(int op_king_in_check, int depth , CorPiece op_turn);
int is_repeated_position(uint64_bit key);



/// transposition /////////////////////////

uint64_bit xorshift64(uint64_bit *state);
void init_zobrist(void);
void tt_init(void);
uint64_bit compute_zobrist(GameStruct * game, CorPiece turn);
TTEntry * tt_probe(uint64_bit key);
void tt_store(uint64_bit key, int depth, int score, TTFlag flag, Jogada best_move , int ply);
TTEntry * getPositionTTMove(uint64_bit key , int depth , int * alpha , int * beta , int * move_eval , Jogada * * hash_move , int ply);
void updateZobrist(GameStruct * game , Jogada * jogada ,CorPiece turn , CorPiece op_turn);



/// LookUp Table init //////////////////////

void init_lmrLT_table();
void init_passedPawnsLT_table();



/// MAGIC MOVES //////////////////////////

void initMagicMoveGeneration(void);
uint64_bit get_magic_piece_attacks(uint64_bit pos,Pieces piece,GameStruct * game , CorPiece cor_turno , int only_captures , uint64_bit bitboardPieces);
uint64_bit get_magic_knight_attacks(int square);
uint64_bit get_magic_sliding_attacks(int square , uint64_bit occupancy);
uint64_bit get_magic_cross_attacks(int square, uint64_bit occupancy);