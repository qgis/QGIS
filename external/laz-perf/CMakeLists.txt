
set(SRCS
    charbuf.cpp
    lazperf.cpp
    filestream.cpp
    io.cpp
    vlr.cpp
    detail/field_point10.cpp
    detail/field_gpstime10.cpp
    detail/field_rgb10.cpp
    detail/field_byte10.cpp
    detail/field_rgb14.cpp
    detail/field_nir14.cpp
    detail/field_point14.cpp
    detail/field_byte14.cpp
)

if (NOT EMSCRIPTEN)
    add_library(${LAZPERF_SHARED_LIB} SHARED ${SRCS})
    lazperf_target_compile_settings(${LAZPERF_SHARED_LIB})
endif()

add_library(${LAZPERF_STATIC_LIB} STATIC ${SRCS})
lazperf_target_compile_settings(${LAZPERF_STATIC_LIB})

