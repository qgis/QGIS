Output cl2.hpp Header from OpenCL-CLHPP Project
===============================================

Copied here to QGIS project on 2018-09-30 from source tree bbccc50 commit.

https://github.com/KhronosGroup/OpenCL-CLHPP

During CMake build, header is directly copied from source's 'input_cl2.hpp'.

Useful for platforms where v2 C++ bindings are not provided with the default
OpenCL installation, e.g. Mac.



OpenCL-CLHPP Project README (below)
===================================

Sources for the OpenCL Host API C++ bindings (cl.hpp and cl2.hpp).

Doxgen documentation for the cl2.hpp header is available here:

  http://khronosgroup.github.io/OpenCL-CLHPP/


Components:
  input_cl.hpp:
    Acts as the master source for the 1.x version of the header.
    The reason for doing it this way is to generate an appropriate set of functors with varying argument counts without assuming variadic template support in the header.

  input_cl2.hpp:
    Acts as the master source for the 2.x version of the header.
    Directly copied as cl2.hpp

  gen_cl_hpp.py:
    A generator script written in python to convert input_cl.hpp into cl.hpp, generating the functor expansions as necessary.
    cl2.hpp does not require this as it uses variadic templates expanded in the compiler.

  docs:
    Doxygen file used to generate HTML documentation for cl2.hpp.

  examples:
    A simple example application using the very basic features of the header and generating cl.hpp dynamically through the build system.

  tests:
    A (very small, incomplete) set of regression tests. Building the tests requires Python, Ruby, Unity and CMock. For the last two I've used Unity 2.1.0 [1] and CMock top-of-tree from Github [2] (the version 2.0.204 on Sourceforge does not work). At the moment there is no way to skip building the tests.

  CMake scripts:
    A build system that both generates the example and drives generation of cl.hpp.

You need to tell cmake where to find external dependencies, using the variables OPENCL_DIST_DIR, UNITY_DIR and CMOCK_DIR. These can be set either as environment variables, or on the cmake command line using the syntax -D<VAR>=<VALUE>. For the lazy, I use the following commands to build and test (you'll need to adapt your paths):

mkdir build
cd build
cmake -DUNITY_DIR=$HOME/src/unity -DCMOCK_DIR=$HOME/src/cmock -DOPENCL_DIST_DIR=/opt/AMD-APP-SDK-v2.7-RC-lnx64/ ..
make
tests/test_clhpp
tests/test_clhpp_cxx11
tests/test_clhpp_deprecated_1_1

After building, the headers appear in build/include/CL/. If Doxygen is available, you can generate HTML documentation by typing `make docs`.

[1] https://github.com/ThrowTheSwitch/Unity/releases/tag/v2.1.0
[2] https://github.com/ThrowTheSwitch/CMock
