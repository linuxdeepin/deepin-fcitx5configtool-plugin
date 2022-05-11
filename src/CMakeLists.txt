#版本设置
set(VERSION 34.1)
#二进制名称
set(BIN_NAME deepin-fcitxconfigtool-plugin)
#项目名称设置
project(${BIN_NAME})

#龙芯机器缺少符号表
if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
    option(USE_MIPS64 "USE MIPS64" ON)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wl,-O1 -Wl,--as-needed -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -z noexecstack")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wl,-O1 -Wl,--as-needed -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -z noexecstack")
    SET(CMAKE_EXE_LINKER_FLAGS  "-pie")
endif()

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -pthread")
  # Enable Qt builtin debug mode
  add_definitions("-DQT_MESSAGELOGCONTEXT -std=c++11")
else()
  # -Wl, -O2 Enable linker optimizations
  # -Wl, --gc-sections Remove unused code resulting from -fdsta-sections and
  # -ffunction-sections
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wl,--gc-sections")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wl,--gc-sections -pthread")
endif()

# 环境设置
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)#moc 预编译包含QObject文件，生成xx_moc.h cpp
set(CMAKE_AUTORCC ON)#开启rcc 编译资源文件qrc
set(CMAKE_AUTOUIC ON)#开启uic 编译转换*.ui文件生存设计界面对应的.h .cpp文件
set(CMAKE_CXX_STANDARD 17)

# 查找库
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DtkCMake REQUIRED)
find_package(DdeControlCenter REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Fcitx5Core 5.0.4 REQUIRED)
find_package(Fcitx5Config REQUIRED)
find_package(Fcitx5Utils REQUIRED)
find_package(Fcitx5Qt5DBusAddons REQUIRED)
find_package(Fcitx5Qt5WidgetsAddons 5.0.2 REQUIRED)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)
#设置资源目录 加载资源文件 翻译文件 源码
set(APP_RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../res")
set(APP_QRC "${APP_RES_DIR}/fcitxconfig.qrc")
set(ADDIM_QRC "${CMAKE_CURRENT_SOURCE_DIR}/addim/resources.qrc")
#创建翻译文件 
#set(APP_QRC "${TS_DIR}/translations.qrc")
set(TS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../translations")
set(TS_FILES "${TS_DIR}/${PROJECT_NAME}_en_US.ts")
#更新翻译
find_program(LUPDATE_EXECUTABLE lupdate)
foreach(_ts_file ${TS_FILES})
    execute_process(
        COMMAND ${LUPDATE_EXECUTABLE} -recursive ${CMAKE_SOURCE_DIR} -ts ${_ts_file})  #递归扫描源码 输出ts文件
endforeach()

#创建qm文件
file (GLOB DTNG_TS_FILES ${TS_DIR}/*.ts)
#qt5_add_translation(DTNG_QM_FILES ${CMAKE_SOURCE_DIR} ${DTNG_TS_FILES})
add_custom_target(translations ALL DEPENDS ${DTNG_QM_FILES})

#set(UI_HEADER
#    ${CMAKE_CURRENT_SOURCE_DIR}/fcitxconfigplugin.h
#    )
#QT5_WRAP_CPP(UI_HEADER_MOCK ${UI_HEADER})

FILE(GLOB_RECURSE HEAD_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/fcitx5Interface/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/fcitx5Interface/configwidgetslib/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/window/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/fcitxconfigplugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/publisher/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/addim/configlib/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/addim/widgetslib/*.h
)

FILE(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/fcitx5Interface/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fcitx5Interface/configwidgetslib/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/window/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fcitxconfigplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/publisher/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/publisher/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/addim/configlib/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/addim/configlib/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/addim/widgetslib/*.cpp
)

FILE(GLOB_RECURSE TEST_HEAD_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../tests/*.h)

FILE(GLOB_RECURSE TEST_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../tests/*.cpp)

set(ALL_SRCS
    ${HEAD_FILES}
    ${SOURCE_FILES}
    ${APP_QRC}
    ${ADDIM_QRC}
    ${DTNG_QM_FILES}
    )

set(TEST_SRCS
    ${TEST_HEAD_FILES}
    ${TEST_SOURCE_FILES}
    )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/addim/configlib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/publisher)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/fcitx5Interface)

#add_executable(${PROJECT_NAME}
#    ${ALL_SRCS}
#    ${DTNG_QM_FILES}
#    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
#    )

add_library(${PROJECT_NAME} SHARED
        ${ALL_SRCS}
        ${DTNG_QM_FILES}
       # ${TS_DIR}/translations.qrc
        #${UI_HEADER_MOCK}
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    AUTOMOC TRUE
    AUTOUIC TRUE
    AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h"
)

get_target_property(FCITX5_QT5_GUI_WRAPPER Fcitx5Qt5::gui-wrapper IMPORTED_LOCATION)
#include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")
target_include_directories(${PROJECT_NAME} PUBLIC
        "/usr/include/dde-control-center"
        ${DdeControlCenter_INCLUDE_DIRS}
        ${DtkCore_INCLUDE_DIRS}
        ${DtkWidget_INCLUDE_DIRS}
        ${DFrameworkDBus_INCLUDE_DIRS}
        ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
        ${GSETTINGS_INCLUDE_DIRS}
        )

target_link_libraries(${PROJECT_NAME} PRIVATE
        "/usr/lib/dde-control-center"
        dcc-interface
        ${DdeControlCenter_LIBRARIES}
        ${DFrameworkDBus_LIBRARIES}
        ${DtkWidget_LIBRARIES}
        ${DtkCore_ILIBRARIES}
        ${Qt5Widgets_LIBRARIES}
        ${Qt5DBus_LIBRARIES}
        ${OBJECT_BINARY_DIR}
        Fcitx5::Core
        Fcitx5::Utils
        Fcitx5Qt5::DBusAddons
        Fcitx5Qt5::WidgetsAddons
        ${GSETTINGS_LIBRARIES}
        )

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

# 设置执行 make install 时哪个目标应该被 install 到哪个位置
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION /lib/dde-control-center/modules)

# 安装 .qm 文件
install(FILES ${DTNG_QM_FILES} DESTINATION share/${PROJECT_NAME}/translations)

