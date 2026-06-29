cmake_minimum_required(VERSION 3.11)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "MinSizeRel" CACHE STRING "Choose the type of build, options are: Debug, Release, or MinSizeRel." FORCE)
    message(STATUS "No build type specified, defaulting to MinSizeRel.")
endif()

project(sdl3 VERSION "${PROJECT_VERSION}" LANGUAGES C)
message(STATUS "${PROJECT_NAME} version: ${PROJECT_VERSION}")

message(STATUS "SDL3_LIBRARIES = ${SDL3_LIBRARIES}")
message(STATUS "SDL3_INCLUDE_DIR = ${SDL3_INCLUDE_DIR}")
message(STATUS "SDL3_LIBRARY_DIR = ${SDL3_LIBRARY_DIR}")
if(PDC_WIDE OR PDC_UTF8)
    message(STATUS "SDL3_TTF_LIBRARY = ${SDL3_TTF_LIBRARY}")
    message(STATUS "SDL3_TTF_INCLUDE_DIR = ${SDL3_TTF_INCLUDE_DIR}")
    message(STATUS "SDL3_TTF_LIBRARY_DIR = ${SDL3_TTF_LIBRARY_DIR}")
endif()

include_directories(${SDL3_INCLUDE_DIR} ${SDL3_TTF_INCLUDE_DIR})
link_directories(${SDL3_LIBRARY_DIR} ${SDL3_TTF_LIBRARY_DIR})

if(NOT pdcurses_src_files)
    file(GLOB pdcurses_src_files ${CMAKE_CURRENT_SOURCE_DIR}/../pdcurses/*.c)
endif()

include(project_common)

macro (sdl3_app dir targ)

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

    if(PDC_WIDE OR PDC_UTF8)
        target_link_libraries(${bin_name} ${PDCURSE_PROJ} ${EXTRA_LIBS}
            "${SDL3_LIBRARIES};${SDL3_TTF_LIBRARY};${FT2_LIBRARY};${ZLIB_LIBRARY};${SDL3_DEP_LIBRARIES}")
    else()
        target_link_libraries(${bin_name} ${PDCURSE_PROJ} ${EXTRA_LIBS}
        "${SDL3_LIBRARIES};${SDL3_TTF_DEP_LIBRARIES};${SDL3_DEP_LIBRARIES}")
    endif()

    add_dependencies(${bin_name} ${PDCURSE_PROJ})
    set_target_properties(${bin_name} PROPERTIES OUTPUT_NAME ${targ})

    install(TARGETS ${bin_name} RUNTIME DESTINATION ${PDCURSES_DIST}/bin/${PROJECT_NAME} COMPONENT applications)

endmacro ()

sdl3_app(../demos version)
sdl3_app(../demos calendar)
sdl3_app(../demos firework)
sdl3_app(../demos init_col)
sdl3_app(../demos mbrot)
sdl3_app(../demos ozdemo)
sdl3_app(../demos newtest WIN32)
sdl3_app(../demos picsview)
sdl3_app(../demos ptest)
sdl3_app(../demos rain)
sdl3_app(../demos speed)
sdl3_app(../demos testcurs)
sdl3_app(../demos test_pan)
sdl3_app(../demos tuidemo)
sdl3_app(../demos widetest)
sdl3_app(../demos worm)
sdl3_app(../demos xmas)
sdl3_app(./ sdltest)

if(PDC_SDL3_DEPS_BUILD)
    if(PDC_WIDE OR PDC_UTF8)
        add_dependencies(${PDCURSE_PROJ} sdl3_ext sdl3_ttf_ext)
    else()
        add_dependencies(${PDCURSE_PROJ} sdl3_ext)
    endif()
endif()

set(CPACK_COMPONENTS_ALL applications)
