
set(PRODUCT_FILE_DESCRIPTION "Public Domain Curses")
set(PRODUCT_INTERNAL_NAME "PDCurses - ${PROJECT_NAME}")
set(PRODUCT_COMPANY_COPYRIGHT "Public Domain")
set(PRODUCT_NAME "Public Domain Curses Library")
set(PRODUCT_ORIGINAL_FILENAME "PDCURSES.DLL")
set(PRODUCT_ICON "pdcurses.ico")

MESSAGE(STATUS "**** ${PROJECT_NAME} ****")
MESSAGE(STATUS "Installation Path = ${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "PDC_DLL_BUILD = ${PDC_DLL_BUILD}")
MESSAGE(STATUS "PDC_UTF8 = ${PDC_UTF8}")
MESSAGE(STATUS "PDC_WIDE = ${PDC_WIDE}")
MESSAGE(STATUS "PDCDEBUG = ${PDCDEBUG}")
MESSAGE(STATUS "PDC_CHTYPE_32 = ${PDC_CHTYPE_32}")
MESSAGE(STATUS "PDC_CHTYPE_16 = ${PDC_CHTYPE_16}")

set(PROJECT_VERSION_REVISION 0)
set(PRODUCT_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(PRODUCT_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(PRODUCT_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(PRODUCT_VERSION_BUILD ${PROJECT_VERSION_REVISION})

set(FILE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(FILE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(FILE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(FILE_VERSION_BUILD ${PROJECT_VERSION_REVISION})

set(PDCURSES_SRCDIR ${CMAKE_SOURCE_DIR})
set(PDCURSES_DIST ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME})

set(osdir ${PDCURSES_SRCDIR}/${PROJECT_NAME})
set(srcdir ${PDCURSES_SRCDIR}/pdcurses)
set(demodir ${PDCURSES_SRCDIR}/demos)

configure_file ( ${CMAKE_SOURCE_DIR}/cmake/version.in.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.rc @ONLY )
configure_file ( ${CMAKE_SOURCE_DIR}/cmake/resource.in.cmake ${CMAKE_CURRENT_BINARY_DIR}/resource.h @ONLY )

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

IF (MSVC)

    ADD_DEFINITIONS(-D_WIN32 -D_CRT_SECURE_NO_WARNINGS)

    set(WINDOWS_KIT_UM_LIBS 
        gdi32.lib 
        winspool.lib 
        shell32.lib 
        ole32.lib 
        comdlg32.lib 
        advapi32.lib
        )

    set(WINDOWS_KIT_UM_SDL2_LIBS 
        version.lib 
        winmm.lib
        imm32.lib
        )

ELSEIF ()

    set(WINDOWS_KIT_UM_LIBS 
        ${WINDOWS_KIT_LIBRARY_DIR}/gdi32.lib
        ${WINDOWS_KIT_LIBRARY_DIR}/winspool.lib
        ${WINDOWS_KIT_LIBRARY_DIR}/shell32.lib
        ${WINDOWS_KIT_LIBRARY_DIR}/ole32.lib
        ${WINDOWS_KIT_LIBRARY_DIR}/comdlg32.lib
        ${WINDOWS_KIT_LIBRARY_DIR}/advapi32.lib
        )

    set(WINDOWS_KIT_UM_SDL2_LIBS
        ${WINDOWS_KIT_LIBRARY_DIR}/version.lib
        ${WINDOWS_KIT_LIBRARY_DIR}/winmm.lib
        ${WINDOWS_KIT_LIBRARY_DIR}/imm32.lib
        )
        
ENDIF ()

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
IF(PDCDEBUG)
    ADD_DEFINITIONS(-DPDCDEBUG)
ENDIF(PDCDEBUG)
IF(PDC_HAVE_VSNPRINTF)
    ADD_DEFINITIONS(-DHAVE_VSNPRINTF)
ENDIF(PDC_HAVE_VSNPRINTF)
IF(PDC_HAVE_VSSCANF)
    ADD_DEFINITIONS(-DHAVE_VSSCANF)
ENDIF(PDC_HAVE_VSSCANF)

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS(-D_DEBUG)
ENDIF ()

FUNCTION (define_exe dir _name)
    set(exe_name "${PROJECT_NAME}_${_name}")
    ADD_EXECUTABLE (${exe_name} ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${_name}.c)
    TARGET_LINK_LIBRARIES (${exe_name} ${PROJECT_NAME}_pdcurses ${WINDOWS_KIT_UM_LIBS})
    ADD_DEPENDENCIES (${exe_name} ${PROJECT_NAME}_pdcurses)
    INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${PDCURSES_DIST}/Debug/bin COMPONENT applications CONFIGURATIONS Debug)
    INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${PDCURSES_DIST}/Release/bin COMPONENT applications CONFIGURATIONS Release)
    INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${PDCURSES_DIST}/MinSizeRel/bin COMPONENT applications CONFIGURATIONS MinSizeRel)
    INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${PDCURSES_DIST}/RelWithDebInfo/bin COMPONENT applications CONFIGURATIONS RelWithDebInfo)
    SET_TARGET_PROPERTIES(${exe_name} PROPERTIES OUTPUT_NAME ${_name})
ENDFUNCTION ()

FUNCTION (define_sdl2_exe dir _name)
    set(exe_name "${PROJECT_NAME}_${_name}")
    ADD_EXECUTABLE (${exe_name} ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${_name}.c)
    TARGET_LINK_LIBRARIES (${exe_name} ${PROJECT_NAME}_pdcurses ${WINDOWS_KIT_UM_LIBS} ${SDL2_LIBRARY} ${WINDOWS_KIT_UM_SDL2_LIBS})
    IF(BUILD_SDL2)  
        ADD_DEPENDENCIES (${exe_name} ${PROJECT_NAME}_pdcurses sdl2_ext)
    ELSE()
        ADD_DEPENDENCIES (${exe_name} ${PROJECT_NAME}_pdcurses)
    ENDIF()
    INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${PDCURSES_DIST}/Debug/bin COMPONENT applications CONFIGURATIONS Debug)
    INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${PDCURSES_DIST}/Release/bin COMPONENT applications CONFIGURATIONS Release)
    INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${PDCURSES_DIST}/MinSizeRel/bin COMPONENT applications CONFIGURATIONS MinSizeRel)
    INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${PDCURSES_DIST}/RelWithDebInfo/bin COMPONENT applications CONFIGURATIONS RelWithDebInfo)
    SET_TARGET_PROPERTIES(${exe_name} PROPERTIES OUTPUT_NAME ${_name})
ENDFUNCTION ()
