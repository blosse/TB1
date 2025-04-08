# === Compiler and flags ===
CC = clang
CFLAGS = -Wall -Wextra -std=c11 -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lportaudio -lm

# === Source and build ===
SRC = synth.c waveform.c
OBJ_DIR = build
OBJ = $(SRC:.c=.o)
OBJ := $(addprefix $(OBJ_DIR)/, $(notdir $(OBJ)))

TARGET = tb1

# === Default rule ===
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

# === Compile .c to build/*.o ===
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

# === Make sure build/ exists ===
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# === Clean up ===
clean:
	rm -f $(OBJ) $(TARGET)
