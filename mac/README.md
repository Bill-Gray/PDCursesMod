PDCursesMod for Macintosh Classic/Carbon
=========================================

This directory contains PDCursesMod source code files 
specific to the Macintosh classic/carbon graphics mode 
(Macintosh classic has no command line, so this is the only 
mode there). This will work with System 7 and early OSX with
CarbonLib on either 68k or PowerPC.

Building
--------

- You can cross-compile with Retro68 using CMake. This is
  the main way to do it as you get a modern compiler with
  modern optimizations. Some Linux distros have a package 
  for Retro68, but for those that don't there are a couple 
  options:
  
  1) The main Retro68 repo at https://github.com/autc04/Retro68/
  2) Matthewdeaves's fork at https://github.com/matthewdeaves/Retro68 which includes a more hands-free setup script
 
- A CodeWarrior 8 file is included for native compilation 
  and debugging. The non-CMake options include:

        pdcursescw8mac.sit - Macintosh CodeWarrior 8 (non-Pro) 68k project file
    
- Use Stuffit Expander to extract the native Macintosh CodeWarrior 8
  project. It is compressed to preserve its resources.
    
- The CodeWarrior project files include only the library itself. 
  If you want compile a demo, test, or your own application, 
  either add it as a target to the project or make your own 
  project file and add PDCursesMod as a library.

- If making an application include pdcurses.r or your own resource file.

- As mentioned above this port supports several architectures:

        68k - Builds for early Macintosh Classic non-PowerPC
        PowerPC - Builds for Macintosh Classic PowerPC
        FAT - Builds for Macintosh Classic 68k/PowerPC hybrid
        Carbon - Builds for Macintosh Carbon 1.0
        
- Carbon requires System 8+ with the CarbonLib extension on 
  Macintosh Classic or early PowerPC-based Mac OSX.

Retro68 CMake Information
------------------------------ 

Using Retro68 with CMake generally requires its own cmake 
toolchain file. Typically you'd have this already set up. For
example if Retro68 was setup in /home/user/Retro68-build your
toolchain would look something like:

    if(RETROPOWERPCOPT OR RETROCARBONOPT)
        set(CMAKE_AR /home/user/Retro68-build/toolchain/bin/powerpc-apple-macos-ar)
        set(CMAKE_RANLIB /home/user/Retro68-build/toolchain/bin/powerpc-apple-macos-ranlib)
        set(CMAKE_LINKER /home/user/Retro68-build/toolchain/bin/powerpc-apple-macos-ld)
        set(CMAKE_C_COMPILER /home/user/Retro68-build/toolchain/bin/powerpc-apple-macos-gcc)
        set(CMAKE_CXX_COMPILER /home/user/Retro68-build/toolchain/bin/powerpc-apple-macos-g++)
        if(RETROCARBONOPT)
            set(CMAKE_EXE_LINKER_FLAGS_INIT "-carbon")
            add_definitions(-DTARGET_API_MAC_CARBON=1)
            set(CMAKE_SYSTEM_NAME RetroCarbon)
        else()
            set(CMAKE_SYSTEM_NAME RetroPPC)
        endif()
    else()
        set(CMAKE_AR /home/user/Retro68-build/toolchain/bin/m68k-apple-macos-ar)
        set(CMAKE_RANLIB /home/user/Retro68-build/toolchain/bin/m68k-apple-macos-ranlib)
        set(CMAKE_LINKER /home/user/Retro68-build/toolchain/bin/m68k-apple-macos-ld)
        set(CMAKE_C_COMPILER /home/user/Retro68-build/toolchain/bin/m68k-apple-macos-gcc)
        set(CMAKE_CXX_COMPILER /home/user/Retro68-build/toolchain/bin/m68k-apple-macos-g++)
        set(CMAKE_SYSTEM_NAME Retro68)
    endif()
    
    set(REZ /home/user/Retro68-build/toolchain/bin/Rez)

Once you have that set up you'd either pass it to the commandline or 
add it to the cmake files via:

    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/retro68-gcc-toolchain.cmake)

Distribution Status
-------------------

The files in this directory are released to the Public Domain.

Acknowledgements
----------------

Structure based on the Win32 GUI port by Bill Gray.
