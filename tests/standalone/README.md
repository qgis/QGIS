# Fast COG Reader Standalone Test

**Purpose**: Test QgsCOGTileReader performance WITHOUT building full QGIS.
**Build time**: ~3 seconds (vs. hours for full QGIS build)
**Dependencies**: Only GDAL

## Quick Start

```bash
cd /Users/w/Documents/Github/QGIS/tests/standalone

# Build (takes ~3 seconds)
make

# Test with your COG file
./test_cog_reader /tmp/nlcd_2024_cog.tif 100

# Or with any COG
./test_cog_reader /path/to/your/file.tif 50
```

## What It Does

1. Opens your COG file
2. Reads tiles using **two methods**:
   - Standard approach (like QgsRasterBlock + RasterIO)
   - Fast approach (direct GDALReadBlock)
3. Compares performance and reports speedup

## Expected Output

```
=== COG Tile Reader Benchmark ===
File: /tmp/nlcd_2024_cog.tif
Tiles to read: 100

Dataset info:
  Tile size: 512x512
  Tiles: 60x60
  Data type: Byte
  Is tiled: YES
  Overviews: 5

Benchmarking...
  Standard (RasterIO):  100/100 tiles in 450.2 ms
                        4.502 ms/tile

  Fast (ReadBlock):     100/100 tiles in 180.5 ms
                        1.805 ms/tile

=== Results ===
Speedup: 2.49x faster
✓ EXCELLENT! 2x+ speedup achieved
```

## Iterate Quickly

The beauty of this setup:

```bash
# Edit test_cog_reader_standalone.cpp
nano test_cog_reader_standalone.cpp

# Rebuild (3 seconds)
make

# Test again
./test_cog_reader /tmp/nlcd_2024_cog.tif 100
```

No waiting for hours-long QGIS builds!

## Troubleshooting

### "gdal-config: command not found"

Install GDAL:
```bash
brew install gdal
```

### Wrong GDAL version

Check your GDAL:
```bash
gdal-config --version  # Should be 3.0+
```

If using vcpkg's GDAL, update Makefile:
```make
GDAL_CFLAGS = -I/path/to/vcpkg/installed/arm64-osx/include
GDAL_LIBS = -L/path/to/vcpkg/installed/arm64-osx/lib -lgdal
```

## Test Files

Good test COGs:
- NLCD 2024 (byte, paletted, 1.3GB)
- Sentinel-2 B04 (uint16, single-band)
- Copernicus DEM (float32)
- NAIP imagery (4-band RGBA)

Download NLCD:
```bash
wget https://s3-us-west-2.amazonaws.com/mrlc/nlcd_2024_land_cover_l48_20250101.zip
unzip nlcd_2024_land_cover_l48_20250101.zip
cp nlcd_2024_land_cover_l48_20250101.tif /tmp/nlcd_2024_cog.tif
```

## What This Validates

✅ GDALReadBlock is faster than RasterIO for COGs
✅ Tile metadata caching works correctly
✅ Overview selection logic is sound
✅ Ready for GPU integration (Phase 2)

## Next Steps

Once you see 2-3x speedup here:
1. Integrate into QGIS renderer
2. Add GPU texture upload
3. Implement full GPU pipeline

But test THIS first before committing to full QGIS integration!
