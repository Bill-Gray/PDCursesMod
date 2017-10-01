
CMake Options
-------------

    OPTION(PDC_DLL_BUILD "Build pdcurses.dll" OFF)
    OPTION(PDC_UTF8 "Force to UTF8" OFF)
    OPTION(PDC_WIDE "Unicode" OFF)
    OPTION(PDCDEBUG "Debug" OFF)
    OPTION(PDC_CHTYPE_32 "CHTYPE_32" OFF)
    OPTION(PDC_CHTYPE_16 "CHTYPE_16" OFF)
    OPTION(PDC_SDL2_BUILD "Build SDL2 Project" ON)
    OPTION(PDC_SDL2_DEPS_BUILD "Build SDL2 and dependencies" ON)
    

To override the default option value, use the "-DBUILD_SDL2=ON" scheme.  If you set it once, CMake caches the value.

When using pre-built SDL2 bits, simply set these two variables when invoking cmake the first time.

        -DSDL2_INCLUDE_DIR=/my/path/to/sdl2/include/SDL2
        -DSDL2_LIBRARY_DIR=/my/path/to/sdl2/lib/folder


Native Windows Building
-----------------------

Win32 (pdcurses.sln)

    mkdir build32 & pushd build32
	cmake -G"Visual Studio 15" -DPDC_WIDE=ON -DCMAKE_INSTALL_PREFIX=c:\tmp\pdcurses\Win32 -DCMAKE_BUILD_TYPE=DEBUG -DPDCDEBUG=ON ..
	popd
	cmake --build build32 --config Debug --target install

Win64 (pdcurses.sln)

    mkdir build64 & pushd build64
	cmake -G"Visual Studio 15 Win64" -DPDC_WIDE=ON -DCMAKE_INSTALL_PREFIX=c:\tmp\pdcurses\Win64 -DCMAKE_BUILD_TYPE=Debug -DPDCDEBUG=ON ..
	popd
	cmake --build build64 --config Debug --target install

Build script example

From pdcurses source root

    cmake\vsvs2015_build.cmd


Available CMake Visual Studio Generators


    Visual Studio 15 [arch]      = Generates Visual Studio 15 project files.
                                 Optional [arch] can be "Win64" or "ARM".

    Visual Studio 14 2015 [arch] = Generates Visual Studio 2015 project files.
                                 Optional [arch] can be "Win64" or "ARM".

    Visual Studio 12 2013 [arch] = Generates Visual Studio 2013 project files.
                                 Optional [arch] can be "Win64" or "ARM".

    Visual Studio 11 2012 [arch] = Generates Visual Studio 2012 project files.
                                 Optional [arch] can be "Win64" or "ARM".

    Visual Studio 10 2010 [arch] = Generates Visual Studio 2010 project files.
                                 Optional [arch] can be "Win64" or "IA64".

    Visual Studio 9 2008 [arch]  = Generates Visual Studio 2008 project files.
                                 Optional [arch] can be "Win64" or "IA64".

    Visual Studio 8 2005 [arch]  = Generates Visual Studio 2005 project files.
                                 Optional [arch] can be "Win64".


Cygwin
------

    mkdir build && pushd build
    cmake .. -G"Unix Makefiles" -DPDC_SDL2_BUILD=OFF -DCMAKE_INSTALL_PREFIX=/cygdrive/c/tmp/pdcurses/Cyg64 -DCMAKE_BUILD_TYPE=DEBUG -DPDCDEBUG=ON -DWINDOWS_KIT_LIBRARY_DIR=/cygdrive/c/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.14393.0/um/x64 ..
    popd
	cmake --build build --config Debug --target install

Note: The demo apps will all build, only version.exe works.  All other apps print "Redirection is not supported."


Linux Building
--------------

SDL2 (Only project currently supported with CMake)

    cmake .. -DPDC_WIDE=ON -DCMAKE_INSTALL_PREFIX=/home/joel/git/PDCurses/build64/out -DCMAKE_BUILD_TYPE=DEBUG
    make install -j16

