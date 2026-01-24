// Create a COG with continuous float values for GPU testing
#include <gdal_priv.h>
#include <cpl_conv.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

int main() {
    const char* output_path = "/private/tmp/continuous_terrain_cog.tif";
    const int width = 4000;
    const int height = 3000;
    const int tile_size = 512;

    GDALAllRegister();

    std::cout << "Creating continuous terrain COG..." << std::endl;
    std::cout << "Size: " << width << "x" << height << std::endl;

    // Create in-memory dataset
    GDALDriver *mem_driver = GetGDALDriverManager()->GetDriverByName("MEM");
    GDALDataset *mem_ds = mem_driver->Create("", width, height, 1, GDT_Float32, nullptr);

    // Set geotransform
    double geotransform[6] = {-180.0, 360.0/width, 0, 90.0, 0, -180.0/height};
    mem_ds->SetGeoTransform(geotransform);

    // Set projection
    OGRSpatialReference srs;
    srs.importFromEPSG(4326);
    char *wkt = nullptr;
    srs.exportToWkt(&wkt);
    mem_ds->SetProjection(wkt);
    CPLFree(wkt);

    // Generate continuous data
    std::cout << "Generating terrain data..." << std::endl;
    GDALRasterBand *band = mem_ds->GetRasterBand(1);
    band->SetNoDataValue(-9999);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> noise(0.0f, 5.0f);
    std::normal_distribution<float> medium_noise(0.0f, 15.0f);

    std::vector<float> row_data(width);

    for (int y = 0; y < height; y++) {
        if (y % 500 == 0) {
            std::cout << "  Row " << y << "/" << height << std::endl;
        }

        float ny = (float)y / height * 10.0f;

        for (int x = 0; x < width; x++) {
            float nx = (float)x / width * 10.0f;

            // Smooth terrain
            float value = 200.0f;
            value += 100.0f * std::sin(nx * 0.5f) * std::cos(ny * 0.5f);  // Large hills
            value += 50.0f * std::sin(nx * 2.0f) * std::sin(ny * 2.0f);   // Medium features
            value += 20.0f * std::sin(nx * 5.0f) * std::cos(ny * 5.0f);   // Small details
            value += noise(gen);           // Fine noise
            value += medium_noise(gen);    // Medium variation

            row_data[x] = value;
        }

        band->RasterIO(GF_Write, 0, y, width, 1, row_data.data(),
                       width, 1, GDT_Float32, 0, 0);
    }

    band->FlushCache();
    std::cout << "Data generation complete" << std::endl;

    // Compute stats
    double min, max, mean, stddev;
    band->GetStatistics(FALSE, TRUE, &min, &max, &mean, &stddev);
    std::cout << "Value range: " << min << " to " << max << std::endl;
    std::cout << "Mean: " << mean << ", StdDev: " << stddev << std::endl;

    // Create COG
    std::cout << "\nConverting to COG..." << std::endl;
    GDALDriver *cog_driver = GetGDALDriverManager()->GetDriverByName("COG");

    char **options = nullptr;
    options = CSLSetNameValue(options, "COMPRESS", "DEFLATE");
    options = CSLSetNameValue(options, "PREDICTOR", "3");
    options = CSLSetNameValue(options, "BLOCKSIZE", std::to_string(tile_size).c_str());
    options = CSLSetNameValue(options, "OVERVIEWS", "AUTO");
    options = CSLSetNameValue(options, "RESAMPLING", "AVERAGE");
    options = CSLSetNameValue(options, "NUM_THREADS", "ALL_CPUS");

    GDALDataset *cog_ds = cog_driver->CreateCopy(output_path, mem_ds, FALSE, options, nullptr, nullptr);

    CSLDestroy(options);
    GDALClose(mem_ds);

    if (cog_ds) {
        GDALClose(cog_ds);
        std::cout << "\n✓ Created: " << output_path << std::endl;

        // Verify
        GDALDataset *verify_ds = (GDALDataset*)GDALOpen(output_path, GA_ReadOnly);
        if (verify_ds) {
            std::cout << "\nVerification:" << std::endl;
            std::cout << "  Size: " << verify_ds->GetRasterXSize() << "x" << verify_ds->GetRasterYSize() << std::endl;
            GDALRasterBand *b = verify_ds->GetRasterBand(1);
            int bx, by;
            b->GetBlockSize(&bx, &by);
            std::cout << "  Block size: " << bx << "x" << by << std::endl;
            std::cout << "  Overviews: " << b->GetOverviewCount() << std::endl;
            std::cout << "  Data type: " << GDALGetDataTypeName(b->GetRasterDataType()) << std::endl;

            GDALClose(verify_ds);
        }

        return 0;
    } else {
        std::cerr << "Failed to create COG" << std::endl;
        return 1;
    }
}
