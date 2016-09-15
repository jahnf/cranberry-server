find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --always --dirty --tags
                    RESULT_VARIABLE res_var
                    OUTPUT_VARIABLE GIT_COM_ID
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                )
    if( NOT ${res_var} EQUAL 0 )
        set( PROJECT_VERSION "unknown" )
        message( WARNING "Git failed (not a repo, or no tags). Build will not contain git revision info." )
    else()
        string( REPLACE "\n" "" PROJECT_VERSION ${GIT_COM_ID} )
    endif()
else()
    set( PROJECT_VERSION "unknown" )
    message( WARNING "Git not found. Build will not contain git revision info." )
endif()

# Only needed if we set the git tag convention to vX.Y.Z
# If we set the version tag convention to the actual X.Y.Z we can skip it
string(REGEX REPLACE "^v(.*)$" "\\1" PROJECT_VERSION "${PROJECT_VERSION}")

string(REPLACE "." ";" VERSION_LIST ${PROJECT_VERSION})
list(LENGTH VERSION_LIST VERSION_LIST_LEN)
if(${VERSION_LIST_LEN} LESS 2)
    set(PROJECT_VERSION_MAJOR 0)
    set(PROJECT_VERSION_MINOR 0)
    set(PROJECT_VERSION_PATCH "0-${PROJECT_VERSION}")
elseif(${VERSION_LIST_LEN} LESS 3)
    string(REGEX REPLACE "^v?([0-9]+)\\.([0-9]+)\\-?(.*)$" "\\1" PROJECT_VERSION_MAJOR "${PROJECT_VERSION}")
    string(REGEX REPLACE "^v?([0-9]+)\\.([0-9]+)\\-?(.*)$" "\\2" PROJECT_VERSION_MINOR "${PROJECT_VERSION}")
    string(REGEX REPLACE "^v?([0-9]+)\\.([0-9]+)\\-?(.*)$" "0-\\3" PROJECT_VERSION_PATCH "${PROJECT_VERSION}")
else()
    string(REGEX REPLACE "^v?([0-9]+)\\.([0-9]+)\\.([0-9]+)\\-?(.*)$" "\\1" PROJECT_VERSION_MAJOR "${PROJECT_VERSION}")
    string(REGEX REPLACE "^v?([0-9]+)\\.([0-9]+)\\.([0-9]+)\\-?(.*)$" "\\2" PROJECT_VERSION_MINOR "${PROJECT_VERSION}")
    string(REGEX REPLACE "^v?([0-9]+)\\.([0-9]+)\\.(.*)$" "\\3" PROJECT_VERSION_PATCH "${PROJECT_VERSION}")
endif()

set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

string(REGEX REPLACE "^([0-9]+)-?(.*)$" "\\1" PROJECT_VERSION_PATCH_ONLY "${PROJECT_VERSION_PATCH}")
string(REGEX REPLACE "^([0-9]+)-?(.*)$" "\\2" PROJECT_VERSION_TWEAK "${PROJECT_VERSION_PATCH}")

configure_file(${VHEADER_SRC} ${VHEADER_DST} @ONLY)
configure_file(${VCMAKE_SRC} ${VCMAKE_DST} @ONLY)
