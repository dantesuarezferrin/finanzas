CC = gcc
CFLAGS = -Wall -Wextra -pedantic -Iinclude $(shell pkg-config --cflags sqlite3 ncurses)
LDFLAGS = $(shell pkg-config --libs sqlite3 ncurses)

SRC_DIR = src
BUILD_DIR = build
TARGET = finanzas

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) db.sqlite
