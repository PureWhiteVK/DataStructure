set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)


add_executable(test_integer test_integer.cpp)
target_link_libraries(test_integer PRIVATE misc fmt::fmt)

add_executable(test_sort test_sort.cpp)
target_link_libraries(test_sort PRIVATE misc sort fmt::fmt)

add_executable(test_tree test_tree.cpp)
target_link_libraries(test_tree PRIVATE misc tree fmt::fmt)

add_executable(test_bs_tree test_bs_tree.cpp)
target_link_libraries(test_bs_tree PRIVATE misc tree fmt::fmt)

add_executable(test_tree_set test_tree_set.cpp)
target_link_libraries(test_tree_set PRIVATE tree_set)