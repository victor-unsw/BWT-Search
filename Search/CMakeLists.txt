cmake_minimum_required(VERSION 3.2)
project(Search)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(bwtsearch inc/preprocess.h src/preprocess.cpp inc/FBucket.h src/FBucket.cpp inc/FidoSearch.h src/FidoSearch.cpp inc/pool.h src/pool.cpp)
add_executable(Search ${SOURCE_FILES} inc/preprocess.h src/preprocess.cpp inc/FBucket.h src/FBucket.cpp inc/FidoSearch.h src/FidoSearch.cpp inc/pool.h src/pool.cpp)