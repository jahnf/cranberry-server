find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --always --dirty --tags
                    RESULT_VARIABLE res_var 
                    OUTPUT_VARIABLE GIT_COM_ID
                    WORKING_DIRECTORY ${SOURCE_DIR})
    if( NOT ${res_var} EQUAL 0 )
        set( GIT_COMMIT_ID "unknown")
        message( WARNING "Git failed (not a repo, or no tags). Build will not contain git revision info." )
    else()
        string( REPLACE "\n" "" GIT_COMMIT_ID ${GIT_COM_ID} )
    endif()
else()
    set( GIT_COMMIT_ID "unknown")
    message( WARNING "Git not found. Build will not contain git revision info." )
endif()

set( vstring "//version_string.h - auto generated\n"
              "#define VERSION_STRING \"${GIT_COMMIT_ID}\"\;\n")

file(WRITE version_string.h.tmp ${vstring} )
# copy the file to the final header only if the version changes
# reduces needless rebuilds
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        version_string.h.tmp ${CMAKE_CURRENT_BINARY_DIR}/include/version_string.h)

