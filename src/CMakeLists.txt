# cranberry-server. A small C web server application with lua scripting,
# session and sqlite support. https://github.com/jahnf/cranberry-server
# For licensing see LICENSE file or
# https://github.com/jahnf/cranberry-server/blob/master/LICENSE

cmake_minimum_required (VERSION 2.6.0)

# web server sources
set(server_srcs
    # ----------------------------- main and threads
        main.c              # main
        kvlist.c            # key-value list utils
        str_utils.c         # str utilities
        kv_iter.c           # key value string parser/iterator
        http_reply.c        # http reply functions
        http_request.c      # reading http requests
        post_wwwform.c      # http x-www-form post related functions
        post_multipart.c    # http x-www-form post related functions
        http_time.c         # http time helpers
        webthread.c         # main webthread function
        websession.c        # session functionality
        cthreads.c          # wraps pthread and windows basic thread functionality
        cmdline.c           # command option parsing
        optparse.c          # ...
        ip_socket_utils.c   # socket and ip related functions
        log.c               # logging functionality
        settings.c          # settings loading
        cfile.c             # file functions
    # ----------------------------- server commands
        server_commands.c
    # ----------------------------- header
        ../include/cthreads.h
        ../include/cfile.h
    )

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(server_libs ws2_32)
else()
    set(server_libs pthread)
    if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
        list(APPEND server_libs socket nsl rt)
    endif()
endif()

if(SQLITE_SUPPORT)
    list(APPEND server_libs sqlite3)
endif()

if(LUA_SUPPORT)
    # -- at least in cygwin creating luasp as a static library
    # -- causes problems with wrong parameters/ uninialized setting variables..
    #add_library( luasp STATIC luasp.c luasp_cache.c )
    #target_link_libraries(luasp lualib)
    #list(APPEND server_libs luasp)

    # -- this way no problems were discovered so far
    list(APPEND server_srcs luasp.c luasp_reader.c luasp_common.c
                            luasp_cache.c luasp_session.c)
    list(APPEND server_libs lualib)

    if(SQLITE_SUPPORT)
        list(APPEND server_libs lua-sqlite3)
    endif()
endif()

# STR(__SRCFILE_NAME__) in code will be replaced by a string of the
# base filename without path
foreach(f IN LISTS server_srcs)
  get_filename_component(b ${f} NAME)
  set_property(SOURCE ${f} APPEND PROPERTY
    COMPILE_DEFINITIONS "__SRCFILE_NAME__=${b}")
endforeach()

if(DEFLATE_SUPPORT)
    # compile deflate support as static library...
    add_library(miniz STATIC miniz.c)
    list(APPEND server_libs miniz)
    # add definitons for miniz source file
    if(CMAKE_COMPILER_IS_GNUCC)
        set_source_files_properties( miniz.c PROPERTIES
            COMPILE_FLAGS "-Wno-unused-value"
            COMPILE_FLAGS "-Wno-attributes")
    endif()
    set_source_files_properties( miniz.c PROPERTIES
        COMPILE_DEFINITIONS "MINIZ_NO_ARCHIVE_APIS;MINIZ_NO_TIME;MINIZ_NO_STDIO;NO_MINIZ_DECOMPRESS")
endif()

add_library(server_version STATIC version.c)
list(APPEND server_libs server_version)

set_property( SOURCE version.c APPEND PROPERTY COMPILE_DEFINITIONS "GITVERSION")

string(TOLOWER ${CMAKE_BUILD_TYPE} _type)
if(${_type} STREQUAL "debug")
    set_property( SOURCE version.c APPEND PROPERTY COMPILE_DEFINITIONS "BUILD_TYPE_ENUM=2")
elseif(${_type} STREQUAL "release" OR
       ${_type} STREQUAL "minsizerelease" OR
       ${_type} STREQUAL "minsizerel" OR
       ${_type} STREQUAL "relwithdebinfo")
    set_property( SOURCE version.c APPEND PROPERTY COMPILE_DEFINITIONS "BUILD_TYPE_ENUM=1")
else()
    set_property( SOURCE version.c APPEND PROPERTY COMPILE_DEFINITIONS "BUILD_TYPE_ENUM=0")
endif()

add_executable(cranberry-server ${server_srcs})
target_link_libraries(cranberry-server ${server_libs} strawberryIniStatic)

# add resources library
target_link_libraries(cranberry-server server_resourcelib)

if(${CMAKE_SYSTEM_NAME} STREQUAL "CYGWIN")
    set_property(TARGET cranberry-server APPEND PROPERTY COMPILE_DEFINITIONS "_CYGWIN")
endif()

# For release and minsizerelease builds, automatically strip the executable
if (${_type} STREQUAL "release" OR ${_type} STREQUAL "minsizerel")
    if (${CMAKE_C_COMPILER_ID} STREQUAL "Clang" OR
        ${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
        set_target_properties(cranberry-server PROPERTIES LINK_FLAGS "-s")
    endif()
endif()

# Increase supported characters per line in config file
set_property(SOURCE ini_parser.c APPEND PROPERTY COMPILE_DEFINITIONS "INI_MAX_LINE=512")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # windows...
    set_property(TARGET cranberry-server APPEND PROPERTY COMPILE_DEFINITIONS "_CRT_SECURE_NO_WARNINGS")
    set_property(TARGET cranberry-server APPEND PROPERTY COMPILE_DEFINITIONS "_WINDOWS")
else()
    if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR
        "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
        # turn on all warnings in gcc and clang
        set_property(TARGET server_version APPEND PROPERTY COMPILE_FLAGS "-Wall")
        set_property(TARGET cranberry-server APPEND PROPERTY COMPILE_FLAGS "-Wall")
    endif()
    set_property(TARGET cranberry-server APPEND PROPERTY COMPILE_DEFINITIONS "REENTRANT")
endif()
