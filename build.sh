source ./public_config.sh

gcc try.c -o try
rm riscv_asm/*
gcc try.c -S -o "$ASM_DIR/try.asm"
./try