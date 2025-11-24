cmake_minimum_required(VERSION 3.7)

set(BIN_NAME "dcc-fcitx5configtool-exec")

project(${BIN_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")

# 增加安全编译参数
ADD_DEFINITIONS("-fstack-protector-strong -D_FORTIFY_SOURCE=2 -z noexecstack -pie -fPIC -z lazy")

# 指定要用到的库
find_package(Qt${QT_DESIRED_VERSION} COMPONENTS Core Gui Widgets REQUIRED)
find_package(Fcitx5Qt${QT_DESIRED_VERSION}DBusAddons REQUIRED)

file(GLOB_RECURSE DccFcitx5configtool_SRCS
    "*.cpp"
    "*.h"
)

add_executable(${BIN_NAME}
    ${DccFcitx5configtool_SRCS}
)

set(DccFcitx5configtool_Libraries
        Qt${QT_DESIRED_VERSION}::Gui
        Qt${QT_DESIRED_VERSION}::Widgets
        Dtk${DTK_VERSION_MAJOR}::Core
)

target_link_libraries(${BIN_NAME} PRIVATE
        ${DccFcitx5configtool_Libraries}
        configlib${QT_DESIRED_VERSION}
        configwidgetslib${QT_DESIRED_VERSION}
        Fcitx5Qt${QT_DESIRED_VERSION}::DBusAddons
)

target_include_directories(${BIN_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/lib/configwidgetslib
)

message(STATUS "Install ${BIN_NAME} to ${CONFIGTOOL_EXEC_DIR}")
message(STATUS "CONFIGTOOL_EXEC_DIR=${CONFIGTOOL_EXEC_DIR}")
install(TARGETS ${BIN_NAME} DESTINATION ${CONFIGTOOL_EXEC_DIR})
