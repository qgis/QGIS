#pragma once

// For exporting symbols from Windows' DLLs
#if defined (_WIN32) && defined(O2_SHARED_LIB)
    #ifdef O2_DLL_EXPORT
        #define O0_EXPORT __declspec(dllexport)
    #else
        #define O0_EXPORT __declspec(dllimport)
    #endif
#else
    #define O0_EXPORT
#endif
