# QUICKSTART: Test COG Reader in 3 Commands

## The Fastest Path

```bash
cd /Users/w/Documents/Github/QGIS/tests/standalone
./test_both_cogs.sh
```

**That's it!** This will:
1. ✅ Create float32 COG (if needed)
2. ✅ Build test program (~3 seconds)
3. ✅ Test both byte and float32 COGs
4. ✅ Show speedup results

## What You'll See

```
=== COG Performance Comparison: Byte vs Float32 ===

Building test program...
✓ Build complete! Binary: ./test_cog_reader

==========================================
TEST 1: Byte COG (original)
==========================================

=== COG Tile Reader Benchmark ===
File: /private/tmp/nlcd_2024_cog.tif

Dataset info:
  Tile size: 512x512
  Tiles: 312x205
  Is tiled: YES
  Overviews: 6

  Standard (RasterIO):  100/100 tiles in 450 ms
  Fast (ReadBlock):     100/100 tiles in 180 ms

Speedup: 2.5x faster
✓ EXCELLENT! 2x+ speedup achieved

==========================================
TEST 2: Float32 COG (converted)
==========================================

=== COG Tile Reader Benchmark ===
File: /private/tmp/nlcd_2024_float_cog.tif

Dataset info:
  Tile size: 512x512
  Tiles: 312x205
  Is tiled: YES
  Overviews: 6

  Standard (RasterIO):  100/100 tiles in 520 ms
  Fast (ReadBlock):     100/100 tiles in 210 ms

Speedup: 2.48x faster
✓ EXCELLENT! 2x+ speedup achieved

=== Summary ===
Both tests complete. Compare the speedup values above.
```

## Individual Commands (if needed)

### Test just the byte COG

```bash
cd /Users/w/Documents/Github/QGIS/tests/standalone
./quick_test.sh /private/tmp/nlcd_2024_cog.tif 100
```

### Create float COG manually

```bash
cd /Users/w/Documents/Github/QGIS/tests/standalone
./create_float_cog.sh
```

### Test with your own COG

```bash
cd /Users/w/Documents/Github/QGIS/tests/standalone
cp build.mk Makefile
make
./test_cog_reader /path/to/your/file.tif 100
```

## Troubleshooting

**"gdal-config: command not found"**
```bash
brew install gdal
```

**"Makefile not found"**
```bash
cp build.mk Makefile
```

**Float COG creation fails (no space)**
- Float32 is ~4-5x larger than byte
- Free up ~5-6 GB in /private/tmp
- Or test with byte COG only (still validates approach)

## Expected Results

| Scenario | Expected Speedup |
|----------|------------------|
| Byte COG (NLCD) | 2-3x faster |
| Float32 COG | 2-3x faster |
| Any properly tiled COG | 2-3x faster |

## If You See Good Results

✅ **Approach is validated!**
→ Ready for Phase 2 (GPU integration)

## If Speedup is Low (<1.5x)

⚠️ Check file structure:
```bash
/opt/homebrew/bin/gdalinfo /private/tmp/nlcd_2024_cog.tif | grep -E "LAYOUT|Block="
```

Should show:
- `LAYOUT=COG`
- `Block=512x512` (or 256x256)

If not, file may not be a true COG.

---

**GO!** Run `./test_both_cogs.sh` now!
