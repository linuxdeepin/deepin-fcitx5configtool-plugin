#cmake版本设置 
cmake_minimum_required(VERSION 3.7)
project(deepin-fcitx5configtool-plugin VERSION 5.0.5)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(XKeyboardConfig REQUIRED)
find_package(IsoCodes REQUIRED)
configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#加载子目录
add_subdirectory(src)

#调用外部程序  生成qm文件
execute_process(COMMAND bash translate_generation.sh
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
