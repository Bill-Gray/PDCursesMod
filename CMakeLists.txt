CMAKE_MINIMUM_REQUIRED (VERSION 2.8.4)

FIND_PACKAGE(Git REQUIRED)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: DEBUG, RELEASE, or MINSIZEREL." FORCE)
    message("No build type specified, defaulting to Release.")
endif()

OPTION(BUILD_SDL2 "Build SDL2 Library" OFF)


INCLUDE(git_label_version)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif()
#if(POLICY CMP0054)
#    cmake_policy(SET CMP0054 NEW)
#endif()
#if(POLICY CMP0056)
#    cmake_policy(SET CMP0056 NEW)
#endif()

INCLUDE(build_external)

PROJECT (pdcurses VERSION "${PROJECT_VERSION}" LANGUAGES C)

#INCLUDE(gen_config_header)

ADD_SUBDIRECTORY(win32)
ADD_SUBDIRECTORY(win32a)

set(CPACK_COMPONENTS_ALL applications)
