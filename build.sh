gcc "try.c" -o "try"
source "public_config.sh" && gcc "try.c" -S -o "$ASM_DIR/try.asm"
./try