#!/usr/bin/env python3
"""
Create a Cloud Optimized GeoTIFF with continuous values for GPU rendering tests.
Generates synthetic terrain-like data with smooth gradients and noise.
"""

import numpy as np
from osgeo import gdal, osr
import sys

def generate_terrain_like_data(width, height):
    """Generate continuous terrain-like data with gradients and noise."""
    print(f"Generating {width}x{height} continuous data...")

    # Create coordinate grids
    x = np.linspace(0, 10, width)
    y = np.linspace(0, 10, height)
    X, Y = np.meshgrid(x, y)

    # Generate base terrain: sinusoidal hills
    terrain = (
        100 * np.sin(X * 0.5) * np.cos(Y * 0.5) +  # Large hills
        50 * np.sin(X * 2) * np.sin(Y * 2) +        # Medium features
        200                                          # Base elevation
    )

    # Add fine-grained noise for texture
    noise = np.random.normal(0, 5, (height, width))

    # Add medium-scale variation
    medium_noise = np.random.normal(0, 15, (height // 10, width // 10))
    medium_noise = np.repeat(np.repeat(medium_noise, 10, axis=0), 10, axis=1)[:height, :width]

    # Combine all components
    data = terrain + noise + medium_noise

    # Convert to float32
    data = data.astype(np.float32)

    print(f"  Value range: {data.min():.2f} to {data.max():.2f}")
    print(f"  Mean: {data.mean():.2f}, Std: {data.std():.2f}")

    return data

def create_continuous_cog(output_path, width=16000, height=10500, tile_size=512):
    """Create a COG with continuous float values."""

    print(f"\nCreating continuous COG: {output_path}")
    print(f"Dimensions: {width}x{height}")
    print(f"Tile size: {tile_size}x{tile_size}")

    # Generate data
    data = generate_terrain_like_data(width, height)

    # Create in-memory dataset
    print("\nCreating GeoTIFF...")
    mem_driver = gdal.GetDriverByName('MEM')
    mem_ds = mem_driver.Create('', width, height, 1, gdal.GDT_Float32)

    # Set geotransform (arbitrary coordinates)
    mem_ds.SetGeoTransform([
        -180.0,                    # top left x
        360.0 / width,             # w-e pixel resolution
        0,                         # rotation (0 if north is up)
        90.0,                      # top left y
        0,                         # rotation (0 if north is up)
        -180.0 / height            # n-s pixel resolution (negative)
    ])

    # Set projection (WGS84)
    srs = osr.SpatialReference()
    srs.ImportFromEPSG(4326)
    mem_ds.SetProjection(srs.ExportToWkt())

    # Write data
    band = mem_ds.GetRasterBand(1)
    band.WriteArray(data)
    band.SetNoDataValue(-9999)
    band.FlushCache()

    # Create COG with compression
    print("Converting to Cloud Optimized GeoTIFF...")
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

    # Verify it's a valid COG
    print("\nVerifying COG structure...")
    ds = gdal.Open(output_path)

    print(f"  Driver: {ds.GetDriver().ShortName}")
    print(f"  Size: {ds.RasterXSize}x{ds.RasterYSize}")

    band = ds.GetRasterBand(1)
    print(f"  Data type: {gdal.GetDataTypeName(band.DataType)}")
    print(f"  Block size: {band.GetBlockSize()}")
    print(f"  Overviews: {band.GetOverviewCount()}")

    stats = band.GetStatistics(True, True)
    print(f"  Min: {stats[0]:.2f}")
    print(f"  Max: {stats[1]:.2f}")
    print(f"  Mean: {stats[2]:.2f}")
    print(f"  StdDev: {stats[3]:.2f}")

    del ds

    return True

if __name__ == '__main__':
    output_file = '/private/tmp/continuous_terrain_cog.tif'

    # Create full resolution version
    success = create_continuous_cog(
        output_file,
        width=16000,
        height=10500,
        tile_size=512
    )

    if success:
        print(f"\n{'='*60}")
        print("SUCCESS! Continuous COG created")
        print(f"{'='*60}")
        print(f"\nTo test in QGIS:")
        print(f"1. Layer → Add Raster Layer")
        print(f"2. Browse to: {output_file}")
        print(f"3. Click Add")
        print(f"\nLook for 'GPU rendering succeeded' in the log!")
        print(f"\nTo view info:")
        print(f"  gdalinfo {output_file}")
    else:
        print("\nFAILED to create COG")
        sys.exit(1)
