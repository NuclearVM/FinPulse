#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

set -a
source "$PROJECT_ROOT/.env"
set +a

cmake --build "$SCRIPT_DIR/build" && \
"$SCRIPT_DIR/build/finpulse_backend"