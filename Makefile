NAME		= tgrep

CC 			= clang
BUILD_DIR 	= build
SRC_DIR		= src
CFLAGS 		= -std=c23 -Wall -Wextra -I$(SRC_DIR)

SRCS		= $(wildcard $(SRC_DIR)/*.c)
OBJS		= $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))


all: $(BUILD_DIR)/$(NAME)

debug: CFLAGS += -g
debug: all

$(BUILD_DIR)/$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)/*.o $(BUILD_DIR)/$(NAME)

.PHONY: all clean debug
