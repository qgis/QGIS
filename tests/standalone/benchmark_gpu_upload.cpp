/***************************************************************************
  benchmark_gpu_upload.cpp - Simplified GPU texture upload benchmark
  --------------------------------------
  Tests tile reading + simulated GPU upload timing
 ***************************************************************************/

#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <gdal.h>
#include <gdal_priv.h>

using namespace std::chrono;

struct TileInfo
{
    int width = 0;
    int height = 0;
    int tilesX = 0;
    int tilesY = 0;
    int bytesPerPixel = 0;
};

class COGBenchmark
{
public:
    COGBenchmark( const char *filename )
    {
        mDataset = (GDALDataset *)GDALOpen( filename, GA_ReadOnly );
        if ( !mDataset )
        {
            std::cerr << "Failed to open: " << filename << std::endl;
            return;
        }

        GDALRasterBand *band = mDataset->GetRasterBand( 1 );
        if ( !band )
        {
            std::cerr << "No raster band" << std::endl;
            return;
        }

        band->GetBlockSize( &mInfo.width, &mInfo.height );

        const int rasterWidth = mDataset->GetRasterXSize();
        const int rasterHeight = mDataset->GetRasterYSize();

        mInfo.tilesX = ( rasterWidth + mInfo.width - 1 ) / mInfo.width;
        mInfo.tilesY = ( rasterHeight + mInfo.height - 1 ) / mInfo.height;

        GDALDataType dataType = band->GetRasterDataType();
        mInfo.bytesPerPixel = GDALGetDataTypeSizeBytes( dataType );

        mValid = true;

        std::cout << "COG Info:\n";
        std::cout << "  Raster: " << rasterWidth << "x" << rasterHeight << "\n";
        std::cout << "  Tile size: " << mInfo.width << "x" << mInfo.height << "\n";
        std::cout << "  Tiles: " << mInfo.tilesX << "x" << mInfo.tilesY << "\n";
        std::cout << "  Type: " << GDALGetDataTypeName( dataType ) << "\n\n";
    }

    ~COGBenchmark()
    {
        if ( mDataset )
            GDALClose( mDataset );
    }

    bool isValid() const { return mValid; }

    void runBenchmark( int tilesToTest )
    {
        if ( !mValid )
            return;

        std::cout << "=== GPU-Style Rendering Benchmark ===\n";
        std::cout << "Testing " << tilesToTest << " tiles\n\n";

        // Benchmark 1: Tile reading (Phase 1 - validated)
        auto readTime = benchmarkTileReading( tilesToTest );

        // Benchmark 2: Tile reading + simulated GPU upload
        auto uploadTime = benchmarkGPUUpload( tilesToTest );

        // Benchmark 3: Tile reading + CPU image creation
        auto cpuTime = benchmarkCPURendering( tilesToTest );

        // Results
        std::cout << "\n=== Results Summary ===\n";
        std::cout << "Tile reading only:     " << readTime << " ms (" << (readTime / (double)tilesToTest) << " ms/tile)\n";
        std::cout << "Read + GPU upload:     " << uploadTime << " ms (" << (uploadTime / (double)tilesToTest) << " ms/tile)\n";
        std::cout << "Read + CPU rendering:  " << cpuTime << " ms (" << (cpuTime / (double)tilesToTest) << " ms/tile)\n\n";

        if ( cpuTime > 0 && uploadTime > 0 )
        {
            double speedup = (double)cpuTime / uploadTime;
            std::cout << "GPU vs CPU speedup: " << speedup << "x ";

            if ( speedup >= 2.0 )
                std::cout << "✓ EXCELLENT!\n";
            else if ( speedup >= 1.5 )
                std::cout << "✓ GOOD!\n";
            else if ( speedup >= 1.1 )
                std::cout << "~ Modest\n";
            else
                std::cout << "⚠ Needs optimization\n";
        }
    }

private:
    long long benchmarkTileReading( int count )
    {
        std::cout << "--- Benchmark 1: Tile Reading (Phase 1) ---\n";

        GDALRasterBand *band = mDataset->GetRasterBand( 1 );
        std::vector<uint8_t> buffer( mInfo.width * mInfo.height * mInfo.bytesPerPixel );

        auto start = high_resolution_clock::now();

        int success = 0;
        for ( int i = 0; i < count; ++i )
        {
            int tileX = i % mInfo.tilesX;
            int tileY = (i / mInfo.tilesX) % mInfo.tilesY;

            if ( band->ReadBlock( tileX, tileY, buffer.data() ) == CE_None )
                success++;
        }

        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>( end - start ).count();

        std::cout << "  Tiles read: " << success << "/" << count << "\n";
        std::cout << "  Time: " << duration << " ms\n";
        std::cout << "  Rate: " << (1000.0 * success / duration) << " tiles/sec\n\n";

        return duration;
    }

    long long benchmarkGPUUpload( int count )
    {
        std::cout << "--- Benchmark 2: Read + GPU Upload (Simulated) ---\n";

        GDALRasterBand *band = mDataset->GetRasterBand( 1 );
        std::vector<uint8_t> buffer( mInfo.width * mInfo.height * mInfo.bytesPerPixel );

        // Simulated GPU texture storage
        std::vector<std::vector<uint8_t>> gpuTextures;
        gpuTextures.reserve( count );

        auto start = high_resolution_clock::now();

        int success = 0;
        for ( int i = 0; i < count; ++i )
        {
            int tileX = i % mInfo.tilesX;
            int tileY = (i / mInfo.tilesX) % mInfo.tilesY;

            // Read tile
            if ( band->ReadBlock( tileX, tileY, buffer.data() ) == CE_None )
            {
                // Simulate GPU upload (copy to "GPU" memory)
                // In real GPU, this would be glTexImage2D()
                std::vector<uint8_t> gpuBuffer( buffer.begin(), buffer.end() );
                gpuTextures.push_back( std::move( gpuBuffer ) );
                success++;
            }
        }

        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>( end - start ).count();

        std::cout << "  Tiles uploaded: " << success << "/" << count << "\n";
        std::cout << "  Time: " << duration << " ms\n";
        std::cout << "  Rate: " << (1000.0 * success / duration) << " tiles/sec\n\n";

        return duration;
    }

    long long benchmarkCPURendering( int count )
    {
        std::cout << "--- Benchmark 3: Read + CPU Rendering ---\n";

        GDALRasterBand *band = mDataset->GetRasterBand( 1 );
        std::vector<uint8_t> buffer( mInfo.width * mInfo.height * mInfo.bytesPerPixel );

        // Simulated CPU image rendering
        struct CPUImage
        {
            std::vector<uint32_t> pixels;  // RGBA
            int width, height;
        };

        std::vector<CPUImage> renderedImages;
        renderedImages.reserve( count );

        auto start = high_resolution_clock::now();

        int success = 0;
        for ( int i = 0; i < count; ++i )
        {
            int tileX = i % mInfo.tilesX;
            int tileY = (i / mInfo.tilesX) % mInfo.tilesY;

            // Read tile
            if ( band->ReadBlock( tileX, tileY, buffer.data() ) == CE_None )
            {
                // Simulate CPU rendering: convert byte data to RGBA
                CPUImage img;
                img.width = mInfo.width;
                img.height = mInfo.height;
                img.pixels.resize( mInfo.width * mInfo.height );

                for ( int p = 0; p < mInfo.width * mInfo.height; ++p )
                {
                    uint8_t val = buffer[p];
                    // Grayscale to RGBA
                    img.pixels[p] = (0xFF << 24) | (val << 16) | (val << 8) | val;
                }

                renderedImages.push_back( std::move( img ) );
                success++;
            }
        }

        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>( end - start ).count();

        std::cout << "  Tiles rendered: " << success << "/" << count << "\n";
        std::cout << "  Time: " << duration << " ms\n";
        std::cout << "  Rate: " << (1000.0 * success / duration) << " tiles/sec\n\n";

        return duration;
    }

    GDALDataset *mDataset = nullptr;
    TileInfo mInfo;
    bool mValid = false;
};

int main( int argc, char *argv[] )
{
    if ( argc < 2 )
    {
        std::cout << "Usage: " << argv[0] << " <cog_file> [num_tiles]\n";
        std::cout << "Example: " << argv[0] << " /private/tmp/nlcd_2024_cog.tif 100\n";
        return 1;
    }

    const char *cogFile = argv[1];
    const int tilesToTest = ( argc > 2 ) ? atoi( argv[2] ) : 100;

    GDALAllRegister();

    COGBenchmark benchmark( cogFile );
    if ( benchmark.isValid() )
    {
        benchmark.runBenchmark( tilesToTest );
    }

    return 0;
}
