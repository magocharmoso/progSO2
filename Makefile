CC = gcc

CFLAGS = -Wall -Wextra -O2 -Iinc

SRCS = src/main.c src/server.c src/wallet.c src/process.c src/memory.c

OBJ_DIR = obj
OBJS = $(SRCS:src/%.c=$(OBJ_DIR)/%.o)

TARGET = bin/SOchain002

all: $(TARGET)

prepare:
	mkdir -p $(OBJ_DIR) bin

$(TARGET): prepare $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(OBJ_DIR)/%.o: src/%.c inc/%.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) bin