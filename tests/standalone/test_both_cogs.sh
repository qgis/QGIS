#!/bin/bash
# Test both Byte and Float32 COGs to compare performance

set -e

BYTE_COG="/private/tmp/nlcd_2024_cog.tif"
FLOAT_COG="/private/tmp/nlcd_2024_float_cog.tif"
TILE_COUNT=100

echo "=== COG Performance Comparison: Byte vs Float32 ==="
echo ""

# Check if float COG exists
if [ ! -f "$FLOAT_COG" ]; then
  echo "Float32 COG not found. Creating it..."
  echo ""
  ./create_float_cog.sh
  echo ""
fi

# Build test program
echo "Building test program..."
[ ! -f Makefile ] && cp build.mk Makefile
make clean > /dev/null 2>&1 || true
make
echo ""

echo "=========================================="
echo "TEST 1: Byte COG (original)"
echo "=========================================="
./test_cog_reader "$BYTE_COG" $TILE_COUNT

echo ""
echo "=========================================="
echo "TEST 2: Float32 COG (converted)"
echo "=========================================="
./test_cog_reader "$FLOAT_COG" $TILE_COUNT

echo ""
echo "=== Summary ==="
echo "Both tests complete. Compare the speedup values above."
echo ""
echo "Expected results:"
echo "  Byte:    2-3x speedup (palette lookups avoided)"
echo "  Float32: 2-3x speedup (fewer conversions)"
echo ""
echo "If both show similar speedup, the approach is validated!"
