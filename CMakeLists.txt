# cranberry-server - a small C web server.
# https://github.com/jahnf/cranberry-server
# ======================================================================

cmake_minimum_required (VERSION 2.6.0)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

project(cranberry-server C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

set(LUA_SUPPORT 1 CACHE BOOL "Build the web server with built in lua support.")
set(SQLITE_SUPPORT 1 CACHE BOOL "Build the web server with built in sqlite support.")
set(DEFLATE_SUPPORT 1 CACHE BOOL "Build the web server with built in deflate support.")
set(LOGGING 1 CACHE BOOL "Build with logging.")

# Set default build type
if( NOT CMAKE_BUILD_TYPE )
  set(CMAKE_BUILD_TYPE Release)
endif()

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

# Define input and output of generated version header and version cmake files
set(VHEADER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/version_string.h.in)
set(VHEADER_DST ${CMAKE_CURRENT_BINARY_DIR}/include/version/version_string.h)
set(VCMAKE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/version_string.cmake.in)
set(VCMAKE_DST ${CMAKE_CURRENT_BINARY_DIR}/version_string.cmake)

# Make sure the version generation is run at configuration time...
include(${CMAKE_CURRENT_SOURCE_DIR}/gitversion.cmake)
# ...so that the version_string.cmake file can be included
# The generated version_string.cmake sets the generated version for the project
# and for CPack
include(${CMAKE_CURRENT_BINARY_DIR}/version_string.cmake)

# Add generation of the version information as custom target
# This ensures the generation of version informatito at compile time and not
# only the configuration time.
add_custom_target(version_header ${CMAKE_COMMAND}
                  -D VHEADER_SRC=${VHEADER_SRC} -D VHEADER_DST=${VHEADER_DST}
                  -D VCMAKE_SRC=${VCMAKE_SRC} -D VCMAKE_DST=${VCMAKE_DST}
                  -P ${CMAKE_CURRENT_SOURCE_DIR}/gitversion.cmake)

# create version header before building the server_version library
add_dependencies(server_version version_header)

# Add a target to generate API documentation with Doxygen
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    message(STATUS "Doxygen found. Adding 'cranberry-doc' target.")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(cranberry-doc
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif(BUILD_TESTS)
