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

# 指定要用到的库
# -DQT_MAJOR_VERSION=6
# find_package(QT NAMES Qt6 Qt5)
# set(QT_MAJOR_VERSION 6)
# set(QT_DEFAULT_MAJOR_VERSION 6)
find_package(Qt6 COMPONENTS Core Gui DBus LinguistTools REQUIRED)
find_package(DdeControlCenter REQUIRED)
find_package(Dtk6 COMPONENTS Core REQUIRED)
find_package(PkgConfig REQUIRED)


set(DccFcitx5configtool_Name fcitx5configtool)
file(GLOB_RECURSE DccFcitx5configtool_SRCS
        "operation/qrc/*.qrc"
        "*.cpp"
        "*.h"
        "operation/*.cpp"
        "operation/*.h"
)

add_library(${DccFcitx5configtool_Name} MODULE
        ${DccFcitx5configtool_SRCS}
)

target_include_directories(${DccFcitx5configtool_Name} PUBLIC
    Dde::Control-Center
    configlib${QT_MAJOR_VERSION}
    configwidgetslib${QT_MAJOR_VERSION}
)

set(DccFcitx5configtool_Libraries
        Qt${QT_MAJOR_VERSION}::DBus
        Qt${QT_MAJOR_VERSION}::Gui
        Dtk${QT_MAJOR_VERSION}::Core
        Dde::Control-Center
)

target_link_libraries(${DccFcitx5configtool_Name} PRIVATE
        ${DccFcitx5configtool_Libraries}
        configlib${QT_MAJOR_VERSION}
        configwidgetslib${QT_MAJOR_VERSION}
)

dcc_install_plugin(NAME ${DccFcitx5configtool_Name} TARGET ${DccFcitx5configtool_Name})
