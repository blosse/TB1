# === Compiler and flags ===
CC = clang
LIBS = -lportaudio -lraylib -lm \
        -framework CoreVideo -framework IOKit \
		-framework Cocoa -framework GLUT -framework OpenGL
CFLAGS = -Wall -Wextra -std=c11 -Iinclude -Iexternal -I/opt/homebrew/include -isystem external/
LDFLAGS = -L/opt/homebrew/lib 

# === Source and build ===
SRC = main.c synth.c waveform.c gui.c
OBJ_DIR = build
OBJ = $(SRC:.c=.o)
OBJ := $(addprefix $(OBJ_DIR)/, $(notdir $(OBJ)))

TARGET = tb1

# === Default rule ===
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

# === Compile .c to build/*.o ===
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

# === Make sure build/ exists ===
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# === Clean up ===
clean:
	rm -f $(OBJ) $(TARGET)
