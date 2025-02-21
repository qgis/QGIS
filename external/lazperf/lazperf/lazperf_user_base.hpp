
#pragma once

#define LAZPERF_MAJOR_VERSION 1
#define LAZPERF_MINOR_VERSION 3
#define LAZPERF_REVISION 0
#define LAZPERF_VERSION 1.3.0

// Note that this __declspec(dllimport) is not necessary and you can remove it if you like.
// It allows the compiler to make a potential optimization in calling exported functions
// by using an indirect call rather than a call to a thunk which then calls the exported function.
#ifdef _WIN32
#define LAZPERF_EXPORT __declspec(dllimport)
#else
#define LAZPERF_EXPORT
#endif

