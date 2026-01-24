# Fast COG Reader Standalone Test - Build Configuration
# Rename to Makefile or use: make -f build.mk

CXX = clang++
CXXFLAGS = -std=c++17 -O3 -Wall
GDAL_CFLAGS = -I/opt/homebrew/include
GDAL_LIBS = -L/opt/homebrew/lib -lgdal

TARGET = test_cog_reader
SOURCE = test_cog_reader_standalone.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	@echo "Building standalone COG reader test..."
	$(CXX) $(CXXFLAGS) $(GDAL_CFLAGS) -o $(TARGET) $(SOURCE) $(GDAL_LIBS)
	@echo "✓ Build complete! Binary: ./$(TARGET)"

clean:
	rm -f $(TARGET)

test: $(TARGET)
	@echo "\n=== Running test ==="
	@if [ -f /private/tmp/nlcd_2024_cog.tif ]; then \
		./$(TARGET) /private/tmp/nlcd_2024_cog.tif 100; \
	else \
		echo "Place test COG at /private/tmp/nlcd_2024_cog.tif"; \
	fi

.PHONY: all clean test
