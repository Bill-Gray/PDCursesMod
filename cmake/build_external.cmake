if(BUILD_SDL2)

    include(ExternalProject)


    if(BUILD_SDL2)
    
        set(SDL2_SOURCE_DIR ${CMAKE_BINARY_DIR}/sdl2 CACHE INTERNAL "sdl2 source dir")

        ExternalProject_Add(sdl2
            URL https://www.libsdl.org/release/SDL2-2.0.6.zip
            URL_HASH "SHA256=744398b8a8ad65b36e805ac1ed53acb94bd62eeeaa4683507328b9d4b76a0d3b"
            UPDATE_COMMAND ""
            DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
            SOURCE_DIR ${SDL2_SOURCE_DIR}
            BUILD_IN_SOURCE 0
            CMAKE_ARGS ${SDL_CMAKE_BUILD_OPTS}
                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            )

        MESSAGE(STATUS "SDL2 Installing to: ${CMAKE_INSTALL_PREFIX}")

        INCLUDE_DIRECTORIES(${SDL2_SOURCE_DIR})
        LINK_DIRECTORIES(${CMAKE_INSTALL_PREFIX}/lib)
        
    endif()
    
endif()
