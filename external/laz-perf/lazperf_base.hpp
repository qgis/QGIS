
#pragma once

#define LAZPERF_MAJOR_VERSION 1
#define LAZPERF_MINOR_VERSION 3
#define LAZPERF_REVISION 0
#define LAZPERF_VERSION 1.3.0

#ifdef _WIN32
#define LAZPERF_EXPORT __declspec(dllexport)
#else
// This may not be necessary. The GCC doc says it take __declspec((dllexport))
#define LAZPERF_EXPORT __attribute__((visibility ("default")))
#endif

