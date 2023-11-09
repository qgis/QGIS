# CMake version check
cmake_minimum_required(VERSION 3.10.0)

# Default Test
add_executable(Test_RTree
    Test.cpp)
add_dependencies(Test_RTree
    RTree)
target_include_directories(Test_RTree
    PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(Test_RTree
    PRIVATE RTree)
if(MSVC)
    target_compile_options(Test_RTree
        PRIVATE /permissive- /sdl)
    target_compile_definitions(Test_RTree
        PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Memory Test file
add_executable(Test_Memory_RTree
    MemoryTest.cpp)
target_include_directories(Test_Memory_RTree
    PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(Test_Memory_RTree
    PRIVATE RTree)
if(MSVC)
    target_compile_options(Test_Memory_RTree
        PRIVATE /permissive- /sdl)
    target_compile_definitions(Test_Memory_RTree
        PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Bad Data Test file
add_executable(Test_BadData_RTree
    TestBadData.cpp)
target_include_directories(Test_BadData_RTree
    PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(Test_BadData_RTree
    PRIVATE RTree)
if(MSVC)
    target_compile_options(Test_BadData_RTree
        PRIVATE /permissive- /sdl)
    target_compile_definitions(Test_BadData_RTree
        PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
