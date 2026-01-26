# GPU Raster Rendering Benchmark

Standalone benchmark for measuring GPU raster rendering performance without building full QGIS.

## Quick Start

```bash
cd tests/standalone
make
./benchmark_gpu_upload /path/to/your.tif 100
```

## What It Tests

Compares GPU texture upload vs CPU rendering performance:
- Fast COG tile reading (GDALReadBlock)
- Direct GPU texture upload
- CPU rendering (baseline comparison)

## Expected Output

```
=== GPU Rendering Benchmark ===
File: your_file.tif
Tiles: 100

Read + GPU upload:    10.2 ms (0.102 ms/tile)
Read + CPU rendering: 32.5 ms (0.325 ms/tile)

GPU vs CPU speedup: 3.2x faster ✓ EXCELLENT!
```

## Requirements

- GDAL 3.0+
- COG (Cloud Optimized GeoTIFF) test file

Install GDAL:
```bash
brew install gdal
```

## Using vcpkg's GDAL

Edit Makefile to use vcpkg paths:
```make
GDAL_CFLAGS = -I/path/to/vcpkg/installed/arm64-osx/include
GDAL_LIBS = -L/path/to/vcpkg/installed/arm64-osx/lib -lgdal
```
