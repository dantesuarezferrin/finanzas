PREFIX ?= /usr/local
CC = gcc
CFLAGS = -Wall -Wextra -g -pedantic -Iinclude $(shell pkg-config --cflags sqlite3 ncursesw)
LIBS = $(shell pkg-config --libs sqlite3 ncursesw) -lm

SRC_DIR = src
BUILD_DIR = build
TARGET = finanzas

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) db.sqlite

install: $(TARGET)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(TARGET) $(DESTDIR)$(PREFIX)/bin/
