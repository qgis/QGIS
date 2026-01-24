#!/bin/bash
# Create Float32 COG from NLCD - Fixed for palette issues

set -e

INPUT="/private/tmp/nlcd_2024_cog.tif"
OUTPUT="/private/tmp/nlcd_2024_float_cog.tif"

echo "=== Creating Float32 COG (Fixed) ==="
echo "Input:  $INPUT"
echo "Output: $OUTPUT"
echo ""

# Check input
if [ ! -f "$INPUT" ]; then
  echo "ERROR: Input not found"
  exit 1
fi

GDAL_TRANSLATE="/opt/homebrew/bin/gdal_translate"

# Remove old output if exists
rm -f "$OUTPUT"

echo "Converting Byte to Float32..."
echo "(This will take ~3-4 minutes for the full NLCD file)"
echo ""

# Convert with COG driver, expanding palette to RGB first, then convert to Float32
# This avoids the color table export error
$GDAL_TRANSLATE \
  -of COG \
  -ot Float32 \
  -expand rgb \
  -b 1 \
  -co COMPRESS=DEFLATE \
  -co BIGTIFF=YES \
  -co BLOCKSIZE=512 \
  -co NUM_THREADS=ALL_CPUS \
  -co OVERVIEW_RESAMPLING=NEAREST \
  -co RESAMPLING=NEAREST \
  "$INPUT" \
  "$OUTPUT"

echo ""
echo "✓ Done!"
echo ""
echo "Verification:"
/opt/homebrew/bin/gdalinfo "$OUTPUT" | grep -E "Size is|Block=|Type=|Overviews:"
echo ""
echo "Test with:"
echo "  ./test_cog_reader $OUTPUT 100"
