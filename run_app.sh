#!/usr/bin/env bash

set -euo pipefail

MODE="${1:-release}"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_FILE="$ROOT_DIR/Dj Archetex/Dj Archetex.cpp"
INCLUDE_DIR="$ROOT_DIR/Dj Archetex"
BUILD_DIR="$ROOT_DIR/.build"

mkdir -p "$BUILD_DIR"

case "$MODE" in
  debug)
    OUTPUT_FILE="$BUILD_DIR/dj_archetex_debug"
    echo "Building debug test runner..."
    c++ -std=c++17 -D_DEBUG -g -I"$INCLUDE_DIR" "$SRC_FILE" -o "$OUTPUT_FILE"
    echo "Running debug build (doctest mode)..."
    exec "$OUTPUT_FILE"
    ;;
  release)
    OUTPUT_FILE="$BUILD_DIR/dj_archetex_release"
    echo "Building release app..."
    c++ -std=c++17 -O2 -I"$INCLUDE_DIR" "$SRC_FILE" -o "$OUTPUT_FILE"
    echo "Running release build (interactive app)..."
    cd "$INCLUDE_DIR"
    exec "$OUTPUT_FILE"
    ;;
  *)
    echo "Usage: $0 [debug|release]"
    echo "  debug   Builds with -D_DEBUG and runs doctests"
    echo "  release Builds optimized app and runs the interactive program"
    exit 1
    ;;
esac
