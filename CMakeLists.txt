CMAKE_MINIMUM_REQUIRED (VERSION 2.8.4)

PROJECT (pdcurses C)

#read version from curses.h
file(READ ${CMAKE_SOURCE_DIR}/curses.h _curses_h_contents)
string(REGEX REPLACE ".*#define[ \t]+PDC_VER_MAJOR[ \t]+([-0-9A-Za-z.]+).*" "\\1" PRODUCT_VERSION_MAJOR ${_curses_h_contents})
string(REGEX REPLACE ".*#define[ \t]+PDC_VER_MINOR[ \t]+([-0-9A-Za-z.]+).*" "\\1" PRODUCT_VERSION_MINOR ${_curses_h_contents})
string(REGEX REPLACE ".*#define[ \t]+PDC_VER_CHANGE[ \t]+([-0-9A-Za-z.]+).*" "\\1" PRODUCT_VERSION_PATCH ${_curses_h_contents})
set(PRODUCT_VERSION_REVISION 0) #not defined in curses.h
    
#read version from version.mif
file(READ ${CMAKE_SOURCE_DIR}/version.mif _version_mif_contents)
string(REGEX REPLACE ".*VER[ \t]+=[ \t]+([-0-9A-Za-z.]+).*" "\\1" VER ${_version_mif_contents})
string(REGEX REPLACE ".*VERDOT[ \t]+=[ \t]+([-0-9A-Za-z.]+).*" "\\1" VERDOT ${_version_mif_contents})
string(REGEX REPLACE ".*ABI_MAJOR[ \t]+=[ \t]+([-0-9A-Za-z.]+).*" "\\1" ABI_MAJOR ${_version_mif_contents})
string(REGEX REPLACE ".*ABI_MINOR[ \t]+=[ \t]+([-0-9A-Za-z.]+).*" "\\1" ABI_MINOR ${_version_mif_contents})
set(ABI ${ABI_MAJOR}.${ABI_MINOR})

MESSAGE(STATUS "PRODUCT_VERSION_MAJOR = ${PRODUCT_VERSION_MAJOR}")
MESSAGE(STATUS "PRODUCT_VERSION_MINOR = ${PRODUCT_VERSION_MINOR}")
MESSAGE(STATUS "PRODUCT_VERSION_PATCH = ${PRODUCT_VERSION_PATCH}")
MESSAGE(STATUS "VER = ${VER}")
MESSAGE(STATUS "VERDOT = ${VERDOT}")
MESSAGE(STATUS "ABI_MAJOR = ${ABI_MAJOR}")
MESSAGE(STATUS "ABI_MINOR = ${ABI_MINOR}")
MESSAGE(STATUS "ABI = ${ABI}")

ADD_SUBDIRECTORY(win32)
ADD_SUBDIRECTORY(win32a)

set(CPACK_COMPONENTS_ALL applications)
