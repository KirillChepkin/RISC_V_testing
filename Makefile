CXXFLAGS = -Wall -Wextra -I.

.PHONY: all run clean

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

build/intrinsics_test:
	g++ -march=rv64gcv -S src/intrinsics.cpp -o riscv_asm/intrinsics.asm
	g++ -march=rv64gcv -c src/intrinsics.cpp -o build/intrinsics_test.o
	g++ build/intrinsics_test.o -o build/intrinsics_test

intrinsics: build/intrinsics_test
	perf stat -e cycles ./build/intrinsics_test

clean:
	rm -f build/jpge.o build/jpge_color_transform.o build/jpge_forward_dct.o build/jpge_huffman_tables.o build/jpge_marker_writer.o build/libjpge.a build/test.o build/test build/stb_image.o build/photo_compressor.o build/photo_compressor