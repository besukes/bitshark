CC = gcc

CFLAGS = -Wall -ggdb -Wextra -g3 -fsanitize=address,undefined -I. -Ielib
LDFLAGS = -lm -fsanitize=address,undefined

SRC = main.c \
	  esrc/depth_search.c \
	  esrc/evaluation.c \
	  esrc/initialization.c \
	  esrc/pruning.c

OBJ = $(SRC:%.c=build/%.o)


TARGET = engine

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build
	rm engine

check:
	@command -v gcc >/dev/null 2>&1 || { echo "gcc not installed"; exit 1; }
	@command -v make >/dev/null 2>&1 || { echo "make not installed"; exit 1; }