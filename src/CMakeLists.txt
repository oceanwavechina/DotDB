project(DotDB)
cmake_minimum_required(VERSION 3.6)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -O2 -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -g -std=c++17")


# B+ tree test
set(BPlusTree_test_SRC BPlusTree_test.cpp BPlusTree.cpp)
add_executable(BPlusTree_test ${BPlusTree_test_SRC})

