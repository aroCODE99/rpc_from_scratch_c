CC             := gcc
CFLAGS         := -Wall -Wextra -ggdb 
OBJ_DIR        := build
TARGET         := $(OBJ_DIR)/rpc_gen
GENERATED_FILE := rpc_server.c rpc_client.c

SRC = $(wildcard $(SRC_DIR)/*.c)

OBJ = $(SRC:.c=.o)

.PHONY = all clean

all = $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

$(OBJ_DIR)%.o: %.c | $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -f $(OBJ) $(TARGET) $(GENERATED_FILE)

