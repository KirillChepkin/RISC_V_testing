if ["$(uname -s)" = "Linux"]; then
    source ./public_config.sh

    gcc try.c -o try
    gcc try.c -S -o "$ASM_DIR/try.asm"
    ./try
else
    echo "This script is for running on Debian, don't launch it here!"
fi