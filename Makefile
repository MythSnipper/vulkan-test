.PHONY: linux objs run clean_objs clean test

CC = g++
CFLAGS = -std=c++23 \
-Wall \
-Wextra \
-Iinclude \
-O0 \
-g
LDFLAGS = -lglfw \
-lvulkan \
-ldl \
-lpthread \
-lX11 \
-lXxf86vm \
-lXrandr \
-lXi

CC_WIN = x86_64-w64-mingw32-g++
CFLAGS_WIN = -std=c++23 \
-Wall \
-Wextra \
-Iinclude \
-O0 \
-g
LDFLAGS_WIN = -static \
-Llibs/vulkan -Llibs/glfw \
-lvulkan-1 \
-lglfw3 \
-lgdi32 \
-luser32 \
-lkernel32

OBJ = obj/main.o

TARGET = build/main
TARGET_WIN = build/main.exe

linux: $(TARGET) clean_objs
$(TARGET): objs
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)
objs:
	$(CC) $(CFLAGS) -c src/main.cpp -o obj/main.o


windows: $(TARGET_WIN) clean_objs
$(TARGET_WIN): objs_win
	$(CC_WIN) $(OBJ) -o $(TARGET_WIN) $(LDFLAGS_WIN)
objs_win:
	$(CC_WIN) $(CFLAGS_WIN) -c src/main.cpp -o obj/main.o



test: clean linux run
test_win: clean windows run_win

run:
	./build/main
run_win:
	wine ./build/main.exe


clean_objs:
	rm obj/main.o

clean:
	rm -rf build/*

test: clean linux run

