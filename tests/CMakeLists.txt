cmake_minimum_required (VERSION 2.6.0)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

include_directories("../include")

# for now disable all tests, that use the "check" test framework on Windows
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_executable(test_kv_iter check_kviter.c ../src/kv_iter.c)
    target_link_libraries(test_kv_iter check)
    
    add_executable(test_cthread check_cthreads.c ../src/cthreads.c)
    target_link_libraries(test_cthread check)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_link_libraries(test_cthread pthread)
    endif()

    add_test("KeyValue.Iterator.Tests" test_kv_iter)
    add_test("CThread.Tests" test_cthread)
endif()
