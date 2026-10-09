source ./private_config.sh
source ./public_config.sh

# g++ try.cpp -o "$ASM_DIR/try.asm"
# ./try
# echo $PATH
# g++
g++ jpg/tga2jpg.cpp jpg/jpge.cpp jpg/jpgd.cpp jpg/timer.cpp -o result