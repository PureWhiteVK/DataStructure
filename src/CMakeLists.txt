add_library(misc INTERFACE)
target_include_directories(misc INTERFACE misc)
set_property(TARGET misc PROPERTY CXX_STANDARD 17)
set_property(TARGET misc PROPERTY CXX_STANDARD_REQUIRED TRUE)

add_library(sort INTERFACE)
target_include_directories(sort INTERFACE sort)
set_property(TARGET sort PROPERTY CXX_STANDARD 17)
set_property(TARGET sort PROPERTY CXX_STANDARD_REQUIRED TRUE)

add_library(tree STATIC)
target_sources(tree PUBLIC 
  tree/avl_tree.cpp 
  tree/bs_tree.cpp 
  tree/rb_tree.cpp
)
target_include_directories(tree PUBLIC tree)
set_property(TARGET tree PROPERTY CXX_STANDARD 17)
set_property(TARGET tree PROPERTY CXX_STANDARD_REQUIRED TRUE)