# CMake file for strawberry-ini parser/reader/dictionary tests

cmake_minimum_required (VERSION 2.6.0)
include_directories("../include")

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# for now disable all tests, that use the "check" test framework
# on windows systems - needs further investigation
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_executable(test_ini check_ini.c)
    target_link_libraries(test_ini check strawberryIniStatic)
    add_test("INI.Read.Parse.Dictionary.Tests" test_ini)

    add_custom_target(run_tests COMMAND ${CMAKE_CTEST_COMMAND}
                      DEPENDS test_ini)
 endif()
