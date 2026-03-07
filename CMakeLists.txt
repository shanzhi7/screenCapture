cmake_minimum_required(VERSION 3.19)
project(screenCapture LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Svg)

qt_standard_project_setup()

qt_add_executable(screenCapture
    WIN32
    MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    selectionoverlay.cpp
    selectionoverlay.h
    showtip.cpp
    showtip.h
    showtip.ui
    closedialog.cpp
    closedialog.h
    closedialog.ui
    capturesettingsdialog.cpp
    capturesettingsdialog.h
    capturesettingsdialog.ui
    aboutdialog.cpp
    aboutdialog.h
    aboutdialog.ui
    globalhotkeymanager.cpp
    globalhotkeymanager.h
    capturehistorymanager.cpp
    capturehistorymanager.h
    captureuistatecoordinator.cpp
    captureuistatecoordinator.h
    captureresulthandler.cpp
    captureresulthandler.h
    longcapturestitcher.cpp
    longcapturestitcher.h
    settingsservice.cpp
    settingsservice.h
    tippresenter.cpp
    tippresenter.h
)

if (WIN32)
    target_sources(screenCapture PRIVATE appicon.rc)
endif()

# 固定源码编码：在 MSVC 下强制使用 UTF-8，避免中文字符串乱码。
if (MSVC)
    target_compile_options(screenCapture PRIVATE /utf-8)
endif()

qt_add_resources(screenCapture app_resources
    PREFIX "/"
    FILES
        styles/hyperos.qss
        icons/camera.svg
        icons/folder.svg
        icons/settings.svg
        icons/chevron_left.svg
        icons/chevron_right.svg
        icons/crop.svg
        icons/history.svg
        icons/plus_white.svg
        icons/info.svg
        icon.ico
)

target_link_libraries(screenCapture
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Svg
)

include(GNUInstallDirs)

install(TARGETS screenCapture
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET screenCapture
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
