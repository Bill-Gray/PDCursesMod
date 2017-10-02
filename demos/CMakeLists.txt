# Handle RPATH settings on unix in case of building shared library
IF(PDCURSES_BUILD_SHARED_LIBS AND UNIX)
    # use, i.e. don't skip the full RPATH for the build tree
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
    # when building, don't use the install RPATH already
    # (but later on when installing)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    # the RPATH to be used when installing, but only if it's not a system directory
    LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
    IF("${isSystemDir}" STREQUAL "-1")
        SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    ENDIF("${isSystemDir}" STREQUAL "-1")
ENDIF()

add_executable(firework firework.c)
add_executable(newdemo newdemo.c)
add_executable(ptest ptest.c)
add_executable(rain rain.c)
add_executable(testcurs testcurs.c)
add_executable(tuidemo tuidemo.c tui.c tui.h)
add_executable(worm worm.c)
add_executable(xmas xmas.c)

target_link_libraries(firework XCurses)
target_link_libraries(newdemo XCurses)
target_link_libraries(ptest XCurses)
target_link_libraries(rain XCurses)
target_link_libraries(testcurs XCurses)
target_link_libraries(tuidemo XCurses)
target_link_libraries(worm XCurses)
target_link_libraries(xmas XCurses)

install(TARGETS firework DESTINATION examples)
install(TARGETS newdemo DESTINATION examples)
install(TARGETS ptest DESTINATION examples)
install(TARGETS rain DESTINATION examples)
install(TARGETS testcurs DESTINATION examples)
install(TARGETS tuidemo DESTINATION examples)
install(TARGETS worm DESTINATION examples)
install(TARGETS xmas DESTINATION examples)
