CC = gcc

TFLAGS = -Wall -ggdb -Wextra -g3 -fsanitize=address,undefined -I. -Iengine/chess_lib
CFLAGS = -Wall -O3 -flto -DNDEBUG -I. -Iengine/chess_lib
LTFLAGS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_gfx -lm -fsanitize=address,undefined
LCFLAGS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_gfx -lm

SRC =   engine/chess_logic/castle_logic.c \
        engine/chess_logic/checkAndCheckmate.c \
        engine/chess_logic/chess_important.c \
        engine/chess_logic/en_passant.c \
        engine/chess_logic/moveMaker.c \
        engine/chess_logic/possibleMoves.c \
        engine/chess_logic/undoMove.c \
        engine/chess_logic/magicMoves.c \
        engine/core/engine.c \
        engine/core/evaluation.c \
        engine/core/moves.c \
        engine/core/search.c \
        engine/core/transposition.c \
        engine/core/initLT.c \
        gui/initialization/initStructs.c \
        gui/initialization/initTabuleiro.c \
        gui/initialization/startAndCleanup.c \
        gui/initialization/loadAssets.c \
        gui/interface/corefunctions.c \
        gui/interface/events.c \
        gui/interface/handleGameplay.c \
        gui/interface/main.c \
        gui/user/music.c \
        gui/user/gui.c \
        gui/user/universal_draws.c \

OBJ = $(SRC:%.c=build/%.o)

# Use sudo only when not already running as root
SUDO = $(shell [ "$$(id -u)" -eq 0 ] || echo sudo)

TARGET = bshark

.PHONY: all clean bench cleanb check deps install-deps

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LCFLAGS)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build
	rm bshark


bench: bench_nps
	./bench_nps

cleanb:
	rm bench_nps
BENCH_SRC = engine/chess_logic/castle_logic.c \
        engine/chess_logic/checkAndCheckmate.c \
        engine/chess_logic/chess_important.c \
        engine/chess_logic/en_passant.c \
        engine/chess_logic/moveMaker.c \
        engine/chess_logic/possibleMoves.c \
        engine/chess_logic/undoMove.c \
        engine/chess_logic/magicMoves.c \
        engine/core/engine.c \
        engine/core/evaluation.c \
        engine/core/moves.c \
        engine/core/search.c \
        engine/core/transposition.c \
        engine/core/initLT.c \
        gui/initialization/initTabuleiro.c \
        gui/interface/corefunctions.c \
        resources/benchmarks/bench_nps.c \
        resources/benchmarks/bench_globals.c
# Benchmark de nodes/segundo, isolado do GUI (nao precisa de SDL_image/mixer/ttf/assets).
# Usa o mesmo orcamento de tempo por jogada (2s) que o motor usa em jogo real.
bench_nps: $(BENCH_SRC)
	$(CC) -Wall -O3 -flto -DNDEBUG -I. -Iengine/chess_lib $(BENCH_SRC) -o $@ -lSDL2 -lm


check:
	@command -v gcc >/dev/null 2>&1 || { echo "gcc not installed"; exit 1; }
	@command -v make >/dev/null 2>&1 || { echo "make not installed"; exit 1; }
	@command -v pkg-config >/dev/null 2>&1 || { echo "pkg-config not installed"; exit 1; }

	@pkg-config --exists sdl2 || { echo "SDL2 missing"; exit 1; }
	@pkg-config --exists SDL2_image || { echo "SDL2_image missing"; exit 1; }
	@pkg-config --exists SDL2_mixer || { echo "SDL2_mixer missing"; exit 1; }
	@pkg-config --exists SDL2_ttf || { echo "SDL2_ttf missing"; exit 1; }
	@pkg-config --exists SDL2_gfx || { echo "SDL2_gfx missing"; exit 1; }
	@test -f /usr/include/dirent.h || { echo "dirent.h missing (Non-POSIX system?)"; exit 1; }

	@echo "All dependencies OK"

deps:
	@if $(MAKE) --no-print-directory check >/dev/null 2>&1; then \
		echo "All dependencies OK"; \
	else \
		echo "Missing dependencies, installing..."; \
		$(MAKE) --no-print-directory install-deps && $(MAKE) --no-print-directory check; \
	fi

install-deps:
	@if command -v apt-get >/dev/null 2>&1; then \
		$(SUDO) apt-get update && \
		$(SUDO) apt-get install -y gcc make pkg-config libc6-dev libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libsdl2-gfx-dev; \
	elif command -v dnf >/dev/null 2>&1; then \
		$(SUDO) dnf install -y gcc make pkgconf-pkg-config glibc-devel SDL2-devel SDL2_image-devel SDL2_mixer-devel SDL2_ttf-devel SDL2_gfx-devel; \
	elif command -v pacman >/dev/null 2>&1; then \
		$(SUDO) pacman -S --needed --noconfirm gcc make pkgconf sdl2 sdl2_image sdl2_mixer sdl2_ttf sdl2_gfx; \
	elif command -v zypper >/dev/null 2>&1; then \
		$(SUDO) zypper install -y gcc make pkg-config glibc-devel libSDL2-devel libSDL2_image-devel libSDL2_mixer-devel libSDL2_ttf-devel libSDL2_gfx-devel; \
	else \
		echo "No supported package manager found (apt, dnf, pacman, zypper). Install dependencies manually."; \
		exit 1; \
	fi