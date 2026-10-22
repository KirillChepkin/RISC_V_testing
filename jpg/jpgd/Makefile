CXXFLAGS = -Wall -Wextra -O3 -std=gnu++20 -march=rv64gcv

BUILD_DIR = ../../build

OBJS = $(BUILD_DIR)/jpgd.o $(BUILD_DIR)/icdt.o $(BUILD_DIR)/color_converter.o $(BUILD_DIR)/huffman_decoder.o

.PHONY: clean

$(BUILD_DIR)/libjpgd.a: $(OBJS)
	ar rcs $(BUILD_DIR)/libjpgd.a $(OBJS)

$(BUILD_DIR)/jpgd.o: jpgd.cpp jpgd.h jpgd_scripts.h
	g++ $(CXXFLAGS) -c jpgd.cpp -o $(BUILD_DIR)/jpgd.o

$(BUILD_DIR)/icdt.o: icdt.cpp jpgd.h jpgd_scripts.h
	g++ $(CXXFLAGS) -c icdt.cpp -o $(BUILD_DIR)/icdt.o

$(BUILD_DIR)/color_converter.o: color_converter.cpp jpgd.h jpgd_scripts.h
	g++ $(CXXFLAGS) -c color_converter.cpp -o $(BUILD_DIR)/color_converter.o

$(BUILD_DIR)/huffman_decoder.o: huffman_decoder.cpp jpgd.h jpgd_scripts.h
	g++ $(CXXFLAGS) -c huffman_decoder.cpp -o $(BUILD_DIR)/huffman_decoder.o

clean:
	rm -f $(OBJS) $(BUILD_DIR)/libjpgd.a