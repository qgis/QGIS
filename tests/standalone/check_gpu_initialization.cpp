// Quick test to verify GPU rendering would work
// Checks if COG is compatible with GPU rendering

#include <gdal_priv.h>
#include <iostream>
#include <string>

void checkCOGCompatibility(const char* filename) {
    GDALAllRegister();

    std::cout << "=== GPU Rendering Compatibility Check ===" << std::endl;
    std::cout << "File: " << filename << std::endl << std::endl;

    GDALDataset* ds = static_cast<GDALDataset*>(GDALOpen(filename, GA_ReadOnly));
    if (!ds) {
        std::cout << "❌ FAILED: Cannot open file" << std::endl;
        return;
    }

    bool compatible = true;

    // Check 1: Is it tiled?
    GDALRasterBand* band = ds->GetRasterBand(1);
    int blockX, blockY;
    band->GetBlockSize(&blockX, &blockY);

    std::cout << "Check 1: Tiled layout" << std::endl;
    std::cout << "  Block size: " << blockX << "x" << blockY << std::endl;

    if (blockX == ds->GetRasterXSize()) {
        std::cout << "  ❌ NOT TILED (strip layout, block width = image width)" << std::endl;
        std::cout << "  GPU rendering will SKIP this file" << std::endl;
        compatible = false;
    } else {
        std::cout << "  ✅ TILED (suitable for GPU)" << std::endl;
    }

    // Check 2: Data type supported?
    GDALDataType dtype = band->GetRasterDataType();
    std::string dtype_name = GDALGetDataTypeName(dtype);

    std::cout << "\nCheck 2: Data type" << std::endl;
    std::cout << "  Type: " << dtype_name << std::endl;

    bool supported_type = (dtype == GDT_Byte || dtype == GDT_UInt16 ||
                           dtype == GDT_Int16 || dtype == GDT_Float32);

    if (supported_type) {
        std::cout << "  ✅ Supported type" << std::endl;
    } else {
        std::cout << "  ⚠️  Type might not be optimized" << std::endl;
    }

    // Check 3: File format
    std::cout << "\nCheck 3: File format" << std::endl;
    std::cout << "  Driver: " << ds->GetDriver()->GetDescription() << std::endl;

    const char* layout = ds->GetMetadataItem("LAYOUT", "IMAGE_STRUCTURE");
    if (layout) {
        std::cout << "  Layout: " << layout << std::endl;
        if (std::string(layout) == "COG") {
            std::cout << "  ✅ Cloud Optimized GeoTIFF" << std::endl;
        }
    }

    // Check 4: Overviews
    int overview_count = band->GetOverviewCount();
    std::cout << "\nCheck 4: Overviews" << std::endl;
    std::cout << "  Count: " << overview_count << std::endl;
    if (overview_count > 0) {
        std::cout << "  ✅ Has overviews (better performance)" << std::endl;
    } else {
        std::cout << "  ⚠️  No overviews (may be slower)" << std::endl;
    }

    // Summary
    std::cout << "\n=== Summary ===" << std::endl;
    if (compatible) {
        std::cout << "✅ This file SHOULD trigger GPU rendering in QGIS" << std::endl;
        std::cout << "\nExpected QGIS log messages:" << std::endl;
        std::cout << "  - 'GPU rendering path enabled for this layer'" << std::endl;
        std::cout << "  - 'GPU rendering completed successfully'" << std::endl;
    } else {
        std::cout << "❌ This file will NOT trigger GPU rendering" << std::endl;
        std::cout << "   GPU rendering requires tiled layout" << std::endl;
    }

    GDALClose(ds);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file.tif>" << std::endl;
        std::cout << "\nTest files:" << std::endl;
        std::cout << "  /private/tmp/continuous_terrain_cog.tif" << std::endl;
        std::cout << "  /private/tmp/nlcd_2024_cog.tif" << std::endl;
        return 1;
    }

    checkCOGCompatibility(argv[1]);
    return 0;
}
