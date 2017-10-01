
set(PRODUCT_FILE_DESCRIPTION "Public Domain Curses")
set(PRODUCT_INTERNAL_NAME "PDCurses - ${PROJECT_NAME}")
set(PRODUCT_COMPANY_COPYRIGHT "Public Domain")
set(PRODUCT_NAME "Public Domain Curses Library")
set(PRODUCT_ORIGINAL_FILENAME "PDCURSES.DLL")
set(PRODUCT_ICON "pdcurses.ico")

MESSAGE(STATUS "**** ${PROJECT_NAME} ****")

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
set(PDCURSES_DIST ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE})

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

IF(PDC_BUILD_SHARED)
    SET(PDCURSE_PROJ ${PROJECT_NAME}_pdcurses)
    ADD_LIBRARY (${PDCURSE_PROJ} SHARED ${src_files} ${pdc_src_files})

    IF(${PROJECT_NAME} STREQUAL "sdl2")
        IF(PDC_WIDE)
            TARGET_LINK_LIBRARIES (${PDCURSE_PROJ} ${WIN_DEP_LIBRARIES}
                ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARY} ${FT2_LIBRARY} ${ZLIB_LIBRARY} 
                ${SDL2_DEP_LIBRARIES})
        ELSE()
            TARGET_LINK_LIBRARIES (${PDCURSE_PROJ} ${WIN_DEP_LIBRARIES}
                ${SDL2_LIBRARIES} ${SDL2_DEP_LIBRARIES})
        ENDIF()
    ELSE()
        TARGET_LINK_LIBRARIES (${PDCURSE_PROJ} ${WIN_DEP_LIBRARIES})
    ENDIF()

    INSTALL (TARGETS ${PDCURSE_PROJ}
        ARCHIVE DESTINATION ${PDCURSES_DIST}/lib/${PROJECT_NAME}
        LIBRARY DESTINATION ${PDCURSES_DIST}/lib/${PROJECT_NAME}
        RUNTIME DESTINATION ${PDCURSES_DIST}/bin/${PROJECT_NAME} COMPONENT applications)
    SET_TARGET_PROPERTIES(${PDCURSE_PROJ} PROPERTIES OUTPUT_NAME "pdcurses")
ELSE()
    SET(PDCURSE_PROJ ${PROJECT_NAME}_pdcursesstatic)
    ADD_LIBRARY (${PDCURSE_PROJ} STATIC ${src_files} ${pdc_src_files})
    INSTALL (TARGETS ${PDCURSE_PROJ} ARCHIVE DESTINATION ${PDCURSES_DIST}/lib/${PROJECT_NAME} COMPONENT applications)
    SET_TARGET_PROPERTIES(${PDCURSE_PROJ} PROPERTIES OUTPUT_NAME "pdcursesstatic")
ENDIF()


IF (WIN32)
    ADD_DEFINITIONS(-D_WIN32 -D_CRT_SECURE_NO_WARNINGS)
    set(WIN_DEP_LIBRARIES "gdi32.lib;winspool.lib;shell32.lib;ole32.lib;comdlg32.lib;advapi32.lib")
    set(SDL2_DEP_LIBRARIES "version.lib;winmm.lib;imm32.lib")
ELSEIF(APPLE)
    set(WIN_DEP_LIBRARIES "")
    set(SDL2_DEP_LIBRARIES "dl;png;bz2")
ELSE()
    set(WIN_DEP_LIBRARIES "")
    set(SDL2_DEP_LIBRARIES "dl;png;sndio;bz2")
ENDIF()

IF (PDC_BUILD_SHARED)
    ADD_DEFINITIONS(-DPDC_DLL_BUILD)
ENDIF (PDC_BUILD_SHARED)
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

IF ("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    ADD_DEFINITIONS(-D_DEBUG)
ENDIF ()

FUNCTION (define_exe dir _name)
    set(exe_name "${PROJECT_NAME}_${_name}")
    ADD_EXECUTABLE (${exe_name} ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${_name}.c)
    TARGET_LINK_LIBRARIES (${exe_name} ${PDCURSE_PROJ} ${WIN_DEP_LIBRARIES})
    ADD_DEPENDENCIES (${exe_name} ${PDCURSE_PROJ})
    INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${PDCURSES_DIST}/bin/${PROJECT_NAME} COMPONENT applications)
    SET_TARGET_PROPERTIES(${exe_name} PROPERTIES OUTPUT_NAME ${_name})
ENDFUNCTION ()

FUNCTION (define_sdl2_exe dir _name)
    set(exe_name "${PROJECT_NAME}_${_name}")
    ADD_EXECUTABLE (${exe_name} ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${_name}.c)
    IF(PDC_WIDE)
        TARGET_LINK_LIBRARIES (${exe_name} ${PDCURSE_PROJ} ${WIN_DEP_LIBRARIES}
            "${SDL2_LIBRARIES};${SDL2_TTF_LIBRARY};${FT2_LIBRARY};${ZLIB_LIBRARY};${SDL2_DEP_LIBRARIES}")
    ELSE()
        TARGET_LINK_LIBRARIES (${exe_name} ${PDCURSE_PROJ} ${WIN_DEP_LIBRARIES}
            "${SDL2_LIBRARIES};${SDL2_TTF_DEP_LIBRARIES};${SDL2_DEP_LIBRARIES}")
    ENDIF()
    IF(PDC_WIDE AND PDC_SDL2_DEPS_BUILD)
        IF(PDC_WIDE)
            ADD_DEPENDENCIES(${exe_name} ${PDCURSE_PROJ} sdl2_ttf_ext freetype2_ext zlib_ext)
        ELSE()
            ADD_DEPENDENCIES (${exe_name} ${PDCURSE_PROJ} sdl2_ext)
        ENDIF()
    ENDIF()
    INSTALL (TARGETS ${exe_name} RUNTIME DESTINATION ${PDCURSES_DIST}/bin/${PROJECT_NAME} COMPONENT applications)
    SET_TARGET_PROPERTIES(${exe_name} PROPERTIES OUTPUT_NAME ${_name})
ENDFUNCTION ()
