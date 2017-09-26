CMAKE_MINIMUM_REQUIRED (VERSION 2.8.6)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif()

PROJECT(sdl2 VERSION "${PROJECT_VERSION}" LANGUAGES C)

include (project_common)

MESSAGE(STATUS "SDL2_LIBRARY = ${SDL2_LIBRARY}")
MESSAGE(STATUS "SDL2_INCLUDE_DIR = ${SDL2_INCLUDE_DIR}")
MESSAGE(STATUS "SDL2_LIBRARY_DIR = ${SDL2_LIBRARY_DIR}")

INCLUDE_DIRECTORIES (${SDL2_INCLUDE_DIR})
LINK_DIRECTORIES(${SDL2_LIBRARY_DIR})

define_sdl2_exe(../demos version)
define_sdl2_exe(../demos firework)
define_sdl2_exe(../demos newdemo)
define_sdl2_exe(../demos newtest)
define_sdl2_exe(../demos ptest)
define_sdl2_exe(../demos rain)
define_sdl2_exe(../demos testcurs)
define_sdl2_exe(../demos worm)
define_sdl2_exe(../demos xmas)
define_sdl2_exe(./ sdltest)

if(WIN32)
    SET_TARGET_PROPERTIES(${PROJECT_NAME}_newtest PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif(WIN32)

ADD_EXECUTABLE (${PROJECT_NAME}_tuidemo ${demodir}/tuidemo.c ${demodir}/tui.c)
TARGET_LINK_LIBRARIES (${PROJECT_NAME}_tuidemo ${PROJECT_NAME}_pdcurses ${WINDOWS_KIT_UM_LIBS} ${SDL2_LIBRARY} ${WINDOWS_KIT_UM_SDL2_LIBS})
ADD_DEPENDENCIES (${PROJECT_NAME}_tuidemo ${PROJECT_NAME}_pdcurses)
INSTALL (TARGETS ${PROJECT_NAME}_tuidemo RUNTIME DESTINATION ${PDCURSES_DIST}/Debug/bin COMPONENT applications CONFIGURATIONS Debug)
INSTALL (TARGETS ${PROJECT_NAME}_tuidemo RUNTIME DESTINATION ${PDCURSES_DIST}/Release/bin COMPONENT applications CONFIGURATIONS Release)
INSTALL (TARGETS ${PROJECT_NAME}_tuidemo RUNTIME DESTINATION ${PDCURSES_DIST}/MinSizeRel/bin COMPONENT applications CONFIGURATIONS MinSizeRel)
INSTALL (TARGETS ${PROJECT_NAME}_tuidemo RUNTIME DESTINATION ${PDCURSES_DIST}/RelWithDebInfo/bin COMPONENT applications CONFIGURATIONS RelWithDebInfo)
SET_TARGET_PROPERTIES(${PROJECT_NAME}_tuidemo PROPERTIES OUTPUT_NAME "tuidemo")


set(CPACK_COMPONENTS_ALL applications)
