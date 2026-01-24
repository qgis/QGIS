#!/usr/bin/env python3
"""
Create a Cloud Optimized GeoTIFF with continuous values for GPU rendering tests.
Uses only GDAL, no numpy required.
"""

from osgeo import gdal, osr
import random
import math

def create_continuous_cog(output_path, width=4000, height=3000, tile_size=512):
    """Create a COG with continuous float values using pure GDAL."""

    print(f"\nCreating continuous COG: {output_path}")
    print(f"Dimensions: {width}x{height}")
    print(f"Tile size: {tile_size}x{tile_size}")

    # Create in-memory dataset
    print("\nGenerating terrain-like data...")
    mem_driver = gdal.GetDriverByName('MEM')
    mem_ds = mem_driver.Create('', width, height, 1, gdal.GDT_Float32)

    # Set geotransform (arbitrary coordinates)
    mem_ds.SetGeoTransform([
        -180.0,                    # top left x
        360.0 / width,             # w-e pixel resolution
        0,                         # rotation
        90.0,                      # top left y
        0,                         # rotation
        -180.0 / height            # n-s pixel resolution (negative)
    ])

    # Set projection (WGS84)
    srs = osr.SpatialReference()
    srs.ImportFromEPSG(4326)
    mem_ds.SetProjection(srs.ExportToWkt())

    # Generate data row by row (memory efficient)
    band = mem_ds.GetRasterBand(1)
    band.SetNoDataValue(-9999)

    print("  Generating pixel values...")
    for y in range(height):
        if y % 500 == 0:
            print(f"    Row {y}/{height}")

        row_data = []
        for x in range(width):
            # Create smooth terrain with multiple frequencies
            # Normalize coordinates to 0-1
            nx = x / width * 10
            ny = y / height * 10

            # Base terrain (smooth hills)
            value = 200.0  # base elevation

            # Large hills
            value += 100.0 * math.sin(nx * 0.5) * math.cos(ny * 0.5)

            # Medium features
            value += 50.0 * math.sin(nx * 2) * math.sin(ny * 2)

            # Small details
            value += 20.0 * math.sin(nx * 5) * math.cos(ny * 5)

            # Fine noise
            value += random.uniform(-5, 5)

            # Medium-scale variation
            value += random.uniform(-15, 15)

            row_data.append(value)

        # Write row
        band.WriteArray([row_data], 0, y)

    band.FlushCache()
    print("  ✓ Data generation complete")

    # Get stats
    print("\nCalculating statistics...")
    stats = band.ComputeStatistics(False)
    print(f"  Min: {stats[0]:.2f}")
    print(f"  Max: {stats[1]:.2f}")
    print(f"  Mean: {stats[2]:.2f}")
    print(f"  StdDev: {stats[3]:.2f}")

    # Create COG
    print("\nConverting to Cloud Optimized GeoTIFF...")
    cog_driver = gdal.GetDriverByName('COG')

    options = [
        'COMPRESS=DEFLATE',
        'PREDICTOR=3',              # Floating point predictor
        f'BLOCKSIZE={tile_size}',
        'OVERVIEWS=AUTO',
        'RESAMPLING=AVERAGE',
        'NUM_THREADS=ALL_CPUS'
    ]

    cog_ds = cog_driver.CreateCopy(output_path, mem_ds, options=options)

    if cog_ds is None:
        print("ERROR: Failed to create COG")
        return False

    # Cleanup
    del cog_ds
    del mem_ds

    print(f"\n✓ Created: {output_path}")

    # Verify
    print("\nVerifying COG structure...")
    ds = gdal.Open(output_path)
    band = ds.GetRasterBand(1)

    print(f"  Driver: {ds.GetDriver().ShortName}")
    print(f"  Size: {ds.RasterXSize}x{ds.RasterYSize}")
    print(f"  Data type: {gdal.GetDataTypeName(band.DataType)}")
    print(f"  Block size: {band.GetBlockSize()}")
    print(f"  Overviews: {band.GetOverviewCount()}")

    del ds

    return True

if __name__ == '__main__':
    output_file = '/private/tmp/continuous_terrain_cog.tif'

    # Create smaller version for faster generation (still plenty of data)
    success = create_continuous_cog(
        output_file,
        width=8000,   # ~2.6M pixels
        height=6000,
        tile_size=512
    )

    if success:
        print(f"\n{'='*60}")
        print("SUCCESS! Continuous terrain COG created")
        print(f"{'='*60}")
        print(f"\nFile: {output_file}")
        print(f"\nTo test in QGIS:")
        print(f"1. Layer → Add Raster Layer")
        print(f"2. Browse to: {output_file}")
        print(f"3. Click Add")
        print(f"\nExpected: Smooth terrain with continuous elevation values")
        print(f"         Each pixel is unique with smooth gradients")
        print(f"\nLook for 'GPU rendering succeeded' in the log!")
    else:
        print("\nFAILED to create COG")
