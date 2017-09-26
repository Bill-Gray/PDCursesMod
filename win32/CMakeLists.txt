CMAKE_MINIMUM_REQUIRED (VERSION 2.8.6)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif()

PROJECT(win32 VERSION "${PROJECT_VERSION}" LANGUAGES C)

include (common_config)

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
    ${osdir}/pdcutil.c ${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

INCLUDE_DIRECTORIES (${srcdir} ${osdir} ${PDCURSES_SRCDIR})

IF(PDC_DLL_BUILD)
  ADD_LIBRARY (${PROJECT_NAME}_pdcurses SHARED ${src_files} ${pdc_src_files})
  INSTALL (TARGETS ${PROJECT_NAME}_pdcurses RUNTIME DESTINATION ${PDCURSES_DIST}/Debug/bin COMPONENT applications CONFIGURATIONS Debug)
  INSTALL (TARGETS ${PROJECT_NAME}_pdcurses RUNTIME DESTINATION ${PDCURSES_DIST}/Release/bin COMPONENT applications CONFIGURATIONS Release)
  INSTALL (TARGETS ${PROJECT_NAME}_pdcurses RUNTIME DESTINATION ${PDCURSES_DIST}/MinSizeRel/bin COMPONENT applications CONFIGURATIONS MinSizeRel)
  INSTALL (TARGETS ${PROJECT_NAME}_pdcurses RUNTIME DESTINATION ${PDCURSES_DIST}/RelWithDebInfo/bin COMPONENT applications CONFIGURATIONS RelWithDebInfo)
ELSE(PDC_DLL_BUILD)
  ADD_LIBRARY (${PROJECT_NAME}_pdcurses STATIC ${src_files} ${pdc_src_files})
  INSTALL (TARGETS ${PROJECT_NAME}_pdcurses ARCHIVE DESTINATION ${PDCURSES_DIST}/Debug/lib COMPONENT applications CONFIGURATIONS Debug)
  INSTALL (TARGETS ${PROJECT_NAME}_pdcurses ARCHIVE DESTINATION ${PDCURSES_DIST}/Release/lib COMPONENT applications CONFIGURATIONS Release)
  INSTALL (TARGETS ${PROJECT_NAME}_pdcurses ARCHIVE DESTINATION ${PDCURSES_DIST}/MinSizeRel/lib COMPONENT applications CONFIGURATIONS MinSizeRel)
  INSTALL (TARGETS ${PROJECT_NAME}_pdcurses ARCHIVE DESTINATION ${PDCURSES_DIST}/RelWithDebInfo/lib COMPONENT applications CONFIGURATIONS RelWithDebInfo)
ENDIF(PDC_DLL_BUILD)
SET_TARGET_PROPERTIES(${PROJECT_NAME}_pdcurses PROPERTIES OUTPUT_NAME "pdcurses")
SET_TARGET_PROPERTIES(${PROJECT_NAME}_pdcurses PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB")
SET_TARGET_PROPERTIES(${PROJECT_NAME}_pdcurses PROPERTIES LINK_FLAGS_RELEASE "/NODEFAULTLIB")
SET_TARGET_PROPERTIES(${PROJECT_NAME}_pdcurses PROPERTIES LINK_FLAGS_MINSIZEREL "/NODEFAULTLIB")
SET_TARGET_PROPERTIES(${PROJECT_NAME}_pdcurses PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/NODEFAULTLIB")
SET_TARGET_PROPERTIES(${PROJECT_NAME}_pdcurses PROPERTIES STATIC_LIBRARY_FLAGS "/NODEFAULTLIB")  

FUNCTION (define_exe arg)
  set(exe_name "${PROJECT_NAME}_${arg}")
  ADD_EXECUTABLE (${exe_name} ${demodir}/${arg}.c)
  TARGET_LINK_LIBRARIES (${exe_name} ${PROJECT_NAME}_pdcurses ${WINDOWS_KIT_UM_LIBS})
  ADD_DEPENDENCIES (${exe_name} ${PROJECT_NAME}_pdcurses)
  INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${PDCURSES_DIST}/Debug/bin COMPONENT applications CONFIGURATIONS Debug)
  INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${PDCURSES_DIST}/Release/bin COMPONENT applications CONFIGURATIONS Release)
  INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${PDCURSES_DIST}/MinSizeRel/bin COMPONENT applications CONFIGURATIONS MinSizeRel)
  INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${PDCURSES_DIST}/RelWithDebInfo/bin COMPONENT applications CONFIGURATIONS RelWithDebInfo)
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
