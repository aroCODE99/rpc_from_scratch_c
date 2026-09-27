CC             := gcc
CFLAGS         := -Wall -Wextra -ggdb
BIN_DIR        := bin
SRC_DIR        := src
BUILD_DIR      := build
BIN_DIR        := bin
TARGET         := $(BIN_DIR)/rpc_gen

SRCS           := $(wildcard $(SRC_DIR)/*.c)
OBJS           := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
.PHONY         := all clean
all            := $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -f $(OBJS) $(TARGET) $(GENERATED_FILE)

