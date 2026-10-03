source ./private_config.sh
source ./public_config.sh

ssh -J "$JUMP" "$DEST"