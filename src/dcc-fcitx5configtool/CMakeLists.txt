cmake_minimum_required(VERSION 3.7)

set(PLUGIN_NAME "dcc-fcitx5configtool")

project(${PLUGIN_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")

# 增加安全编译参数
ADD_DEFINITIONS("-fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

find_package(Qt${QT_DESIRED_VERSION} COMPONENTS Core Gui DBus LinguistTools Qml REQUIRED)
find_package(DdeControlCenter REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Core REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(PolkitQt6-1 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ICU REQUIRED icu-uc icu-i18n)


set(DccFcitx5configtool_Name fcitx5configtool)
file(GLOB_RECURSE DccFcitx5configtool_SRCS
        "operation/qrc/*.qrc"
        "keyboard-layout/operation/qrc/*.qrc"
        "*.cpp"
        "*.h"
        "*.qml"
        "private/*.h"
        "operation/*.cpp"
        "operation/*.h"
        "keyboard-layout/operation/*.cpp"
        "keyboard-layout/operation/*.h"
        "keyboard-layout/qml/*.qml"
)

add_library(${DccFcitx5configtool_Name} MODULE
        ${DccFcitx5configtool_SRCS}
)

target_include_directories(${DccFcitx5configtool_Name} PUBLIC
    Dde::Control-Center
    configlib${QT_DESIRED_VERSION}
    configwidgetslib${QT_DESIRED_VERSION}
    keyboard-layout/operation
    ${ICU_INCLUDE_DIRS}
)

set(DccFcitx5configtool_Libraries
        Qt${QT_DESIRED_VERSION}::DBus
        Qt${QT_DESIRED_VERSION}::Gui
        Qt${QT_DESIRED_VERSION}::Qml
        Dtk${DTK_VERSION_MAJOR}::Core
        Dde::Control-Center
        PolkitQt6-1::Agent
        ${ICU_LIBRARIES}
)

target_link_libraries(${DccFcitx5configtool_Name} PRIVATE
        ${DccFcitx5configtool_Libraries}
        configlib${QT_DESIRED_VERSION}
        configwidgetslib${QT_DESIRED_VERSION}
)

dcc_install_plugin(NAME ${DccFcitx5configtool_Name} TARGET ${DccFcitx5configtool_Name})
dcc_handle_plugin_translation(NAME ${DccFcitx5configtool_Name})
