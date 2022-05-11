#cmake版本设置 
cmake_minimum_required(VERSION 3.7)
project(deepin-fcitx5configtool-plugin)

#加载子目录
add_subdirectory(src)

#调用外部程序  生成qm文件
execute_process(COMMAND bash translate_generation.sh
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
