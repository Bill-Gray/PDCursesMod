CMAKE_MINIMUM_REQUIRED (VERSION 2.8.6)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif()

PROJECT(win32a VERSION "${PROJECT_VERSION}" LANGUAGES C)

include (project_common)
  
define_exe(../demos version)
define_exe(../demos testcurs)
define_exe(../demos newdemo)
define_exe(../demos xmas)
define_exe(../demos firework)
define_exe(../demos newtest)
define_exe(../demos ptest)
define_exe(../demos rain)
define_exe(../demos worm)

if(WIN32)
    SET_TARGET_PROPERTIES(${PROJECT_NAME}_newtest PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif(WIN32)

ADD_EXECUTABLE (${PROJECT_NAME}_tuidemo ${demodir}/tuidemo.c ${demodir}/tui.c)
TARGET_LINK_LIBRARIES (${PROJECT_NAME}_tuidemo ${PROJECT_NAME}_pdcurses ${WINDOWS_KIT_UM_LIBS})
ADD_DEPENDENCIES (${PROJECT_NAME}_tuidemo ${PROJECT_NAME}_pdcurses)
INSTALL (TARGETS ${PROJECT_NAME}_tuidemo RUNTIME DESTINATION ${PDCURSES_DIST}/Debug/bin COMPONENT applications CONFIGURATIONS Debug)
INSTALL (TARGETS ${PROJECT_NAME}_tuidemo RUNTIME DESTINATION ${PDCURSES_DIST}/Release/bin COMPONENT applications CONFIGURATIONS Release)
INSTALL (TARGETS ${PROJECT_NAME}_tuidemo RUNTIME DESTINATION ${PDCURSES_DIST}/MinSizeRel/bin COMPONENT applications CONFIGURATIONS MinSizeRel)
INSTALL (TARGETS ${PROJECT_NAME}_tuidemo RUNTIME DESTINATION ${PDCURSES_DIST}/RelWithDebInfo/bin COMPONENT applications CONFIGURATIONS RelWithDebInfo)
SET_TARGET_PROPERTIES(${PROJECT_NAME}_tuidemo PROPERTIES OUTPUT_NAME "tuidemo")

set(CPACK_COMPONENTS_ALL applications)
