cmake_minimum_required(VERSION 3.19)
project(CPP_ThreadPool)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -pthread)
add_executable(CPP_ThreadPool main.cpp TaskQueue.cpp TaskQueue.h)