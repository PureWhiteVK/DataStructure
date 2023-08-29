cmake_minimum_required(VERSION 3.20.0)
project(data-structure)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

include(extern)

add_subdirectory(src)
add_subdirectory(test)