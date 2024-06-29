if(WITH_INTERNAL_NLOHMANN_JSON)
  # Create imported target nlohmann_json::nlohmann_json
  add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
  
  set_target_properties(nlohmann_json::nlohmann_json PROPERTIES
    INTERFACE_COMPILE_DEFINITIONS "\$<\$<NOT:\$<BOOL:ON>>:JSON_USE_GLOBAL_UDLS=0>;\$<\$<NOT:\$<BOOL:ON>>:JSON_USE_IMPLICIT_CONVERSIONS=0>;\$<\$<BOOL:OFF>:JSON_DISABLE_ENUM_SERIALIZATION=1>;\$<\$<BOOL:OFF>:JSON_DIAGNOSTICS=1>;\$<\$<BOOL:OFF>:JSON_USE_LEGACY_DISCARDED_VALUE_COMPARISON=1>"
    INTERFACE_COMPILE_FEATURES "cxx_std_11"
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/external/nlohmann/"
  )
else()
  find_package(nlohmann_json CONFIG REQUIRED)
endif()
