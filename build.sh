. "$PWD/public_config.sh"

g++ "try.cpp" -o "try"
g++ "try.cpp" -S -o "$ASM_DIR/try.asm"
./try