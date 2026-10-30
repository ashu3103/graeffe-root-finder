# compiler
CXX := clang

# flags
CXXFLAGS := -O3 -fno-math-errno -fopenmp=libomp \
			 -fveclib=libmvec -ffast-math \
			 -fno-inline-functions \
			 -Rpass=loop-vectorize \
			 -Rpass-missed=loop-vectorize \
			 -Rpass-analysis=loop-vectorize \
			 -march=native
ASMFLAGS := -g -fverbose-asm
LDFLAGS  :=         # e.g. -Llib
LDLIBS   := -lm -fopenmp=libomp

SRC_DIR   := src
BUILD_DIR := build
ASM_DIR   := asm
TARGET    := app

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
ASMS := $(SRCS:$(SRC_DIR)/%.c=$(ASM_DIR)/%.s)

.PHONY: all asm clean run release

all: $(TARGET) $(ASMS)

# Link
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile to assembly
$(ASM_DIR)/%.s: $(SRC_DIR)/%.c | $(ASM_DIR)
	$(CXX) $(CXXFLAGS) $(ASMFLAGS) -S $< -o $@

$(ASM_DIR):
	mkdir -p $@

$(BUILD_DIR):
	mkdir -p $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(ASM_DIR) $(TARGET)