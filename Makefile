CC = gcc

TFLAGS = -Wall -ggdb -Wextra -g3 -fsanitize=address,undefined -I. -Iengine/chess_lib
SFLAGS = -Wall -O3 -I. -Iengine/chess_lib
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_gfx -lm -fsanitize=address,undefined
LLFLAGS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_gfx -lm

SRC =	engine/chess_logic/castle_logic.c \
		engine/chess_logic/checkAndCheckmate.c \
		engine/chess_logic/chess_important.c \
		engine/chess_logic/en_passant.c \
		engine/chess_logic/moveMaker.c \
		engine/chess_logic/possibleMoves.c \
		engine/chess_logic/undoMove.c \
		engine/core/engine.c \
		engine/core/evaluation.c \
		engine/core/moves.c \
		engine/core/search.c \
		gui/initialization/initStructs.c \
		gui/initialization/initTabuleiro.c \
		gui/initialization/startAndCleanup.c \
		gui/interface/corefunctions.c \
		gui/interface/events.c \
		gui/interface/handleGameplay.c \
		gui/interface/main.c \
		gui/user/loadAssets.c \
		gui/user/music.c \
		gui/user/gui.c \
		gui/user/universal_draws.c \

OBJ = $(SRC:%.c=build/%.o)


TARGET = bshark

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LLFLAGS)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(SFLAGS) -c $< -o $@

clean:
	rm -rf build
	rm bshark

check:
	@command -v gcc >/dev/null 2>&1 || { echo "gcc not installed"; exit 1; }
	@command -v make >/dev/null 2>&1 || { echo "make not installed"; exit 1; }

	@pkg-config --exists sdl2 || { echo "SDL2 missing"; exit 1; }
	@pkg-config --exists SDL2_image || { echo "SDL2_image missing"; exit 1; }
	@pkg-config --exists SDL2_mixer || { echo "SDL2_mixer missing"; exit 1; }
	@pkg-config --exists SDL2_ttf || { echo "SDL2_ttf missing"; exit 1; }
	@pkg-config --exists SDL2_gfx || { echo "SDL2_gfx missing"; exit 1; }
	@test -f /usr/include/dirent.h || { echo "dirent.h missing (Non-POSIX system?)"; exit 1; }

	@echo "All dependencies OK"
