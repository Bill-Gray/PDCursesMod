if(PDC_SDL2_BUILD)

    include(ExternalProject)


    if(BUILD_SDL2)
    
        set(SDL2_SOURCE_DIR ${CMAKE_SOURCE_DIR}/sdl2.src CACHE INTERNAL "sdl2 source dir")
        set(SDL2_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/sdl2/${CMAKE_BUILD_TYPE})

        ExternalProject_Add(sdl2_ext
            URL https://www.libsdl.org/release/SDL2-2.0.6.zip
            URL_HASH "SHA256=744398b8a8ad65b36e805ac1ed53acb94bd62eeeaa4683507328b9d4b76a0d3b"
            UPDATE_COMMAND ""
            DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}
            SOURCE_DIR ${SDL2_SOURCE_DIR}
            BUILD_IN_SOURCE 0
            CMAKE_ARGS ${SDL_CMAKE_BUILD_OPTS}
                -DCMAKE_INSTALL_PREFIX=${SDL2_INSTALL_PATH}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            )

        MESSAGE(STATUS "SDL2 Installing to: ${CMAKE_INSTALL_PREFIX}")

        set(SDL2_INCLUDE_DIR ${SDL2_INSTALL_PATH}/include/SDL2)
        set(SDL2_LIBRARY_DIR ${SDL2_INSTALL_PATH}/lib)

        IF (NOT PDC_DLL_BUILD)
            IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
                set(SDL2_LIBRARY SDL2maind.lib SDL2-staticd.lib)
            ELSE ()
                set(SDL2_LIBRARY SDL2main.lib SDL2-static.lib)
            ENDIF ()
        ELSE ()
          IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
              set(SDL2_LIBRARY SDL2d.lib)
          ELSE ()
              set(SDL2_LIBRARY SDL2.lib)
          ENDIF ()
        ENDIF ()

    endif()
    
endif()
