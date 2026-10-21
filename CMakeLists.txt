cmake_minimum_required(VERSION 3.16)
project(FileCompressionTool VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ZLIB REQUIRED)

add_library(compression_core
    src/zip_archive.cpp
    src/zip_archive.hpp
    src/huffman.cpp
    src/huffman.hpp
)

target_include_directories(compression_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(compression_core PUBLIC ZLIB::ZLIB)

if (MSVC)
    target_compile_options(compression_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(compression_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(file-compress src/main.cpp)
target_link_libraries(file-compress PRIVATE compression_core)

add_executable(smoke_test tests/smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE compression_core)

enable_testing()
add_test(NAME smoke_test COMMAND smoke_test)
