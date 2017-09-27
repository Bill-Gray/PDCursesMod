
REM ################################
REM # Debug
REM ################################
rmdir build32 /q /s
mkdir build32 & pushd build32
cmake -G"Visual Studio 15" -DCMAKE_INSTALL_PREFIX=c:\tmp\pdcurses\Win32 -DCMAKE_BUILD_TYPE=Debug -DPDC_WIDE=ON -DPDCDEBUG=ON ..
popd
rmdir build64 /q /s
mkdir build64 & pushd build64
cmake -G"Visual Studio 15 Win64" -DCMAKE_INSTALL_PREFIX=c:\tmp\pdcurses\Win64 -DCMAKE_BUILD_TYPE=Debug -DPDC_WIDE=ON -DPDCDEBUG=ON ..
popd
rmdir buildarm /q /s
mkdir buildarm & pushd buildarm
cmake -G"Visual Studio 15 ARM" -DPDC_SDL2_BUILD=OFF -DCMAKE_INSTALL_PREFIX=c:\tmp\pdcurses\ARM -DCMAKE_BUILD_TYPE=Debug -DPDC_WIDE=ON -DPDCDEBUG=ON ..
popd

cmake --build build32 --config Debug --target install
cmake --build build64 --config Debug --target install
cmake --build buildarm --config Debug --target install


REM ################################
REM # Release
REM ################################
pushd build32
cmake -G"Visual Studio 15" -DPDC_SDL2_BUILD=ON -DCMAKE_INSTALL_PREFIX=c:\tmp\pdcurses\Win32 -DCMAKE_BUILD_TYPE=Release -DPDC_WIDE=ON -DPDCDEBUG=OFF ..
popd
pushd build64
cmake -G"Visual Studio 15 Win64" -DPDC_SDL2_BUILD=ON -DCMAKE_INSTALL_PREFIX=c:\tmp\pdcurses\Win64 -DCMAKE_BUILD_TYPE=Release -DPDC_WIDE=ON -DPDCDEBUG=OFF ..
popd
pushd buildarm
cmake -G"Visual Studio 15 ARM" -DPDC_SDL2_BUILD=OFF -DCMAKE_INSTALL_PREFIX=c:\tmp\pdcurses\ARM -DCMAKE_BUILD_TYPE=Release -DPDC_WIDE=ON -DPDCDEBUG=OFF ..
popd
 
cmake --build build32 --config Release --target install
cmake --build build64 --config Release --target install
cmake --build buildarm --config Release --target install
