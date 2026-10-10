. "$PWD/public_config.sh"

g++ jpg/tga2jpg.cpp jpg/jpge.cpp jpg/jpgd.cpp jpg/timer.cpp -o3 -o result
perf stat -e cycles ./result images/uncompressed/linux_pinguin.png images/compressed/linux_pinguin.jpg 100