project(PDCurses C)
cmake_minimum_required(VERSION 2.8)

#read version from curses.h
file(READ ${CMAKE_SOURCE_DIR}/curses.h _curses_h_contents)

string(REGEX REPLACE ".*#define[ \t]+PDC_VER_MAJOR[ \t]+([-0-9A-Za-z.]+).*" "\\1" VERSION_MAJOR ${_curses_h_contents})
string(REGEX REPLACE ".*#define[ \t]+PDC_VER_MINOR[ \t]+([-0-9A-Za-z.]+).*" "\\1" VERSION_MINOR ${_curses_h_contents})
string(REGEX REPLACE ".*#define[ \t]+PDC_VER_CHANGE[ \t]+([-0-9A-Za-z.]+).*" "\\1" VERSION_PATCH ${_curses_h_contents})
    
set(PACKAGE_NAME    ${PROJECT_NAME})
set(PACKAGE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
set(PACKAGE_STRING  ${CMAKE_PROJECT_NAME}-${PACKAGE_VERSION})
set(PACKAGE_BUGREPORT wmcbrine@gmail.com)

MESSAGE(STATUS "PACKAGE_NAME = ${PACKAGE_NAME}")
MESSAGE(STATUS "PACKAGE_VERSION = ${PACKAGE_VERSION}")
MESSAGE(STATUS "PACKAGE_STRING = ${PACKAGE_STRING}")
MESSAGE(STATUS "PACKAGE_BUGREPORT = ${PACKAGE_BUGREPORT}")

# Allow the developer to select if Dynamic or Static libraries are built
OPTION (PDCURSES_BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
# Set the LIB_TYPE variable to STATIC
SET (LIB_TYPE STATIC)
IF (PDCURSES_BUILD_SHARED_LIBS)
  # User wants to build Dynamic Libraries, so change the LIB_TYPE variable to CMake keyword 'SHARED'
  SET (LIB_TYPE SHARED)
  IF (WIN32)
      set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -DPDC_DLL_BUILD")
  ENDIF()
ENDIF (PDCURSES_BUILD_SHARED_LIBS)

option(ENABLE_RGB "Build with RGB color definitions (instead of BGR)" OFF)
option(ENABLE_WIDEC "Build wide-character version of the library" OFF)
option(ENABLE_XIM "Use X Input Methods" OFF)
option(ENABLE_FORCE_UTF8 "Force the use of UTF-8 instead of the system locale" OFF)

IF(ENABLE_RGB)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPDC_RGB")
    MESSAGE(STATUS "Using RGB color definitions")
ENDIF(ENABLE_RGB)
IF(ENABLE_WIDEC)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPDC_WIDE")
    MESSAGE(STATUS "Building wide-character version of the library")
ENDIF(ENABLE_WIDEC)
IF(ENABLE_XIM)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPDC_XIM")
    MESSAGE(STATUS "Using X Input Methods")
ENDIF(ENABLE_XIM)
IF(ENABLE_FORCE_UTF8)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPDC_FORCE_UTF8")
    MESSAGE(STATUS "Forcing the use of UTF-8 instead of the system locale")
ENDIF(ENABLE_FORCE_UTF8)

option(BUILD_EXAMPLES "Build example programs" ON)
message(STATUS "Building example programs: ${BUILD_EXAMPLES}")

include_directories(.)

add_subdirectory(x11)
if(BUILD_EXAMPLES)
    add_subdirectory(demos)
endif(BUILD_EXAMPLES)

add_custom_target(install_libXCurses
        $(MAKE) install
        DEPENDS XCurses
        COMMENT "Installing ${PROJECT_NAME}")
