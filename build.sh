. "public_config.sh"

gcc "try.c" -o "try"
gcc "try.c" -S -o "$ASM_DIR/try.asm"
./try