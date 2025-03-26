include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
option(ENABLE_KCM "Enable KDE Config Module" On)

find_package(Fcitx5Qt${QT_DESIRED_VERSION}DBusAddons REQUIRED)
find_package(Fcitx5Qt${QT_DESIRED_VERSION}WidgetsAddons REQUIRED)
find_package(KF${KF_VERSION_MAJOR}ItemViews REQUIRED)
find_package(KF${KF_VERSION_MAJOR}WidgetsAddons REQUIRED)
kde_enable_exceptions()

configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(layout)
add_subdirectory(configlib)
add_subdirectory(configwidgetslib)
