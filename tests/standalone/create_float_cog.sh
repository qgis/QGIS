#!/bin/bash
# Create a Float32 COG from NLCD Byte COG
# Preserves tiling structure and creates overviews

set -e

INPUT="/private/tmp/nlcd_2024_cog.tif"
OUTPUT="/private/tmp/nlcd_2024_float_cog.tif"
TEMP="/private/tmp/nlcd_2024_float_temp.tif"

echo "=== Creating Float32 COG from NLCD Byte COG ==="
echo ""
echo "Input:  $INPUT"
echo "Output: $OUTPUT"
echo ""

# Check input exists
if [ ! -f "$INPUT" ]; then
  echo "ERROR: Input file not found: $INPUT"
  exit 1
fi

# Get GDAL path
GDAL_TRANSLATE="/opt/homebrew/bin/gdal_translate"
GDALADDO="/opt/homebrew/bin/gdaladdo"

if [ ! -f "$GDAL_TRANSLATE" ]; then
  echo "ERROR: gdal_translate not found at $GDAL_TRANSLATE"
  echo "Install with: brew install gdal"
  exit 1
fi

echo "Step 1: Converting Byte to Float32 (with COG tiling)..."
$GDAL_TRANSLATE \
  -of COG \
  -ot Float32 \
  -co COMPRESS=DEFLATE \
  -co BIGTIFF=YES \
  -co BLOCKSIZE=512 \
  -co NUM_THREADS=ALL_CPUS \
  -co OVERVIEW_RESAMPLING=NEAREST \
  "$INPUT" \
  "$OUTPUT"

echo ""
echo "Step 2: Adding overviews (matching original: 6 levels)..."
$GDALADDO \
  -r nearest \
  --config COMPRESS_OVERVIEW DEFLATE \
  "$OUTPUT" \
  2 4 8 16 32 64

echo ""
echo "=== Verification ==="
echo ""

echo "Original (Byte COG):"
$GDAL_TRANSLATE --version > /dev/null 2>&1
/opt/homebrew/bin/gdalinfo "$INPUT" | grep -E "Size is|Block=|Type=|Overviews:"

echo ""
echo "New (Float32 COG):"
/opt/homebrew/bin/gdalinfo "$OUTPUT" | grep -E "Size is|Block=|Type=|Overviews:"

echo ""
echo "✓ Float32 COG created successfully!"
echo ""
echo "Test with:"
echo "  cd /Users/w/Documents/Github/QGIS/tests/standalone"
echo "  ./quick_test.sh $OUTPUT 100"
