source ./private_config.sh
source ./public_config.sh

scp -J "$JUMP" "$DEST:$PATH_TO_RISCV_REP/$ASM_DIR/*" "$PATH_TO_LOCAL_REP/$ASM_DIR"