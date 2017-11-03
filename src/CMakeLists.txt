

add_library(picounzip picounzip.hpp picounzip.cc)
target_link_libraries(picounzip ${ZLIB_LIBRARIES})

target_include_directories(picounzip PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})