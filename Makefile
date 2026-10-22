CXXFLAGS = -Wall -Wextra -O3 -std=gnu++20 -march=rv64gcv

BUILD_DIR = build

.PHONY: all clean jpge jpgd test comp_photo decomp_photo intrinsics intrinsics_assembly

jpge:
	$(MAKE) -C jpg/jpge

jpgd:
	$(MAKE) -C jpg/jpgd

# stb_image
$(BUILD_DIR)/stb_image.o: jpg/stb_image.c
	gcc -w -c jpg/stb_image.c -o $(BUILD_DIR)/stb_image.o

# Photo compressor
$(BUILD_DIR)/photo_compressor.o: src/photo_compressor.cpp jpg/jpge/jpge.h
	g++ $(CXXFLAGS) -c src/photo_compressor.cpp -o $(BUILD_DIR)/photo_compressor.o

$(BUILD_DIR)/photo_compressor: $(BUILD_DIR)/photo_compressor.o $(BUILD_DIR)/stb_image.o jpge
	g++ $(BUILD_DIR)/photo_compressor.o $(BUILD_DIR)/stb_image.o -L$(BUILD_DIR) -ljpge -o $(BUILD_DIR)/photo_compressor

comp_photo: $(BUILD_DIR)/photo_compressor
	perf stat -e cycles ./$(BUILD_DIR)/photo_compressor

# Photo decompressor
$(BUILD_DIR)/photo_decompressor.o: src/photo_decompressor.cpp jpg/jpgd/jpgd.h
	g++ $(CXXFLAGS) -c src/photo_decompressor.cpp -o $(BUILD_DIR)/photo_decompressor.o

$(BUILD_DIR)/photo_decompressor: $(BUILD_DIR)/photo_decompressor.o $(BUILD_DIR)/stb_image.o jpgd
	g++ $(BUILD_DIR)/photo_decompressor.o $(BUILD_DIR)/stb_image.o -L$(BUILD_DIR) -ljpgd -o $(BUILD_DIR)/photo_decompressor

decomp_photo: $(BUILD_DIR)/photo_decompressor
	perf stat -e cycles ./$(BUILD_DIR)/photo_decompressor

# Test and example
$(BUILD_DIR)/test.o: src/test.cpp jpg/jpge/jpge.h
	g++ $(CXXFLAGS) -c src/test.cpp -o $(BUILD_DIR)/test.o

$(BUILD_DIR)/test: $(BUILD_DIR)/test.o jpge
	g++ $(BUILD_DIR)/test.o -L$(BUILD_DIR) -ljpge -o $(BUILD_DIR)/test

test: $(BUILD_DIR)/test
	./$(BUILD_DIR)/test

# Vector intrinsics and scalar examples
$(BUILD_DIR)/intrinsics_test_vector.o: src/intrinsics.cpp
	g++ $(CXXFLAGS) -DVECTOR -c src/intrinsics.cpp -o $(BUILD_DIR)/intrinsics_test_vector.o

$(BUILD_DIR)/intrinsics_test_vector: $(BUILD_DIR)/intrinsics_test_vector.o
	g++ $(BUILD_DIR)/intrinsics_test_vector.o -o $(BUILD_DIR)/intrinsics_test_vector

$(BUILD_DIR)/intrinsics_test_scalar.o: src/intrinsics.cpp
	g++ $(CXXFLAGS) -c src/intrinsics.cpp -o $(BUILD_DIR)/intrinsics_test_scalar.o

$(BUILD_DIR)/intrinsics_test_scalar: $(BUILD_DIR)/intrinsics_test_scalar.o
	g++ $(BUILD_DIR)/intrinsics_test_scalar.o -o $(BUILD_DIR)/intrinsics_test_scalar

$(BUILD_DIR)/intrinsics_test_vector_assembly: src/intrinsics.cpp
	g++ $(CXXFLAGS) -DVECTOR -S src/intrinsics.cpp -o riscv_asm/intrinsics_for_vector.asm

$(BUILD_DIR)/intrinsics_test_scalar_assembly: src/intrinsics.cpp
	g++ $(CXXFLAGS) -S src/intrinsics.cpp -o riscv_asm/intrinsics_for_general.asm

intrinsics_assembly: $(BUILD_DIR)/intrinsics_test_vector_assembly $(BUILD_DIR)/intrinsics_test_scalar_assembly

intrinsics: $(BUILD_DIR)/intrinsics_test_vector $(BUILD_DIR)/intrinsics_test_scalar
	perf stat -e cycles ./$(BUILD_DIR)/intrinsics_test_vector
	perf stat -e cycles ./$(BUILD_DIR)/intrinsics_test_scalar

# Default targets
all: $(BUILD_DIR)/test $(BUILD_DIR)/photo_compressor $(BUILD_DIR)/photo_decompressor $(BUILD_DIR)/intrinsics_test_vector $(BUILD_DIR)/intrinsics_test_scalar

# Cleanup
clean:
	$(MAKE) -C jpg/jpge clean
	$(MAKE) -C jpg/jpgd clean
	rm -f $(BUILD_DIR)/test.o $(BUILD_DIR)/test
	rm -f $(BUILD_DIR)/stb_image.o
	rm -f $(BUILD_DIR)/photo_compressor.o $(BUILD_DIR)/photo_compressor
	rm -f $(BUILD_DIR)/photo_decompressor.o $(BUILD_DIR)/photo_decompressor
	rm -f $(BUILD_DIR)/intrinsics_test_vector.o $(BUILD_DIR)/intrinsics_test_vector
	rm -f $(BUILD_DIR)/intrinsics_test_scalar.o $(BUILD_DIR)/intrinsics_test_scalar
	rm -f riscv_asm/intrinsics_for_vector.asm riscv_asm/intrinsics_for_general.asm