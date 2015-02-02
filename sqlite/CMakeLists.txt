# sqlite lib and lua-sqlite3 lib
# ======================================================================
cmake_minimum_required (VERSION 2.6.0)

project(sqliteLibs C) 
    
add_library(sqlite3 STATIC sqlite3.c sqlite3.h sqlite3ext.h)
target_link_libraries(sqlite3 dl)

if(LUA_SUPPORT)
    add_library(lua-sqlite3 STATIC lsqlite3.c)
    target_link_libraries(lua-sqlite3 sqlite3)
endif(LUA_SUPPORT)

