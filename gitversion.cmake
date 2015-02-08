
#find_package(Git)
#if(GIT_FOUND)
#  message("git found: ${GIT_EXECUTABLE}")

#  MACRO(GIT_DESCRIBE dir prefix)
#    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} describe  --always --dirty --long
#      OUTPUT_VARIABLE ${prefix}_VERSION_STRING
#      ERROR_VARIABLE GIT_DESCRIBE_ERROR
#      RESULT_VARIABLE GIT_DESCRIBE_RESULT
#      WORKING_DIRECTORY ${dir}
#      OUTPUT_STRIP_TRAILING_WHITESPACE)
#      
#      IF( NOT GIT_DESCRIBE_ERROR STREQUAL "" )
#        MESSAGE(WARNING "Git error: ${GIT_DESCRIBE_ERROR}")
#      ENDIF( NOT GIT_DESCRIBE_ERROR STREQUAL "" )
#      
#    ENDMACRO(GIT_DESCRIBE dir prefix)
#endif()

#create a pretty commit id using git
#uses 'git describe --tags', so tags are required in the repo
#create a tag with 'git tag <name>' and 'git push --tags'

find_package(Git)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --always --dirty --long 
                    RESULT_VARIABLE res_var 
                    OUTPUT_VARIABLE GIT_COM_ID
                    WORKING_DIRECTORY ${SOURCE_DIR})
    if( NOT ${res_var} EQUAL 0 )
        set( GIT_COMMIT_ID "git commit id unknown")
        message( WARNING "Git failed (not a repo, or no tags). Build will not contain git revision info." )
    endif()
    string( REPLACE "\n" "" GIT_COMMIT_ID ${GIT_COM_ID} )
else()
    set( GIT_COMMIT_ID "unknown (git not found!)")
    message( WARNING "Git not found. Build will not contain git revision info." )
endif()

set( vstring "//version_string.h - auto generated\n"
             "#define VERSION_STRING \"${GIT_COMMIT_ID}\"\;\n")

file(WRITE version_string.h.tmp ${vstring} )
# copy the file to the final header only if the version changes
# reduces needless rebuilds
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        version_string.h.tmp ${CMAKE_CURRENT_BINARY_DIR}/include/version_string.h)

