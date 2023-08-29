add_library(misc INTERFACE)
target_include_directories(misc INTERFACE misc)
set_property(TARGET misc PROPERTY CMAKE_CXX_STANDARD 17)
set_property(TARGET misc PROPERTY CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(sort INTERFACE)
target_include_directories(sort INTERFACE sort)
set_property(TARGET sort PROPERTY CMAKE_CXX_STANDARD 17)
set_property(TARGET sort PROPERTY CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(tree INTERFACE)
target_include_directories(tree INTERFACE tree)
set_property(TARGET tree PROPERTY CMAKE_CXX_STANDARD 17)
set_property(TARGET tree PROPERTY CMAKE_CXX_STANDARD_REQUIRED TRUE)