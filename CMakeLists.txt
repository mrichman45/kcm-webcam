cmake_minimum_required(VERSION 3.22)
project(kcm-webcam VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM 6.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Qml Quick)
find_package(KF6 6.0 REQUIRED COMPONENTS KCMUtils Config CoreAddons I18n)

add_subdirectory(core)
add_subdirectory(kcm)
add_subdirectory(daemon)

install(FILES webcam-settings.service DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR})
