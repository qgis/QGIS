@echo off
cmake . -B build_x64 -G"Visual Studio 17" -A x64 -DBLEND2D_TEST=ON
