cmake_minimum_required(VERSION 2.8.11)

project(picounzip)

if(WIN32)
    add_subdirectory(zlib)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/zlib)
    include_directories(${CMAKE_BINARY_DIR}/zlib) # for zconf.h
    set(ZLIB_LIBRARIES zlibstatic)
else(WIN32)
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
endif(WIN32)

add_subdirectory(src)



include (CTest)
enable_testing()
set(gtest_force_shared_crt TRUE)
add_subdirectory(googletest)
add_subdirectory(tests)