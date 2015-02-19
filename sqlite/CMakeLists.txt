# sqlite3 lib and lua-sqlite3 lib
# -------------------------------

cmake_minimum_required (VERSION 2.6.0)

project(sqliteLibs C) 

# Static sqlite library
add_library(sqlite3 STATIC sqlite3.c sqlite3.h sqlite3ext.h)
set_property(TARGET sqlite3 APPEND PROPERTY COMPILE_DEFINITIONS "SQLITE_OMIT_TCL_VARIABLE")
target_link_libraries(sqlite3 dl)

string(TOLOWER ${CMAKE_BUILD_TYPE} _type)
if (${_type} STREQUAL "release" OR ${_type} STREQUAL "minsizerel")
    if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR
        "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
        set_target_properties(sqlite3 PROPERTIES COMPILE_FLAGS "-Wno-int-to-pointer-cast -Os")
    endif()
endif()

if(LUA_SUPPORT)
    add_library(lua-sqlite3 STATIC lsqlite3.c)
    target_link_libraries(lua-sqlite3 sqlite3)

    if (${_type} STREQUAL "release" OR ${_type} STREQUAL "minsizerel")
        if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR
            "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
            set_target_properties(lua-sqlite3 PROPERTIES COMPILE_FLAGS "-Os")
        endif()
    endif()
endif(LUA_SUPPORT)

