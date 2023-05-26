cmake_minimum_required(VERSION 3.11)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "MinSizeRel" CACHE STRING "Choose the type of build, options are: Debug, Release, or MinSizeRel." FORCE)
    message(STATUS "No build type specified, defaulting to MinSizeRel.")
endif()

project(gl VERSION "${PROJECT_VERSION}" LANGUAGES C)
message(STATUS "${PROJECT_NAME} version: ${PROJECT_VERSION}")

message(STATUS "SDL2_LIBRARIES = ${SDL2_LIBRARIES}")
message(STATUS "SDL2_INCLUDE_DIR = ${SDL2_INCLUDE_DIR}")
message(STATUS "SDL2_LIBRARY_DIR = ${SDL2_LIBRARY_DIR}")
message(STATUS "SDL2_TTF_LIBRARY = ${SDL2_TTF_LIBRARY}")
message(STATUS "SDL2_TTF_INCLUDE_DIR = ${SDL2_TTF_INCLUDE_DIR}")
message(STATUS "SDL2_TTF_LIBRARY_DIR = ${SDL2_TTF_LIBRARY_DIR}")

include_directories(${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR})
link_directories(${SDL2_LIBRARY_DIR} ${SDL2_TTF_LIBRARY_DIR})

include(project_common)

macro (gl_app dir targ)

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

    target_link_libraries(${bin_name} ${PDCURSE_PROJ} ${EXTRA_LIBS}
        "${SDL2_LIBRARIES};${SDL2_TTF_LIBRARY};${FT2_LIBRARY};${ZLIB_LIBRARY};${SDL2_DEP_LIBRARIES}")

    add_dependencies(${bin_name} ${PDCURSE_PROJ})
    set_target_properties(${bin_name} PROPERTIES OUTPUT_NAME ${targ})

    install(TARGETS ${bin_name} RUNTIME DESTINATION ${PDCURSES_DIST}/bin/${PROJECT_NAME} COMPONENT applications)
    
endmacro ()

gl_app(../demos version)
gl_app(../demos firework)
gl_app(../demos ozdemo)
gl_app(../demos newtest WIN32)
gl_app(../demos ptest)
gl_app(../demos rain)
gl_app(../demos testcurs)
gl_app(../demos tuidemo)
gl_app(../demos worm)
gl_app(../demos xmas)

if(PDC_SDL2_DEPS_BUILD)
    add_dependencies(${PDCURSE_PROJ} sdl2_ext sdl2_ttf_ext)
    target_link_libraries(${PDCURSE_PROJ} ${EXTRA_LIBS} "${SDL2_LIBRARIES};${SDL2_TTF_LIBRARY};${FT2_LIBRARY};${ZLIB_LIBRARY};${SDL2_DEP_LIBRARIES}")
    target_include_directories(${PDCURSE_PROJ} PUBLIC ${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR})
endif()

set(CPACK_COMPONENTS_ALL applications)
