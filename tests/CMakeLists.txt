

add_executable(unzip_from_file_cpp03 unzip_from_file_cpp03.cc)
target_link_libraries(unzip_from_file_cpp03 ${ZLIB_LIBRARIES} gtest_main)
set_target_properties(unzip_from_file_cpp03 PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")


add_executable(unzip_from_file unzip_from_file.cc)
target_link_libraries(unzip_from_file ${ZLIB_LIBRARIES} gtest_main)
set_target_properties(unzip_from_file PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")


add_executable(unzip_from_large_file unzip_from_large_file.cc)
target_link_libraries(unzip_from_large_file ${ZLIB_LIBRARIES} gtest_main)
set_target_properties(unzip_from_large_file PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")


add_executable(deflate_reader deflate_reader.cc)
target_link_libraries(deflate_reader ${ZLIB_LIBRARIES} gtest_main)
set_target_properties(deflate_reader PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(duplicate_sym_check unzip_from_file_header_only1.cc unzip_from_file_header_only2.cc unzip_from_file_header_only2.cc)
target_link_libraries(duplicate_sym_check ${ZLIB_LIBRARIES} gtest_main)
set_target_properties(duplicate_sym_check PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_test(NAME deflate_reader COMMAND deflate_reader WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME unzip_from_file COMMAND unzip_from_file WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME unzip_from_file_cpp03 COMMAND unzip_from_file_cpp03 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME duplicate_sym_check COMMAND duplicate_sym_check WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME unzip_from_large_file COMMAND unzip_from_large_file WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
