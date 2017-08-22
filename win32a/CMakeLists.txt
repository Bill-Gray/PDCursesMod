CMAKE_MINIMUM_REQUIRED (VERSION 2.8.6)

PROJECT (win32a C)

option(PDC_DLL_BUILD "Dynamic Library" OFF)
option(PDC_UTF8 "Force to UTF8" OFF)
option(PDC_WIDE "Unicode" OFF)
option(PDC_CHTYPE_32 "CHTYPE_32" OFF)
option(PDC_CHTYPE_16 "CHTYPE_16" OFF)

MESSAGE(STATUS "PDC_DLL_BUILD = ${PDC_DLL_BUILD}")
MESSAGE(STATUS "PDC_WIDE = ${PDC_WIDE}")
MESSAGE(STATUS "PDC_UTF8 = ${PDC_UTF8}")
MESSAGE(STATUS "PDC_CHTYPE_32 = ${PDC_CHTYPE_32}")
MESSAGE(STATUS "PDC_CHTYPE_16 = ${PDC_CHTYPE_16}")
MESSAGE(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")


set(FILE_VERSION_MAJOR ${PRODUCT_VERSION_MAJOR})
set(FILE_VERSION_MINOR ${PRODUCT_VERSION_MINOR})
set(FILE_VERSION_PATCH ${PRODUCT_VERSION_PATCH})
set(FILE_VERSION_BUILD ${PRODUCT_VERSION_REVISION})

set(PRODUCT_FILE_DESCRIPTION "Public Domain Curses")
set(PRODUCT_INTERNAL_NAME "PDCurses")
set(PRODUCT_COMPANY_COPYRIGHT "Public Domain")
set(PRODUCT_NAME "Public Domain Curses Library")
set(PRODUCT_ORIGINAL_FILENAME "PDCURSES.DLL")
set(PRODUCT_ICON "pdcurses.ico")

set(PDCURSES_SRCDIR ${CMAKE_SOURCE_DIR})

set(osdir ${PDCURSES_SRCDIR}/${PROJECT_NAME})
set(srcdir ${PDCURSES_SRCDIR}/pdcurses)
set (demodir ${PDCURSES_SRCDIR}/demos)

configure_file ( ${osdir}/resource.in ${osdir}/resource.h @ONLY )

SET (src_files
    ${srcdir}/addch.c ${srcdir}/addchstr.c ${srcdir}/addstr.c ${srcdir}/attr.c
    ${srcdir}/beep.c ${srcdir}/bkgd.c ${srcdir}/border.c ${srcdir}/clear.c
    ${srcdir}/color.c ${srcdir}/delch.c ${srcdir}/deleteln.c ${srcdir}/deprec.c
    ${srcdir}/getch.c ${srcdir}/getstr.c ${srcdir}/getyx.c ${srcdir}/inch.c
    ${srcdir}/inchstr.c ${srcdir}/initscr.c ${srcdir}/inopts.c ${srcdir}/insch.c
    ${srcdir}/insstr.c ${srcdir}/instr.c ${srcdir}/kernel.c ${srcdir}/keyname.c
    ${srcdir}/mouse.c ${srcdir}/move.c ${srcdir}/outopts.c ${srcdir}/overlay.c
    ${srcdir}/pad.c ${srcdir}/panel.c ${srcdir}/printw.c ${srcdir}/refresh.c
    ${srcdir}/scanw.c ${srcdir}/scr_dump.c ${srcdir}/scroll.c ${srcdir}/slk.c
    ${srcdir}/termattr.c ${srcdir}/terminfo.c ${srcdir}/touch.c ${srcdir}/util.c
    ${srcdir}/window.c ${srcdir}/debug.c
)

SET (pdc_src_files
    ${osdir}/pdcclip.c ${osdir}/pdcdisp.c ${osdir}/pdcgetsc.c
    ${osdir}/pdckbd.c ${osdir}/pdcscrn.c ${osdir}/pdcsetsc.c
    ${osdir}/pdcutil.c ${osdir}/version.rc
)

INCLUDE_DIRECTORIES (${srcdir} ${osdir} ${PDCURSES_SRCDIR})

IF(PDC_DLL_BUILD)

  ADD_LIBRARY (pdcurses_${PROJECT_NAME} SHARED ${src_files} ${pdc_src_files})

  file(READ "${PDCURSES_SRCDIR}/exp-base.def" _DEF_BASE_)
  file(READ "${PDCURSES_SRCDIR}/exp-wide.def" _DEF_WIDE_)

  IF (PDC_WIDE)
    SET(deffile "${CMAKE_BINARY_DIR}/pdcursew.def")
    file(WRITE ${deffile} "LIBRARY pdcurses\nEXPORTS\n${_DEF_BASE_}${_DEF_WIDE_}")
  ELSE (PDC_WIDE)
    SET(deffile "${CMAKE_BINARY_DIR}/pdcurses.def")
    file(WRITE ${deffile} "LIBRARY pdcurses\nEXPORTS\n${_DEF_BASE_}")    
  ENDIF (PDC_WIDE)

  SET_TARGET_PROPERTIES(pdcurses_${PROJECT_NAME} PROPERTIES LINK_FLAGS "/DEF:\"${deffile}\"")

  INSTALL (TARGETS pdcurses_${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/Debug COMPONENT applications CONFIGURATIONS Debug)
  INSTALL (TARGETS pdcurses_${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/Release COMPONENT applications CONFIGURATIONS Release)
  INSTALL (TARGETS pdcurses_${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/MinSizeRel COMPONENT applications CONFIGURATIONS MinSizeRel)
  INSTALL (TARGETS pdcurses_${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/RelWithDebInfo COMPONENT applications CONFIGURATIONS RelWithDebInfo)

ELSE(PDC_DLL_BUILD)
  ADD_LIBRARY (pdcurses_${PROJECT_NAME} STATIC ${src_files} ${pdc_src_files})
ENDIF(PDC_DLL_BUILD)
SET_TARGET_PROPERTIES(pdcurses_${PROJECT_NAME} PROPERTIES OUTPUT_NAME "pdcurses")
  
FUNCTION (define_exe arg)
  set(exe_name "${arg}_${PROJECT_NAME}")
  ADD_EXECUTABLE (${exe_name} ${demodir}/${arg}.c)
  TARGET_LINK_LIBRARIES (${exe_name} pdcurses_${PROJECT_NAME} gdi32.lib winspool.lib shell32.lib ole32.lib comdlg32.lib advapi32.lib)
  ADD_DEPENDENCIES (${exe_name} pdcurses_${PROJECT_NAME})
  INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/Debug COMPONENT applications CONFIGURATIONS Debug)
  INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/Release COMPONENT applications CONFIGURATIONS Release)
  INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/MinSizeRel COMPONENT applications CONFIGURATIONS MinSizeRel)
  INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/RelWithDebInfo COMPONENT applications CONFIGURATIONS RelWithDebInfo)
  SET_TARGET_PROPERTIES(${exe_name} PROPERTIES OUTPUT_NAME ${arg})
ENDFUNCTION ()

define_exe(version)
define_exe(testcurs)
define_exe(newdemo)
define_exe(xmas)
define_exe(firework)
define_exe(newtest)
define_exe(ptest)
define_exe(rain)
define_exe(worm)

if(WIN32)
  SET_TARGET_PROPERTIES(newtest_${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif(WIN32)

ADD_EXECUTABLE (tuidemo_${PROJECT_NAME} ${demodir}/tuidemo.c ${demodir}/tui.c)
TARGET_LINK_LIBRARIES (tuidemo_${PROJECT_NAME} pdcurses_${PROJECT_NAME} gdi32.lib winspool.lib shell32.lib ole32.lib comdlg32.lib advapi32.lib)
ADD_DEPENDENCIES (tuidemo_${PROJECT_NAME} pdcurses_${PROJECT_NAME})
INSTALL (TARGETS tuidemo_${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/Debug COMPONENT applications CONFIGURATIONS Debug)
INSTALL (TARGETS tuidemo_${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/Release COMPONENT applications CONFIGURATIONS Release)
INSTALL (TARGETS tuidemo_${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/MinSizeRel COMPONENT applications CONFIGURATIONS MinSizeRel)
INSTALL (TARGETS tuidemo_${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/RelWithDebInfo COMPONENT applications CONFIGURATIONS RelWithDebInfo)
SET_TARGET_PROPERTIES(tuidemo_${PROJECT_NAME} PROPERTIES OUTPUT_NAME "tuidemo")


IF (WIN32)
  ADD_DEFINITIONS(-D_WIN32)
  IF (PDC_DLL_BUILD)
    ADD_DEFINITIONS(-DPDC_DLL_BUILD)
  ENDIF (PDC_DLL_BUILD)
  IF (PDC_WIDE)
    ADD_DEFINITIONS(-DPDC_WIDE)
  ENDIF(PDC_WIDE)
  IF (PDC_UTF8)
    ADD_DEFINITIONS(-DPDC_FORCE_UTF8)
  ENDIF (PDC_UTF8)
  IF (PDC_CHTYPE_16)
    ADD_DEFINITIONS(-DCHTYPE_16)
  ENDIF (PDC_CHTYPE_16)
  IF (PDC_CHTYPE_32)
    ADD_DEFINITIONS(-DCHTYPE_32)
  ENDIF (PDC_CHTYPE_32)

  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  IF (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    ADD_DEFINITIONS(-DPDCDEBUG)
  ENDIF (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
ENDIF (WIN32)

set(CPACK_COMPONENTS_ALL applications)
