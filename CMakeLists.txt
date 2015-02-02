# cranberry-server
# ======================================================================

cmake_minimum_required (VERSION 2.6.0)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

project(cranberry-server C) 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

set(LUA_SUPPORT 1 CACHE BOOL "Build the web server with built in lua support.")
set(SQLITE_SUPPORT 1 CACHE BOOL "Build the web server with built in sqlite support.")
set(DEFLATE_SUPPORT 1 CACHE BOOL "Build the web server with built in deflate support.")
set(LOGGING 1 CACHE BOOL "Build in logging.")

# Build lua static lib
if(LUA_SUPPORT)
    include_directories(lua)
    add_subdirectory(lua)
endif(LUA_SUPPORT)

# Build sqlite support libs
if(SQLITE_SUPPORT)
    include_directories(sqlite)
    add_subdirectory(sqlite)
endif(SQLITE_SUPPORT)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in" 
               "${CMAKE_CURRENT_BINARY_DIR}/include/config.h")

# Include directories
include_directories(include)
include_directories(mkcres)
include_directories(strawberry-ini/include)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")

# Create resource library
include(mkcres/mkcres.cmake)
mkcres_add_library(server_resourcelib resources/resources.json mkcres)

# Add subdirectory projects
add_subdirectory(strawberry-ini)
add_subdirectory(src)
