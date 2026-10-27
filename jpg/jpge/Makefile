CXXFLAGS = -Wall -Wextra -O3 -std=gnu++20 -march=rv64gcv

BUILD_DIR = ../../build

OBJS = $(BUILD_DIR)/jpge.o $(BUILD_DIR)/jpge_color_transform.o $(BUILD_DIR)/jpge_forward_dct.o $(BUILD_DIR)/jpge_huffman_tables.o $(BUILD_DIR)/jpge_marker_writer.o

.PHONY: clean

$(BUILD_DIR)/libjpge.a: $(OBJS)
	ar rcs $(BUILD_DIR)/libjpge.a $(OBJS)

$(BUILD_DIR)/jpge.o: jpge.cpp jpge.h jpge_internal_modules.h
	g++ $(CXXFLAGS) -c jpge.cpp -o $(BUILD_DIR)/jpge.o

$(BUILD_DIR)/jpge_color_transform.o: jpge_color_transform.cpp jpge_color_transform.h jpge_internal_modules.h
	g++ $(CXXFLAGS) -c jpge_color_transform.cpp -o $(BUILD_DIR)/jpge_color_transform.o

$(BUILD_DIR)/jpge_forward_dct.o: forward_dct/jpge_forward_dct.cpp forward_dct/jpge_forward_dct.h jpge_internal_modules.h
	g++ $(CXXFLAGS) -c forward_dct/jpge_forward_dct.cpp -o $(BUILD_DIR)/jpge_forward_dct.o

$(BUILD_DIR)/jpge_huffman_tables.o: jpge_huffman_tables.cpp jpge_huffman_tables.h
	g++ $(CXXFLAGS) -c jpge_huffman_tables.cpp -o $(BUILD_DIR)/jpge_huffman_tables.o

$(BUILD_DIR)/jpge_marker_writer.o: jpge_marker_writer.cpp jpge_marker_writer.h
	g++ $(CXXFLAGS) -c jpge_marker_writer.cpp -o $(BUILD_DIR)/jpge_marker_writer.o

clean:
	rm -f $(OBJS) $(BUILD_DIR)/libjpge.a