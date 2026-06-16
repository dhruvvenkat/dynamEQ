#!/usr/bin/env bash
set -euo pipefail

SERVICE_NAME="dynamEQ.service"
LOCAL_BINARY="./equalizer.out"
INSTALL_PATH="$HOME/.local/bin/dynamEQ"

echo "Building DynaEQ..."
make

if [[ ! -f "$LOCAL_BINARY" ]]; then
    echo "Error: compiled binary not found at $LOCAL_BINARY"
    exit 1
fi

echo "Stopping $SERVICE_NAME..."
systemctl --user stop "$SERVICE_NAME"

echo "Installing new binary..."
mkdir -p "$HOME/.local/bin"
cp "$LOCAL_BINARY" "$INSTALL_PATH"
chmod +x "$INSTALL_PATH"

echo "Starting $SERVICE_NAME..."
systemctl --user start "$SERVICE_NAME"

echo "Status:"
systemctl --user status "$SERVICE_NAME" --no-pager
