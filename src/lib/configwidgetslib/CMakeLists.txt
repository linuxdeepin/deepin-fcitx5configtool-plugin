set(PLUGIN_NAME "configwidgetslib${QT_DESIRED_VERSION}")

add_library(${PLUGIN_NAME} STATIC
    configwidget.cpp
    addonselector.cpp
    categoryhelper.cpp
    erroroverlay.cpp
    fontbutton.cpp
    keylistwidget.cpp
    layoutselector.cpp
    listoptionwidget.cpp
    optionwidget.cpp
    varianthelper.cpp
    verticalscrollarea.cpp
    impage.cpp
)

set_target_properties(${PLUGIN_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    AUTOMOC TRUE
    AUTOUIC TRUE
    AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h"
)

target_include_directories(${PLUGIN_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PLUGIN_NAME}
    Qt${QT_DESIRED_VERSION}::Core
    Qt${QT_DESIRED_VERSION}::Widgets
    KF${KF_VERSION_MAJOR}::WidgetsAddons
    KF${KF_VERSION_MAJOR}::ItemViews
    Fcitx5::Utils
    Fcitx5Qt${QT_DESIRED_VERSION}::DBusAddons
    Fcitx5Qt${QT_DESIRED_VERSION}::WidgetsAddons
    configlib${QT_DESIRED_VERSION}
    layoutlib${QT_DESIRED_VERSION})
