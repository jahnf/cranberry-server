# lua static lib
# ======================================================================
cmake_minimum_required (VERSION 2.6.0)

project(lualib C) 
    
# lua 5.3 sources
set(lua_srcs  lapi.c      lctype.c  lgc.c       loadlib.c   lstring.c  #luac.c
              lauxlib.c   ldblib.c  linit.c     lobject.c   lstrlib.c  lundump.c
              lbaselib.c  ldebug.c  liolib.c    lopcodes.c  ltable.c   lutf8lib.c
              lbitlib.c   ldo.c     llex.c      loslib.c    ltablib.c  lvm.c
              lcode.c     ldump.c   lmathlib.c  lparser.c   ltm.c      lzio.c
              lcorolib.c  lfunc.c   lmem.c      lstate.c    lua.c
    )

add_library(lualib STATIC ${lua_srcs})

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_property(TARGET lualib APPEND PROPERTY COMPILE_DEFINITIONS "_WIN32")
    set_property(TARGET lualib APPEND PROPERTY COMPILE_DEFINITIONS "_CRT_SECURE_NO_WARNINGS")
else()
    set_property(TARGET lualib APPEND PROPERTY COMPILE_DEFINITIONS "LUA_USE_POSIX")
    target_link_libraries( lualib m )
endif()
