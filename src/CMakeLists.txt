add_executable(sort sort.cpp)
set_property(TARGET sort PROPERTY CXX_STANDARD 17)
target_link_libraries(sort fmt::fmt)

add_executable(tree tree.cpp)
set_property(TARGET tree PROPERTY CXX_STANDARD 17)
target_link_libraries(tree fmt::fmt)

add_executable(stdio stdio.cpp)
set_property(TARGET stdio PROPERTY CXX_STANDARD 17)