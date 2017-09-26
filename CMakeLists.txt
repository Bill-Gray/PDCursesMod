CMAKE_MINIMUM_REQUIRED (VERSION 2.8.4)

FIND_PACKAGE(Git REQUIRED)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: DEBUG, RELEASE, or MINSIZEREL." FORCE)
    message("No build type specified, defaulting to Release.")
endif()

option(PDC_DLL_BUILD "Dynamic Library" OFF)
option(PDC_UTF8 "Force to UTF8" OFF)
option(PDC_WIDE "Unicode" OFF)
option(PDCDEBUG "Debug" OFF)
option(PDC_CHTYPE_32 "CHTYPE_32" OFF)
option(PDC_CHTYPE_16 "CHTYPE_16" OFF)
option(PDC_SDL2_BUILD "Build SDL2 bits" ON)
option(BUILD_SDL2 "Build SDL2 Library" ON)

INCLUDE(git_label_version)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif()

if (BUILD_SDL2)
    INCLUDE(build_dependents)
endif ()

PROJECT (pdcurses VERSION "${PROJECT_VERSION}" LANGUAGES C)

INCLUDE(gen_config_header)

IF(CYGWIN)
    MESSAGE(STATUS "Windows Kit um lib dir = ${WINDOWS_KIT_LIBRARY_DIR}")
    LINK_DIRECTORIES(${WINDOWS_KIT_LIBRARY_DIR})
ENDIF()

ADD_SUBDIRECTORY(win32)
ADD_SUBDIRECTORY(win32a)
if (PDC_SDL2_BUILD)
    ADD_SUBDIRECTORY(sdl2)
endif ()

set(CPACK_COMPONENTS_ALL applications)
