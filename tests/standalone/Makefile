# GPU Raster Rendering - Standalone Benchmark
# Build standalone benchmark without full QGIS build

CXX = clang++
CXXFLAGS = -std=c++17 -O3 -Wall
GDAL_CFLAGS = -I/opt/homebrew/include
GDAL_LIBS = -L/opt/homebrew/lib -lgdal

TARGET = benchmark_gpu_upload

all: $(TARGET)

$(TARGET): benchmark_gpu_upload.cpp
	@echo "Building GPU benchmark..."
	$(CXX) $(CXXFLAGS) $(GDAL_CFLAGS) -o $@ $< $(GDAL_LIBS)
	@echo "✓ Built: $@"

clean:
	rm -f $(TARGET)

.PHONY: all clean
