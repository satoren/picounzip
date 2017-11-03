cmake_minimum_required(VERSION 2.8.11)

project(picounzip)



if(PICOUNZIP_TEST_COVERAGE)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage -coverage -fno-inline -fno-inline-small-functions -fno-default-inline")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall -W -fprofile-arcs -ftest-coverage -coverage -fno-inline -fno-inline-small-functions -fno-default-inline")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -coverage -fno-inline -fno-inline-small-functions -fno-default-inline")
endif(PICOUNZIP_TEST_COVERAGE)


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

if(MSVC11)
# extends tuple size for googletest
# https://blogs.msdn.microsoft.com/vcblog/2011/09/12/c11-features-in-visual-c-11/
add_definitions(-D_VARIADIC_MAX=10)
endif()

add_subdirectory(googletest)
add_subdirectory(tests)