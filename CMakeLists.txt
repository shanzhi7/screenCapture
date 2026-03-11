cmake_minimum_required(VERSION 3.19)
project(screenCapture LANGUAGES CXX)

option(SCREENCAPTURE_ENABLE_EXPERIMENTAL_LONG_CAPTURE "Enable experimental Windows long capture pipeline" ON)

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
    settingsservice.cpp
    settingsservice.h
    tippresenter.cpp
    tippresenter.h
)

if (WIN32)
    target_sources(screenCapture PRIVATE appicon.rc)
endif()

if (SCREENCAPTURE_ENABLE_EXPERIMENTAL_LONG_CAPTURE AND WIN32)
    target_compile_definitions(screenCapture PRIVATE SCREENCAPTURE_ENABLE_LONG_CAPTURE=1)
    target_sources(screenCapture PRIVATE
        longcapturetypes.h
        longcapturebackend.h
        desktopgdicapturebackend.cpp
        desktopgdicapturebackend.h
        wgccapturebackend.cpp
        wgccapturebackend.h
        dxgiduplicationbackend.cpp
        dxgiduplicationbackend.h
        scrolldispatcher.cpp
        scrolldispatcher.h
        motiondetector.cpp
        motiondetector.h
        stableframecollector.cpp
        stableframecollector.h
        overlapmatcher.cpp
        overlapmatcher.h
        stitchcomposer.cpp
        stitchcomposer.h
        longcapturesession.cpp
        longcapturesession.h
        longcapturesessioncontroller.cpp
        longcapturesessioncontroller.h
        longcapturepreviewpanel.cpp
        longcapturepreviewpanel.h
    )
else()
    target_compile_definitions(screenCapture PRIVATE SCREENCAPTURE_ENABLE_LONG_CAPTURE=0)
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
        icons/overlay_pen.svg
        icons/overlay_rect.svg
        icons/overlay_ellipse.svg
        icons/overlay_mosaic.svg
        icons/overlay_text.svg
        icons/overlay_scroll.svg
        icons/overlay_save.svg
        icons/overlay_close.svg
        icons/overlay_check.svg
        icon.ico
)

target_link_libraries(screenCapture
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Svg
)

if (SCREENCAPTURE_ENABLE_EXPERIMENTAL_LONG_CAPTURE AND WIN32)
    target_link_libraries(screenCapture PRIVATE d3d11 dxgi)
endif()

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
