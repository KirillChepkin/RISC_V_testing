gcc "try.c" -o "try"
echo "$ASM_DIR/try.asm"
gcc "try.c" -S -o "$ASM_DIR/try.asm"
./try