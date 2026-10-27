BUILD_DIR := build
BIN := $(BUILD_DIR)/clang-tidy-cache
SOURCES := $(shell find src -name '*.cpp')
HEADERS := $(shell find src -name '*.h')

.PHONY: all tidy format clean

all:
	cmake -S . -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

$(BIN): all

tidy: $(BIN)
	$(BIN) -p $(BUILD_DIR) $(SOURCES)

format:
	clang-format -i $(SOURCES) $(HEADERS)

clean:
	rm -rf $(BUILD_DIR)
