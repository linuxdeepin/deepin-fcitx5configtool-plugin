add_executable(fcitx5-plasma-theme-generator main.cpp)

message(WARNING "Plasma theme generator: depends libkf5plasma-dev libkf5i18n-dev")
find_package(KF5Plasma REQUIRED)
find_package(KF5IconThemes REQUIRED)
find_package(KF5I18n REQUIRED)

target_link_libraries(fcitx5-plasma-theme-generator KF5::Plasma KF5::IconThemes KF5::I18n Fcitx5::Utils Fcitx5::Config)

install(TARGETS fcitx5-plasma-theme-generator DESTINATION ${CMAKE_INSTALL_BINDIR})
