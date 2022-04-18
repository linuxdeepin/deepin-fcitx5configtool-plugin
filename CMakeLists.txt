#cmake版本设置 
cmake_minimum_required(VERSION 3.7)
project(deepin-fcitxconfigtool-plugin)

#加载子目录
add_subdirectory(src)

#调用外部程序  生成qm文件
execute_process(COMMAND bash translate_generation.sh
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

install(FILES com.deepin.fcitx-config.gschema.xml DESTINATION share/glib-2.0/schemas)
install(CODE "execute_process(COMMAND glib-compile-schemas ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas)")
