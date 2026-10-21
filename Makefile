CXXFLAGS = -Wall -Wextra -I -O3 -std=gnu++20 -march=rv64gcv

.PHONY: all run clean


# compression cluster
build/jpge.o: jpg/jpge.cpp jpg/jpge.h jpg/jpge_internal_modules.h
	g++ $(CXXFLAGS) -c jpg/jpge.cpp -o build/jpge.o

build/jpge_color_transform.o: jpg/jpge_color_transform.cpp jpg/jpge_color_transform.h jpg/jpge_internal_modules.h
	g++ $(CXXFLAGS) -c jpg/jpge_color_transform.cpp -o build/jpge_color_transform.o

build/jpge_forward_dct.o: jpg/jpge_forward_dct.cpp jpg/jpge_forward_dct.h jpg/jpge_internal_modules.h
	g++ $(CXXFLAGS) -c jpg/jpge_forward_dct.cpp -o build/jpge_forward_dct.o

build/jpge_huffman_tables.o: jpg/jpge_huffman_tables.cpp jpg/jpge_huffman_tables.h
	g++ $(CXXFLAGS) -c jpg/jpge_huffman_tables.cpp -o build/jpge_huffman_tables.o

build/jpge_marker_writer.o: jpg/jpge_marker_writer.cpp jpg/jpge_marker_writer.h
	g++ $(CXXFLAGS) -c jpg/jpge_marker_writer.cpp -o build/jpge_marker_writer.o

build/libjpge.a: build/jpge.o build/jpge_color_transform.o build/jpge_forward_dct.o build/jpge_huffman_tables.o build/jpge_marker_writer.o
	ar rcs build/libjpge.a build/jpge.o build/jpge_color_transform.o build/jpge_forward_dct.o build/jpge_huffman_tables.o build/jpge_marker_writer.o

library: build/libjpge.a





# decompression cluster
build/jpgd.o: jpg/jpgd.cpp jpg/jpgd.h jpg/jpgd_scripts.h
	g++ $(CXXFLAGS) -c jpg/jpgd.cpp -o build/jpgd.o

build/icdt.o: jpg/icdt.cpp jpg/jpgd.h jpg/jpgd_scripts.h
	g++ $(CXXFLAGS) -c jpg/icdt.cpp -o build/icdt.o

build/color_converter.o: jpg/color_converter.cpp jpg/jpgd.h jpg/jpgd_scripts.h
	g++ $(CXXFLAGS) -c jpg/color_converter.cpp -o build/color_converter.o

build/haffman_decoder.o: jpg/haffman_decoder.cpp jpg/jpgd.h jpg/jpgd_scripts.h
	g++ $(CXXFLAGS) -c jpg/haffman_decoder.cpp -o build/haffman_decoder.o

build/libjpgd.a: build/jpgd.o build/icdt.o build/color_converter.o build/haffman_decoder.o
	ar rcs build/libjpgd.a build/jpgd.o build/icdt.o build/color_converter.o build/haffman_decoder.o

dlibrary: build/libjpgd.a

build/photo_decompressor.o: src/photo_decompressor.cpp jpg/jpgd.h
	g++ $(CXXFLAGS) -c src/photo_decompressor.cpp -o build/photo_decompressor.o

build/photo_decompressor: build/photo_decompressor.o build/stb_image.o build/libjpgd.a
	g++ build/photo_decompressor.o build/stb_image.o -Lbuild -ljpgd -o build/photo_decompressor

decomp_photo: build/photo_decompressor
	perf stat -e cycles ./build/photo_decompressor





# test and example cluster
build/test.o: src/test.cpp jpg/jpge.h
	g++ $(CXXFLAGS) -c src/test.cpp -o build/test.o

build/test: build/test.o build/libjpge.a
	g++ build/test.o -Lbuild -ljpge -o build/test

test: build/test
	mkdir -p output
	./build/test

build/stb_image.o: jpg/stb_image.c
	gcc -w -c jpg/stb_image.c -o build/stb_image.o

build/photo_compressor.o: src/photo_compressor.cpp jpg/jpge.h
	g++ $(CXXFLAGS) -c src/photo_compressor.cpp -o build/photo_compressor.o

build/photo_compressor: build/photo_compressor.o build/stb_image.o build/libjpge.a
	g++ build/photo_compressor.o build/stb_image.o -Lbuild -ljpge -o build/photo_compressor

photo: build/photo_compressor
	perf stat -e cycles ./build/photo_compressor
# test for vector multiplication with intrinsics and without.
build/intrinsics_test_vector:
	g++ $(CXXFLAGS) -DVECTOR -c src/intrinsics.cpp -o build/intrinsics_test_vector.o
	g++ build/intrinsics_test_vector.o -o build/intrinsics_test_vector

build/intrinsics_test_vector_assembly:
	g++ $(CXXFLAGS) -DVECTOR -S src/intrinsics.cpp -o riscv_asm/intrinsics_for_vector.asm

# Commands from four of the following rules tell the compiler to:
# 1) run without the VECTOR macro thus hiding all code with intrinsics from compiler.
# 2) compile for riscv vector with -O3 flag which means to optimize generated code.
build/intrinsics_test_scalar:
	g++ $(CXXFLAGS) -c src/intrinsics.cpp -o build/intrinsics_test_scalar.o
	g++ build/intrinsics_test_scalar.o -o build/intrinsics_test_scalar

build/intrinsics_test_scalar_assembly:
	g++ $(CXXFLAGS) -S src/intrinsics.cpp -o riscv_asm/intrinsics_for_general.asm

intrinsics_assembly: build/intrinsics_test_vector_assembly build/intrinsics_test_scalar_assembly

intrinsics: build/intrinsics_test_vector build/intrinsics_test_scalar
	perf stat -e cycles ./build/intrinsics_test_vector
	perf stat -e cycles ./build/intrinsics_test_scalar





clean:
	rm -f build/jpge.o build/libjpge.a build/test.o build/test build/stb_image.o build/photo_compressor.o build/photo_compressor
	rm -f build/jpgd.o build/icdt.o build/color_converter.o build/haffman_decoder.o build/libjpgd.a build/photo_decompressor.o build/photo_decompressor