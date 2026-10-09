. "$PWD/public_config.sh"

g++ "try.cpp" -o "try"
g++ "try.cpp" -S -o "$ASM_DIR/try.asm"
./try
g++ jpg/tga2jpg.cpp jpg/jpge.cpp jpg/jpgd.cpp jpg/timer.cpp -o result
./result