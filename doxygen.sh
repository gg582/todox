#!/bin/bash
set -e

DOXYGEN="${DOXYGEN:-doxygen}"
PYTHON="${PYTHON:-python3}"
PORT="${PORT:-33333}"
DOCS_HTML="docs/html"

if ! command -v "$DOXYGEN" >/dev/null 2>&1; then
    echo "error: doxygen not found. install it first (e.g. apt install doxygen)."
    exit 1
fi

if ! command -v "$PYTHON" >/dev/null 2>&1; then
    echo "error: python3 not found."
    exit 1
fi

echo "generating doxygen documentation..."
"$DOXYGEN" Doxyfile

if [ ! -d "$DOCS_HTML" ]; then
    echo "error: $DOCS_HTML was not generated."
    exit 1
fi

echo "starting doxygen server at http://localhost:$PORT"
echo "press Ctrl+C to stop"
cd "$DOCS_HTML"
exec "$PYTHON" -m http.server "$PORT"
