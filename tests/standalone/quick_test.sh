#!/bin/bash
# Quick test script - build and run in one command

set -e

cd "$(dirname "$0")"

echo "=== Fast COG Reader Test ==="
echo ""

# Build
echo "Building (should take ~3 seconds)..."
# Copy build config if Makefile doesn't exist
[ ! -f Makefile ] && cp build.mk Makefile
make clean > /dev/null 2>&1 || true
time make

echo ""
echo "=== Running Benchmark ==="
echo ""

# Find a COG to test
if [ -f "/private/tmp/nlcd_2024_cog.tif" ]; then
  echo "Testing with NLCD COG..."
  ./test_cog_reader /private/tmp/nlcd_2024_cog.tif 100
elif [ -n "$1" ]; then
  echo "Testing with: $1"
  ./test_cog_reader "$1" "${2:-100}"
else
  echo "ERROR: No test file found"
  echo ""
  echo "Usage:"
  echo "  $0 <cog_file.tif> [tile_count]"
  echo ""
  echo "Or place a test file at: /private/tmp/nlcd_2024_cog.tif"
  exit 1
fi
