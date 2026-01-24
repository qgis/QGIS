/***************************************************************************
  test_cog_reader_standalone.cpp - Standalone COG reader benchmark
  --------------------------------------
  Compiles independently from QGIS - fast iteration!
 ***************************************************************************/

#include <iostream>
#include <chrono>
#include <cstring>
#include <cmath>
#include <vector>
#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_conv.h>

// Minimal QByteArray replacement for standalone testing
class SimpleBuffer {
public:
  SimpleBuffer() : mData(nullptr), mSize(0), mCapacity(0) {}
  ~SimpleBuffer() { delete[] mData; }

  void resize(size_t size) {
    if (size > mCapacity) {
      delete[] mData;
      mCapacity = size;
      mData = new char[mCapacity];
    }
    mSize = size;
  }

  char* data() { return mData; }
  const char* constData() const { return mData; }
  size_t size() const { return mSize; }
  bool isEmpty() const { return mSize == 0; }

private:
  char* mData;
  size_t mSize;
  size_t mCapacity;
};

// Standalone COG tile reader (simplified from QgsCOGTileReader)
class FastCOGReader {
public:
  struct TileInfo {
    int width = 0;
    int height = 0;
    int tilesX = 0;
    int tilesY = 0;
    GDALDataType dataType = GDT_Unknown;
    int bytesPerPixel = 0;
    bool isTiled = false;
  };

  FastCOGReader(const char* filename) {
    mDataset = (GDALDataset*)GDALOpen(filename, GA_ReadOnly);
    if (mDataset) {
      initialize();
    }
  }

  ~FastCOGReader() {
    if (mDataset) {
      GDALClose(mDataset);
    }
  }

  bool isValid() const { return mDataset != nullptr && mValid; }

  const TileInfo& tileInfo(int overview = 0) const {
    if (overview >= 0 && overview < (int)mTileInfos.size()) {
      return mTileInfos[overview];
    }
    static TileInfo empty;
    return empty;
  }

  int overviewCount() const { return mOverviewCount; }

  // Fast tile reading using GDALReadBlock
  bool readTile(int overview, int tileX, int tileY, SimpleBuffer& buffer) {
    if (!mDataset || overview < 0 || overview >= (int)mTileInfos.size()) {
      return false;
    }

    const TileInfo& info = mTileInfos[overview];

    GDALRasterBand* band = mDataset->GetRasterBand(1);
    if (!band) return false;

    if (overview > 0) {
      band = band->GetOverview(overview - 1);
      if (!band) return false;
    }

    buffer.resize(info.width * info.height * info.bytesPerPixel);

    if (info.isTiled) {
      // FAST PATH: Use GDALReadBlock for tiled datasets
      CPLErr err = band->ReadBlock(tileX, tileY, buffer.data());
      return err == CE_None;
    } else {
      // Fallback: RasterIO for strip-based
      int xOff = tileX * info.width;
      int yOff = tileY * info.height;
      int xSize = std::min(info.width, band->GetXSize() - xOff);
      int ySize = std::min(info.height, band->GetYSize() - yOff);

      CPLErr err = band->RasterIO(
        GF_Read, xOff, yOff, xSize, ySize,
        buffer.data(), info.width, info.height,
        info.dataType, 0, 0
      );
      return err == CE_None;
    }
  }

private:
  void initialize() {
    GDALRasterBand* band = mDataset->GetRasterBand(1);
    if (!band) {
      mValid = false;
      return;
    }

    // Cache base level info
    cacheTileInfo(band, 0);

    // Cache overview info
    mOverviewCount = band->GetOverviewCount();
    for (int i = 0; i < mOverviewCount; i++) {
      GDALRasterBand* overview = band->GetOverview(i);
      if (overview) {
        cacheTileInfo(overview, i + 1);
      }
    }

    mValid = true;
  }

  void cacheTileInfo(GDALRasterBand* band, int index) {
    TileInfo info;

    int blockX = 0, blockY = 0;
    band->GetBlockSize(&blockX, &blockY);

    info.width = blockX;
    info.height = blockY;
    info.dataType = band->GetRasterDataType();
    info.bytesPerPixel = GDALGetDataTypeSizeBytes(info.dataType);

    int rasterX = band->GetXSize();
    int rasterY = band->GetYSize();

    info.tilesX = (rasterX + blockX - 1) / blockX;
    info.tilesY = (rasterY + blockY - 1) / blockY;
    info.isTiled = (blockX < rasterX) && (blockY > 1);

    while (mTileInfos.size() <= (size_t)index) {
      mTileInfos.push_back(TileInfo());
    }
    mTileInfos[index] = info;
  }

  GDALDataset* mDataset = nullptr;
  bool mValid = false;
  int mOverviewCount = 0;
  std::vector<TileInfo> mTileInfos;
};

// Benchmark helper
class Timer {
public:
  void start() { mStart = std::chrono::high_resolution_clock::now(); }

  double elapsedMs() {
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - mStart).count();
  }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> mStart;
};

// Standard QgsRasterBlock-like approach for comparison
double benchmarkStandardApproach(const char* filename, int tileCount) {
  GDALDataset* dataset = (GDALDataset*)GDALOpen(filename, GA_ReadOnly);
  if (!dataset) return -1.0;

  GDALRasterBand* band = dataset->GetRasterBand(1);
  if (!band) {
    GDALClose(dataset);
    return -1.0;
  }

  int blockX = 0, blockY = 0;
  band->GetBlockSize(&blockX, &blockY);

  int rasterX = band->GetXSize();
  int rasterY = band->GetYSize();
  int tilesX = (rasterX + blockX - 1) / blockX;

  Timer timer;
  timer.start();

  int successCount = 0;
  for (int i = 0; i < tileCount; i++) {
    int tileX = i % tilesX;
    int tileY = i / tilesX;

    int xOff = tileX * blockX;
    int yOff = tileY * blockY;
    int xSize = std::min(blockX, rasterX - xOff);
    int ySize = std::min(blockY, rasterY - yOff);

    // Simulate QgsRasterBlock: allocate buffer + RasterIO
    int bufferSize = blockX * blockY * GDALGetDataTypeSizeBytes(band->GetRasterDataType());
    char* buffer = new char[bufferSize];

    CPLErr err = band->RasterIO(
      GF_Read, xOff, yOff, xSize, ySize,
      buffer, blockX, blockY,
      band->GetRasterDataType(), 0, 0
    );

    if (err == CE_None) successCount++;
    delete[] buffer;
  }

  double elapsed = timer.elapsedMs();
  GDALClose(dataset);

  std::cout << "  Standard (RasterIO):  " << successCount << "/" << tileCount
            << " tiles in " << elapsed << " ms\n";
  std::cout << "                        " << (elapsed / tileCount) << " ms/tile\n";

  return elapsed;
}

// Fast COG reader approach
double benchmarkFastCOGReader(const char* filename, int tileCount) {
  FastCOGReader reader(filename);
  if (!reader.isValid()) return -1.0;

  const auto& info = reader.tileInfo(0);

  Timer timer;
  timer.start();

  SimpleBuffer buffer;
  int successCount = 0;

  for (int i = 0; i < tileCount; i++) {
    int tileX = i % info.tilesX;
    int tileY = i / info.tilesX;

    if (reader.readTile(0, tileX, tileY, buffer)) {
      successCount++;
    }
  }

  double elapsed = timer.elapsedMs();

  std::cout << "  Fast (ReadBlock):     " << successCount << "/" << tileCount
            << " tiles in " << elapsed << " ms\n";
  std::cout << "                        " << (elapsed / tileCount) << " ms/tile\n";

  return elapsed;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <cog_file.tif> [tile_count]\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << argv[0] << " /tmp/nlcd_2024_cog.tif 100\n";
    return 1;
  }

  const char* filename = argv[1];
  int tileCount = (argc >= 3) ? atoi(argv[2]) : 100;

  GDALAllRegister();

  std::cout << "\n=== COG Tile Reader Benchmark ===\n";
  std::cout << "File: " << filename << "\n";
  std::cout << "Tiles to read: " << tileCount << "\n\n";

  // Check file info
  FastCOGReader reader(filename);
  if (!reader.isValid()) {
    std::cerr << "ERROR: Could not open file\n";
    return 1;
  }

  const auto& info = reader.tileInfo(0);
  std::cout << "Dataset info:\n";
  std::cout << "  Tile size: " << info.width << "x" << info.height << "\n";
  std::cout << "  Tiles: " << info.tilesX << "x" << info.tilesY << "\n";
  std::cout << "  Data type: " << GDALGetDataTypeName(info.dataType) << "\n";
  std::cout << "  Is tiled: " << (info.isTiled ? "YES" : "NO") << "\n";
  std::cout << "  Overviews: " << reader.overviewCount() << "\n\n";

  if (!info.isTiled) {
    std::cout << "WARNING: File is not tiled - limited speedup expected\n\n";
  }

  // Limit tile count to available tiles
  int maxTiles = info.tilesX * info.tilesY;
  if (tileCount > maxTiles) {
    tileCount = maxTiles;
    std::cout << "Limiting to " << tileCount << " tiles (all available)\n\n";
  }

  // Benchmark both approaches
  std::cout << "Benchmarking...\n";

  double standardTime = benchmarkStandardApproach(filename, tileCount);
  std::cout << "\n";
  double fastTime = benchmarkFastCOGReader(filename, tileCount);

  std::cout << "\n=== Results ===\n";
  if (standardTime > 0 && fastTime > 0) {
    double speedup = standardTime / fastTime;
    std::cout << "Speedup: " << speedup << "x faster\n";

    if (speedup > 2.0) {
      std::cout << "✓ EXCELLENT! 2x+ speedup achieved\n";
    } else if (speedup > 1.5) {
      std::cout << "✓ GOOD! Significant speedup\n";
    } else if (speedup > 1.1) {
      std::cout << "✓ OK! Modest speedup\n";
    } else {
      std::cout << "⚠ Limited speedup (file may not be COG or not tiled)\n";
    }
  } else {
    std::cout << "ERROR: Benchmark failed\n";
  }

  std::cout << "\n";
  return 0;
}
