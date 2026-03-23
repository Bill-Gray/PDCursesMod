cmake_minimum_required(VERSION 3.11)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "MinSizeRel" CACHE STRING "Choose the type of build, options are: Debug, Release, or MinSizeRel." FORCE)
    message(STATUS "CMAKE_BUILD_TYPE not set, defaulting to MinSizeRel.")
endif()
set (CMAKE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")
set (PDCURSES_SRCDIR "${CMAKE_SOURCE_DIR}")

include (get_version)
project(pdcurses VERSION "${CURSES_VERSION}" LANGUAGES C)

message(STATUS "Generator .............. ${CMAKE_GENERATOR}")
message(STATUS "Build Type ............. ${CMAKE_BUILD_TYPE}")

include(build_options)
option(PDC_NO_LEAKS "No Leaks" OFF)
option(PDC_CHTYPE_16 "CHTYPE_16" OFF)

set (PDCURSES_SOURCES ${PDCURSES_SRCDIR}/pdcurses/addch.c
                      ${PDCURSES_SRCDIR}/pdcurses/addchstr.c
                      ${PDCURSES_SRCDIR}/pdcurses/addstr.c
                      ${PDCURSES_SRCDIR}/pdcurses/attr.c
                      ${PDCURSES_SRCDIR}/pdcurses/beep.c
                      ${PDCURSES_SRCDIR}/pdcurses/bkgd.c
                      ${PDCURSES_SRCDIR}/pdcurses/border.c
                      ${PDCURSES_SRCDIR}/pdcurses/clear.c
                      ${PDCURSES_SRCDIR}/pdcurses/color.c
                      ${PDCURSES_SRCDIR}/pdcurses/debug.c
                      ${PDCURSES_SRCDIR}/pdcurses/delch.c
                      ${PDCURSES_SRCDIR}/pdcurses/deleteln.c
                      ${PDCURSES_SRCDIR}/pdcurses/getch.c
                      ${PDCURSES_SRCDIR}/pdcurses/getstr.c
                      ${PDCURSES_SRCDIR}/pdcurses/getyx.c
                      ${PDCURSES_SRCDIR}/pdcurses/inch.c
                      ${PDCURSES_SRCDIR}/pdcurses/inchstr.c
                      ${PDCURSES_SRCDIR}/pdcurses/initscr.c
                      ${PDCURSES_SRCDIR}/pdcurses/inopts.c
                      ${PDCURSES_SRCDIR}/pdcurses/insch.c
                      ${PDCURSES_SRCDIR}/pdcurses/insstr.c
                      ${PDCURSES_SRCDIR}/pdcurses/instr.c
                      ${PDCURSES_SRCDIR}/pdcurses/kernel.c
                      ${PDCURSES_SRCDIR}/pdcurses/keyname.c
                      ${PDCURSES_SRCDIR}/pdcurses/mouse.c
                      ${PDCURSES_SRCDIR}/pdcurses/move.c
                      ${PDCURSES_SRCDIR}/pdcurses/outopts.c
                      ${PDCURSES_SRCDIR}/pdcurses/overlay.c
                      ${PDCURSES_SRCDIR}/pdcurses/pad.c
                      ${PDCURSES_SRCDIR}/pdcurses/panel.c
                      ${PDCURSES_SRCDIR}/pdcurses/printw.c
                      ${PDCURSES_SRCDIR}/pdcurses/refresh.c
                      ${PDCURSES_SRCDIR}/pdcurses/scanw.c
                      ${PDCURSES_SRCDIR}/pdcurses/scr_dump.c
                      ${PDCURSES_SRCDIR}/pdcurses/scroll.c
                      ${PDCURSES_SRCDIR}/pdcurses/slk.c
                      ${PDCURSES_SRCDIR}/pdcurses/termattr.c
                      ${PDCURSES_SRCDIR}/pdcurses/terminfo.c
                      ${PDCURSES_SRCDIR}/pdcurses/touch.c
                      ${PDCURSES_SRCDIR}/pdcurses/util.c
                      ${PDCURSES_SRCDIR}/pdcurses/window.c
                      ${PDCURSES_SRCDIR}/x11new/pdcclip.c
                      ${PDCURSES_SRCDIR}/x11new/pdcdisp.c
                      ${PDCURSES_SRCDIR}/x11new/pdcgetsc.c
                      ${PDCURSES_SRCDIR}/x11new/pdckbd.c
                      ${PDCURSES_SRCDIR}/x11new/pdcscrn.c
                      ${PDCURSES_SRCDIR}/x11new/pdcsetsc.c
                      ${PDCURSES_SRCDIR}/x11new/pdcutil.c
                      ${PDCURSES_SRCDIR}/x11new/pdcutil.c)

if (PDC_BUILD_SHARED)
    add_library(pdcurses SHARED ${PDCURSES_SOURCES})
else ()
    add_library(pdcurses STATIC ${PDCURSES_SOURCES})
endif ()

target_include_directories(pdcurses PRIVATE "${PDCURSES_SRCDIR}"
                                            "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_options (pdcurses PRIVATE -Wall -Wextra -pedantic -Werror -fPIC)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options (pdcurses PRIVATE -g3)
    target_compile_definitions(pdcurses PUBLIC -DPDCDEBUG)
else ()
    target_compile_options (pdcurses PRIVATE -O3)
endif ()

if (PDC_NO_LEAKS)
    target_compile_definitions(pdcurses PUBLIC -DNO_LEAKS)
endif ()


if (PDC_UTF8)
    target_compile_definitions(pdcurses PUBLIC -DPDC_FORCE_UTF8)
endif ()

if (PDC_WIDE)
    target_compile_definitions(pdcurses PUBLIC -DPDC_WIDE)
endif ()

if (PDC_CHTYPE_32)
    target_compile_definitions(pdcurses PUBLIC -DCHTYPE_32)
endif ()

if (PDC_CHTYPE_16)
    target_compile_definitions(pdcurses PUBLIC -DCHTYPE_16)
endif ()

target_link_libraries (pdcurses -lX11 -lpthread)

install (TARGETS pdcurses
         EXPORT pdcurses_Targets
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)
install (FILES "${PDCURSES_SRCDIR}/curses.h"
               "${PDCURSES_SRCDIR}/curspriv.h"
         DESTINATION include)

macro (x11_app dir targ)
    set(bin_name "${PROJECT_NAME}_${targ}")

    if(${targ} STREQUAL "tuidemo")
        set(src_files ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/tuidemo.c ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/tui.c)
    else()
        set(src_files ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${targ}.c)
    endif()

    if(${ARGV2})
        add_executable(${bin_name} WIN32 ${src_files})
    else()
        add_executable(${bin_name} ${src_files})
    endif()
    target_include_directories(${bin_name} PRIVATE "${PDCURSES_SRCDIR}"
                                                   "${CMAKE_CURRENT_BINARY_DIR}")

    target_link_libraries(${bin_name} pdcurses -lX11 -lpthread)

    add_dependencies(${bin_name} pdcurses)
    set_target_properties(${bin_name} PROPERTIES OUTPUT_NAME ${targ})
endmacro ()

# demos
x11_app(../demos calendar)
x11_app(../demos firework)
x11_app(../demos init_col)
x11_app(../demos mbrot)
x11_app(../demos newtest)
x11_app(../demos ozdemo)
x11_app(../demos picsview)
x11_app(../demos ptest)
x11_app(../demos rain)
x11_app(../demos speed)
x11_app(../demos test_pan)
x11_app(../demos testcurs)
x11_app(../demos widetest)
x11_app(../demos worm)
x11_app(../demos xmas)
x11_app(../demos tuidemo)

# tests
x11_app(../tests brk_test)
x11_app(../tests del_test)
x11_app(../tests focus)
x11_app(../tests ins_del)
x11_app(../tests keytest)
x11_app(../tests opaque)
x11_app(../tests restart)
x11_app(../tests ripoff)
x11_app(../tests show_col)
x11_app(../tests show_uni)
