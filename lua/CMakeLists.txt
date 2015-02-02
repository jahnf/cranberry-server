# lua static lib
# ======================================================================
cmake_minimum_required (VERSION 2.6.0)

project(lualib C) 
    
# lua 5.2 sources
set(lua_srcs lapi.c     lauxlib.c  lbaselib.c lbitlib.c  lcode.c  
             lcorolib.c lctype.c   ldblib.c   ldebug.c   ldo.c      
             ldump.c    lfunc.c    lgc.c      linit.c    liolib.c 
             llex.c     lmathlib.c lmem.c     loadlib.c  lobject.c
             lopcodes.c loslib.c   lparser.c  lstate.c   lstring.c 
             lstrlib.c  ltable.c   ltablib.c  ltm.c      lua.c 
             lundump.c  lvm.c      lzio.c     #luac.c
    )

add_library(lualib STATIC ${lua_srcs})

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set_target_properties(lualib PROPERTIES COMPILE_FLAGS "-D_WIN32" )
    set_property(TARGET lualib APPEND PROPERTY COMPILE_DEFINITIONS "_CRT_SECURE_NO_WARNINGS")
else()
	set_target_properties( lualib PROPERTIES COMPILE_FLAGS "-DLUA_USE_POSIX" )
	target_link_libraries( lualib m )
endif()

