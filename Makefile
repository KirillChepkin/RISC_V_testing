CXXFLAGS = -Wall -Wextra -I.

.PHONY: all run clean

build/jpge.o: jpg/jpge.cpp jpg/jpge.h
	g++ $(CXXFLAGS) -c jpg/jpge.cpp -o build/jpge.o

build/libjpge.a: build/jpge.o
	ar rcs build/libjpge.a build/jpge.o

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

clean:
	rm -f build/jpge.o build/libjpge.a build/test.o build/test build/stb_image.o build/photo_compressor.o build/photo_compressor