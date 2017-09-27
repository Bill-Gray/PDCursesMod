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

set(SDL2_ttf_source_files SDL_ttf.c)

IF(WIN32)
    LIST(INSERT SDL2_ttf_source_files 0 Version.rc)
ENDIF()    

ADD_LIBRARY(SDL2_ttf-static STATIC ${SDL2_ttf_source_files} SDL_ttf.h)
INSTALL (TARGETS SDL2_ttf-static 
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    COMPONENT libraries)

ADD_LIBRARY(SDL2_ttf SHARED ${SDL2_ttf_source_files} SDL_ttf.h)
IF(WIN32)
TARGET_LINK_LIBRARIES (SDL2_ttf
    debug SDL2maind 
    debug SDL2d 
    debug freetyped
    debug zlibd
    optimized SDL2main
    optimized SDL2
    optimized freetype
    optimized zlib)
ELSE()
TARGET_LINK_LIBRARIES (SDL2_ttf z dl png sndio bz2
    debug SDL2maind
    debug SDL2-2.0d
    debug freetyped
    optimized SDL2main
    optimized SDL2-2.0
    optimized freetype)
ENDIF()

INSTALL (TARGETS SDL2_ttf
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib    
    COMPONENT libraries)

ADD_EXECUTABLE (showfont ${CMAKE_SOURCE_DIR}/showfont.c)
IF(WIN32)
TARGET_LINK_LIBRARIES (showfont SDL2_ttf
    debug SDL2maind 
    debug SDL2d 
    debug freetyped
    optimized SDL2main
    optimized SDL2
    optimized freetype)
ELSE()
TARGET_LINK_LIBRARIES (showfont SDL2_ttf z dl png sndio bz2
    debug SDL2maind 
    debug SDL2-2.0d 
    debug freetyped
    optimized SDL2main
    optimized SDL2-2.0
    optimized freetype)
ENDIF()

INSTALL (TARGETS showfont RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT applications)

ADD_EXECUTABLE (glfont ${CMAKE_SOURCE_DIR}/glfont.c)
IF(WIN32)
TARGET_LINK_LIBRARIES (glfont SDL2_ttf
    debug SDL2maind 
    debug SDL2d 
    debug freetyped
    optimized SDL2main
    optimized SDL2
    optimized freetype)
ELSE()
TARGET_LINK_LIBRARIES (glfont SDL2_ttf z dl png sndio bz2
    debug SDL2maind 
    debug SDL2-2.0d 
    debug freetyped
    optimized SDL2main
    optimized SDL2-2.0
    optimized freetype)
ENDIF()

INSTALL (TARGETS glfont RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT applications)

set(CPACK_COMPONENTS_ALL libraries applications)
