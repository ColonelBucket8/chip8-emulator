CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Werror -Iinclude
LDFLAGS = -lgdi32 -luser32 -lwinmm

TARGET      = chip8.exe
TEST_TARGET = test_opcodes.exe

all: $(TARGET)

$(TARGET): src/main.c src/chip8.c src/platform_win32.c include/chip8.h include/platform.h
	$(CC) $(CFLAGS) src/main.c src/chip8.c src/platform_win32.c -o $(TARGET) $(LDFLAGS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): tests/test_opcodes.c src/chip8.c include/chip8.h
	$(CC) $(CFLAGS) tests/test_opcodes.c src/chip8.c -o $(TEST_TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

.PHONY: all test run clean
