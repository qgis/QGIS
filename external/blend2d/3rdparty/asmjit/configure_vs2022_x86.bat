@echo off
cmake . -B build_x86 -G"Visual Studio 17" -A Win32 -DASMJIT_TEST=ON
