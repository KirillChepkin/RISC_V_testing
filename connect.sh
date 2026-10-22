source ./private_config.sh
source ./public_config.sh

ssh -J "$JUMP" -t "$DEST" "cd RISC_V_testing; exec bash --login"