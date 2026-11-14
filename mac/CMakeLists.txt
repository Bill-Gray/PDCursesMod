cmake_minimum_required(VERSION 3.11)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "MinSizeRel" CACHE STRING "Choose the type of build, options are: Debug, Release, or MinSizeRel." FORCE)
    message(STATUS "No build type specified, defaulting to MinSizeRel.")
endif()

PROJECT(mac VERSION "${PROJECT_VERSION}" LANGUAGES C)

INCLUDE(project_common)

target_include_directories(${PDCURSE_PROJ} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../common)

if(CMAKE_SYSTEM_NAME MATCHES "^Retro")
    set(PDC_MAC_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/pdcurses.rsrc.bin)
    set(PDC_MAC_REZ_INCLUDES ${REZ_INCLUDE_PATH})
    list(TRANSFORM PDC_MAC_REZ_INCLUDES PREPEND -I)
    add_custom_command(OUTPUT ${PDC_MAC_RESOURCES}
        COMMAND ${REZ} ${CMAKE_CURRENT_SOURCE_DIR}/pdcurses.r ${PDC_MAC_REZ_INCLUDES} -o ${PDC_MAC_RESOURCES}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pdcurses.r)
    add_custom_target(${PROJECT_NAME}_resources DEPENDS ${PDC_MAC_RESOURCES})
endif()

demo_app(../demos calendar)
demo_app(../demos firework)
demo_app(../demos init_col)
demo_app(../demos mbrot)
demo_app(../demos newtest)
demo_app(../demos ozdemo)
demo_app(../demos picsview)
demo_app(../demos ptest)
demo_app(../demos rain)
demo_app(../demos speed)
demo_app(../demos test_pan)
demo_app(../demos testcurs)
demo_app(../demos version)
demo_app(../demos widetest)
demo_app(../demos worm)
demo_app(../demos xmas)

SET(CPACK_COMPONENTS_ALL applications)
