CXX ?= g++
CXXFLAGS ?= -Wall -Wextra -Werror -std=c++11 -O2
ROOT := ../..
JPGE_DIR := $(ROOT)/jpg/jpge
BUILD_DIR := build
TARGET := $(BUILD_DIR)/color_transform_tests

.PHONY: all test clean
all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): src/main.cpp $(JPGE_DIR)/jpge_color_transform.cpp $(JPGE_DIR)/jpge_color_transform.h $(JPGE_DIR)/jpge_internal_modules.h $(JPGE_DIR)/jpge.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(JPGE_DIR) src/main.cpp $(JPGE_DIR)/jpge_color_transform.cpp -o $@

test: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
