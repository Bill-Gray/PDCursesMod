
IF (PDC_SDL2_DEPS_BUILD)

    INCLUDE(ExternalProject)


    SET(SDL2_RELEASE 2.0.6)

    ExternalProject_Add(sdl2_ext
        URL https://www.libsdl.org/release/SDL2-${SDL2_RELEASE}.zip
        URL_HASH "SHA256=744398b8a8ad65b36e805ac1ed53acb94bd62eeeaa4683507328b9d4b76a0d3b"
        UPDATE_COMMAND ""
        DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
        SOURCE_DIR ${CMAKE_BINARY_DIR}/SDL2-${SDL2_RELEASE}
        BUILD_IN_SOURCE 0
        CMAKE_ARGS
            ${SDL_CMAKE_BUILD_OPTS}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        )

    MESSAGE(STATUS "SDL2 Installing to: ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}")
    SET(SDL2_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/include/SDL2)
    SET(SDL2_LIBRARY_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/lib)

    IF (PDC_WIDE)

        ExternalProject_Add(zlib_ext
            GIT_REPOSITORY "https://github.com/madler/zlib.git"
            GIT_TAG "v1.2.11"
            UPDATE_COMMAND ""
            DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
            SOURCE_DIR ${CMAKE_BINARY_DIR}/zlib
            BUILD_IN_SOURCE 1
            CMAKE_ARGS
                ${ZLIB_CMAKE_BUILD_OPTS}
                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}
                -DCMAKE_CXX_FLAGS=${EXTERNAL_CXX_FLAGS}
                -DCMAKE_C_FLAGS=${EXTERNAL_C_FLAGS}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DBUILD_SHARED_LIBS=${BUILD_SHARED}
                -DAMD64=${ZLIB_AMD64}
                -DASM686=${ZLIB_ASM686}
            )

        MESSAGE(STATUS "zlib Installing to: ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}")
        SET(ZLIB_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/include)
        SET(ZLIB_LIBRARY_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/lib)


        ExternalProject_Add(freetype2_ext
            GIT_REPOSITORY "https://git.savannah.gnu.org/git/freetype/freetype2.git"
            GIT_TAG "VER-2-8-1"
            UPDATE_COMMAND ""
            DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
            SOURCE_DIR ${CMAKE_BINARY_DIR}/freetype2
            BUILD_IN_SOURCE 0
            CMAKE_ARGS
                ${FT2_CMAKE_BUILD_OPTS}
                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}
                -DCMAKE_CXX_FLAGS=${EXTERNAL_CXX_FLAGS}
                -DCMAKE_C_FLAGS=${EXTERNAL_C_FLAGS}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DWITH_ZLIB=ON
                -DZLIB_INCLUDE_DIR=${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/include
                -DZLIB_LIBRARY_DIR=${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/lib
            )
    
        ADD_DEPENDENCIES(freetype2_ext zlib_ext)
        MESSAGE(STATUS "freetype2 Installing to: ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}")
        SET(FT2_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/include)
        SET(FT2_LIBRARY_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/lib)
           
        
        SET(SDL2_TTF_RELEASE 2.0.14)

        ExternalProject_Add(sdl2_ttf_ext
            URL https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-${SDL2_TTF_RELEASE}.zip
            URL_HASH "SHA256=ad7a7d2562c19ad2b71fa4ab2e76f9f52b3ee98096c0a7d7efbafc2617073c27"
            PATCH_COMMAND cmake -E copy 
                ${CMAKE_SOURCE_DIR}/cmake/sdl2_ttf/CMakeLists.txt 
                ${CMAKE_BINARY_DIR}/sdl2_ttf/CMakeLists.txt
            UPDATE_COMMAND ""
            DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
            SOURCE_DIR ${CMAKE_BINARY_DIR}/sdl2_ttf
            BUILD_IN_SOURCE 0
            CMAKE_ARGS
                ${SDL2_TTF_CMAKE_BUILD_OPTS}
                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}
                -DCMAKE_CXX_FLAGS=${EXTERNAL_CXX_FLAGS}
                -DCMAKE_C_FLAGS=${EXTERNAL_C_FLAGS}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DZLIB_INCLUDE_DIR=${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/include
                -DZLIB_LIBRARY_DIR=${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/lib
                -DFT2_INCLUDE_DIR=${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/include/freetype2
                -DFT2_LIBRARY_DIR=${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/lib
                -DSDL2_INCLUDE_DIR=${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/include/SDL2
                -DSDL2_LIBRARY_DIR=${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/lib
                -DSDL2_LIBRARIES=${SDL2_LIBRARIES}
            )

        ADD_DEPENDENCIES(sdl2_ttf_ext sdl2_ext freetype2_ext)
        MESSAGE(STATUS "SDL2_ttf Installing to: ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}")
        SET(SDL2_TTF_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/include/SDL2_ttf)
        SET(SDL2_TTF_LIBRARY_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/lib)

        IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
            IF(WIN32)
                set(SDL2_LIBRARIES "SDL2d.lib;SDL2maind.lib")
                set(SDL2_TTF_LIBRARIES "SDL2_ttfd.lib;freetyped.lib;zlibd.lib")
            ELSE()
                set(SDL2_LIBRARIES "-lSDL2d;-lSDL2maind")
                set(SDL2_TTF_LIBRARIES "-lSDL2_ttfd;-lfreetyped;-lzlibd")
            ENDIF()
        ELSE()
            IF(WIN32)
                set(SDL2_LIBRARIES "SDL2.lib;SDL2main.lib")
                set(SDL2_TTF_LIBRARIES "SDL2_ttf.lib;freetype.lib;zlib.lib")
            ELSE()
                set(SDL2_LIBRARIES "-lSDL2;-lSDL2main")
                set(SDL2_TTF_LIBRARIES "-lSDL2_ttf;-lfreetype;-lzlib")
            ENDIF()
        ENDIF()
    
    ENDIF (PDC_WIDE)

ENDIF()
