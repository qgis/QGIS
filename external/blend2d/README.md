Blend2D
-------

2D Vector Graphics Powered by a JIT Compiler.

  * [Official Home Page (blend2d.com)](https://blend2d.com)
  * [Official Repository (blend2d/blend2d)](https://github.com/blend2d/blend2d)
  * [Public Chat Channel](https://app.element.io/#/room/#blend2d_blend2d:gitter.im)
  * [Zlib License](./LICENSE.md)

See [blend2d.com](https://blend2d.com) page for more details.

Project Organization
--------------------

  * **`/`**                - Project root, contains this README.md, other markdown files, CMakeLists.txt, and scripts for developers
    * **src**              - Source code (always point include path in here)
      * **blend2d**        - Blend2D source code and headers, all core classes and public headers are here
        * **codec**        - Image codecs and codec utilities (BMP, JPEG, PNG, and QOI)
        * **compression**  - Implementation of compression and decompression algorithms (DEFLATE)
        * **opentype**     - TrueType and OpenType font support
        * **pipeline**     - 2D rendering pipeline definitions and implementations (reference and JIT)
        * **pixelops**     - Pixel operations helpers
        * **raster**       - 2D rendering context implementation
        * **simd**         - SIMD abstraction library Blend2D uses
        * **support**      - Support classes and utilities (arena allocator & containers, bit operations, string operations, ...)
        * **simd**         - SIMD abstraction library Blend2D uses
        * **tables**       - Lookup tables and SIMD constants
        * **threading**    - Multithreading abstraction (POSIX threads or Windows threads)
        * **unicode**      - Support for unicode and conversion between various unicode encodings
    * **testing**          - Testing, benchmarking, samples, and interactive demos (don't embed in your project)
    * **3rdparty**         - Place where to clone [asmjit](https://github.com/asmjit/asmjit) to enable JIT code generation or where to add dependencies for testing and benchmarking purposes

Roadmap
-------

  * See [Roadmap](https://asmjit.com/roadmap.html) page for more details

Documentation
-------------

  * [Documentation Index](https://blend2d.com/doc/index.html)

Contributing Guidelines
-----------------------

  * [CONTRIBUTING.md](./CONTRIBUTING.md)

Building
--------

Blend2D is almost self-containing library. The only dependency is AsmJit, which is only required by when JIT is enabled.

To fetch both Blend2D and AsmJit, type:

```bash
$ git clone https://github.com/blend2d/blend2d
$ git clone https://github.com/asmjit/asmjit blend2d/3rdparty/asmjit
```

Then you can simply configure and build blend2d with CMake:

```bash
$ cmake -S blend2d -B blend2d/build/Release -DCMAKE_BUILD_TYPE=Release -DBLEND2D_TEST=ON
$ cmake --build blend2d/build/Release --config Release
```

And run samples:

```
$ blend2d/build/Release/bl_sample_1
$ blend2d/build/Release/bl_sample_2
...
```

To build Blend2D with interactive demos and with benchmark targeting Qt and other libraries, use `BLEND2D_DEMOS=ON`

```bash
$ cmake -S blend2d -B blend2d/build/Release -DCMAKE_BUILD_TYPE=Release -DBLEND2D_TEST=ON -DBLEND2D_DEMOS=ON
$ cmake --build blend2d/build/Release --config Release
```

Please note that to build interactive demos and benchmarking backends, you need to have Qt installed, in addition to all rendering libraries you would like to benchmark.

For more information, visit [https://blend2d.com/doc/build-instructions.html](Build Instructions)

Bindings
--------

  * [Download Page](https://blend2d.com/download.html#Bindings) provides a list of Blend2D bindings

License
-------

  * [LICENSE.md](./LICENSE.md)
