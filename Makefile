CC = clang
CFLAGS = -Wall -Werror -pedantic

TARGET = main

SRC_DIR = src
OBJ_DIR = build/obj
BIN_DIR = build
LIB_DIR = lib

RAYLIB_DIR = $(LIB_DIR)/raylib-5.5_linux_amd64
RAYLIB_LIB = $(RAYLIB_DIR)/lib
RAYLIB_INC = $(RAYLIB_DIR)/include

IFLAGS = -I./$(RAYLIB_INC)
LDFLAGS = -L./$(RAYLIB_LIB) -l:libraylib.a -lm

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(BIN_DIR)/$(TARGET)

# Link
$(BIN_DIR)/$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf build

.PHONY: all clean

# cc -o build/main src/main.c -I./lib/raylib-5.5_linux_amd64/include -L./lib/raylib-5.5_linux_amd64/lib -l:libraylib.a -lm
