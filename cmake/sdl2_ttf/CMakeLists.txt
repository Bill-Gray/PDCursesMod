CMAKE_MINIMUM_REQUIRED (VERSION 2.8.4)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: DEBUG, RELEASE, or MINSIZEREL." FORCE)
    MESSAGE("No build type specified, defaulting to Release.")
ENDIF()

IF(POLICY CMP0048)
    CMAKE_POLICY(SET CMP0048 NEW)
ENDIF()

SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Name suffix for debug builds")
MARK_AS_ADVANCED(CMAKE_IMPORT_LIBRARY_SUFFIX CMAKE_DEBUG_POSTFIX)

file(READ ${CMAKE_SOURCE_DIR}/SDL2_ttf.spec file_contents)
message(STATUS "file_contents=${file_contents}")
#string( REGEX REPLACE )
#string( REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+)" PROJECT_VERSION "${file_contents}" )

SET(PROJECT_VERSION ${SDL2_TTF_RELEASE})
PROJECT (sdl2_ttf VERSION "${PROJECT_VERSION}" LANGUAGES C)

INCLUDE_DIRECTORIES(
    ${ZLIB_INCLUDE_DIR}
    ${FT2_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR})

LINK_DIRECTORIES(
    ${ZLIB_LIBRARY_DIR}
    ${FT2_LIBRARY_DIR}
    ${SDL2_LIBRARY_DIR})

INSTALL (FILES SDL_ttf.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/SDL2_ttf)

ADD_LIBRARY(SDL2_ttf-static STATIC SDL_ttf.c Version.rc SDL_ttf.h)
INSTALL (TARGETS SDL2_ttf-static 
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    COMPONENT libraries)

ADD_LIBRARY(SDL2_ttf SHARED SDL_ttf.c Version.rc SDL_ttf.h)
TARGET_LINK_LIBRARIES (SDL2_ttf
    debug SDL2maind 
    debug SDL2d 
    debug zlibd 
    debug freetyped
    optimized SDL2main
    optimized SDL2
    optimized zlib
    optimized freetype)

INSTALL (TARGETS SDL2_ttf 
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    COMPONENT libraries)

ADD_EXECUTABLE (showfont ${CMAKE_SOURCE_DIR}/showfont.c)
TARGET_LINK_LIBRARIES (showfont SDL2_ttf-static 
    debug SDL2maind 
    debug SDL2d 
    debug zlibd 
    debug freetyped
    optimized SDL2main
    optimized SDL2
    optimized zlib
    optimized freetype)

INSTALL (TARGETS showfont RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT applications)

ADD_EXECUTABLE (glfont ${CMAKE_SOURCE_DIR}/glfont.c)
TARGET_LINK_LIBRARIES (glfont SDL2_ttf-static
    debug SDL2maind 
    debug SDL2d 
    debug zlibd 
    debug freetyped
    optimized SDL2main
    optimized SDL2
    optimized zlib
    optimized freetype)

INSTALL (TARGETS glfont RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT applications)

set(CPACK_COMPONENTS_ALL libraries applications)
