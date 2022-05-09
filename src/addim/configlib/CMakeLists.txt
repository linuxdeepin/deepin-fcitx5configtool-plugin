add_library(configlib STATIC
    addim_model.cpp
    glo.cpp
)
set_target_properties(configlib PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    AUTOMOC TRUE
    AUTOUIC TRUE
    AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h"
)
target_include_directories(configlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
#target_link_libraries(configlib Qt5::Core Qt5::Gui Fcitx5Qt5::DBusAddons Fcitx5::Core Fcitx5::Utils)
target_link_libraries(configlib Qt5::Core Qt5::Gui Fcitx5Qt5::DBusAddons Fcitx5::Core Fcitx5::Utils pthread)
