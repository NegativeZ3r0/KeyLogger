ifeq ($(OS),Windows_NT)
	CC := gcc
	RM_CMD := rmdir /s /q
	MKDIR_CMD := md
else
	CC := x86_64-w64-mingw32-gcc
	RM_CMD := rm -rf
	MKDIR_CMD := mkdir -p
endif

CFLAGS_BEFORE := -Wall -Wextra -g
CFLAGS_AFTER := -lws2_32 -mwindows
SRC_DIR := src
BIN_DIR := bin

SRCS := $(SRC_DIR)/keylogger.c
TARGET := keylogger

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS_BEFORE) $< -o $(BIN_DIR)/$@.exe $(CFLAGS_AFTER)

$(BIN_DIR):
	@echo "Creating bin directory $(BIN_DIR)/..."
	$(MKDIR_CMD) $(BIN_DIR)

clean:
	@echo "Cleaning up bin directory..."
	$(RM_CMD) $(BIN_DIR)
	@echo "Clean complete."
