include(E:/qt_project/screenCapture/.codex_cmake_check_mingw/.qt/QtDeploySupport.cmake)
include("${CMAKE_CURRENT_LIST_DIR}/screenCapture-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_ALL_MODULES_FOUND_VIA_FIND_PACKAGE "ZlibPrivate;EntryPointPrivate;Core;Gui;Widgets;Svg")

qt6_deploy_runtime_dependencies(
    EXECUTABLE E:/qt_project/screenCapture/.codex_cmake_check_mingw/screenCapture.exe
    GENERATE_QT_CONF
)
