ifeq ($(OS),Windows_NT)
	CC := gcc
	RM_CMD := rmdir /s /q
	MKDIR_CMD := md
else
	CC := x86_64-w64-mingw32-gcc
	RM_CMD := rm -rf
	MKDIR_CMD := mkdir -p
endif

CFLAGS := -Wall -Wextra -g

SRC_DIR := src
BUILD_DIR := build

SRCS := $(SRC_DIR)/keylogger.c
TARGET := $(BUILD_DIR)/KeyLogger.exe

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR):
	@echo "Creating build directory $(BUILD_DIR)/..."
	$(MKDIR_CMD) $(BUILD_DIR)

clean:
	@echo "Cleaning up build directory..."
	$(RM_CMD) $(BUILD_DIR)
	@echo "Clean complete."
