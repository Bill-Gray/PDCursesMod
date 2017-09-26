
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --abbrev=0 --tags
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_LABEL
    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

string( REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+)" VERSION_STRING "${GIT_LABEL}" )

set(PROJECT_VERSION "${VERSION_STRING}")

message(STATUS "Building project version: ${PROJECT_VERSION}")

