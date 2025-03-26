cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")

# 增加安全编译参数
ADD_DEFINITIONS("-fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

find_package(Qt${QT_DESIRED_VERSION} COMPONENTS Core Gui DBus LinguistTools REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Core Widget REQUIRED)


include(GNUInstallDirs)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

if(NOT DEFINED CONFIGTOOL_EXEC_DIR)
    set(CONFIGTOOL_EXEC_DIR ${CMAKE_INSTALL_LIBEXECDIR})
endif()

add_definitions(-DDCC_CONFIGTOOL_EXEC_PATH=\"${CONFIGTOOL_EXEC_DIR}\")

add_subdirectory(lib)
add_subdirectory(dcc-fcitx5configtool)
add_subdirectory(dcc-fcitx5configtool-exec)
