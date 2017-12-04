cmake_minimum_required(VERSION 3.8)
project(P4)

set(CMAKE_CXX_STANDARD 11)

find_package (Threads)
set(SOURCE_FILES main.c lock.h counter.h list.h hash.h lock.c counter.c list.c hash.c)
add_executable(P4 ${SOURCE_FILES})
target_link_libraries (P4 ${CMAKE_THREAD_LIBS_INIT})