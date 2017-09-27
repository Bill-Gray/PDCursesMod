CMAKE_MINIMUM_REQUIRED (VERSION 2.8.6)

IF(POLICY CMP0048)
    CMAKE_POLICY(SET CMP0048 NEW)
ENDIF()

PROJECT(win32a VERSION "${PROJECT_VERSION}" LANGUAGES C)

INCLUDE(project_common)
  
define_exe(../demos version)
define_exe(../demos testcurs)
define_exe(../demos newdemo)
define_exe(../demos xmas)
define_exe(../demos firework)
define_exe(../demos newtest)
define_exe(../demos ptest)
define_exe(../demos rain)
define_exe(../demos worm)

IF(WIN32)
    SET_TARGET_PROPERTIES(${PROJECT_NAME}_newtest PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
ENDIF(WIN32)

ADD_EXECUTABLE (${PROJECT_NAME}_tuidemo ${demodir}/tuidemo.c ${demodir}/tui.c)
TARGET_LINK_LIBRARIES (${PROJECT_NAME}_tuidemo ${PDCURSE_PROJ} ${WINDOWS_KIT_UM_LIBS})
ADD_DEPENDENCIES (${PROJECT_NAME}_tuidemo ${PDCURSE_PROJ})
INSTALL (TARGETS ${PROJECT_NAME}_tuidemo RUNTIME DESTINATION ${PDCURSES_DIST}/bin/${PROJECT_NAME} COMPONENT applications)
SET_TARGET_PROPERTIES(${PROJECT_NAME}_tuidemo PROPERTIES OUTPUT_NAME "tuidemo")

SET(CPACK_COMPONENTS_ALL applications)
