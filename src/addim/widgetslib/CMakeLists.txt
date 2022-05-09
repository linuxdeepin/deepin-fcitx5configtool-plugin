add_library(configwidgetslib STATIC
    addim_window.cpp
    categoryhelper.cpp
    varianthelper.cpp
    impage.cpp
)
set_target_properties(configwidgetslib PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    AUTOMOC TRUE
    AUTOUIC TRUE
    AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h"
)

#”¶”√DTKø‚
set(DTK Widget Core Gui)
find_package(Dtk COMPONENTS ${DTK} REQUIRED)

target_include_directories(configwidgetslib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(configwidgetslib Qt5::Core Qt5::Widgets Qt5::X11Extras KF5::WidgetsAddons
    KF5::ItemViews Fcitx5::Utils Fcitx5Qt5::DBusAddons Fcitx5Qt5::WidgetsAddons
    configlib layoutlib
    dtkcore
    dtkwidget
	)
