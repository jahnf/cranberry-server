# CMake file for strawberry-ini examples

cmake_minimum_required (VERSION 2.6.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${strawberry-ini_BINARY_DIR}/bin)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/example-01.ini 
               ${CMAKE_CURRENT_BINARY_DIR}/example-01.ini COPYONLY)

add_executable( example-01 example-01.c )
target_link_libraries( example-01 strawberryIniStatic )
