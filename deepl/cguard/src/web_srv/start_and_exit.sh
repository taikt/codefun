#!/bin/bash
# Auto-exit wrapper for server startup

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
"$SCRIPT_DIR/start_server.sh" "$@"
# Script sẽ tự exit sau khi start_server.sh hoàn thành
