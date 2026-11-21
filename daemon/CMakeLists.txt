add_executable(webcam-settings-apply main.cpp)
target_link_libraries(webcam-settings-apply PRIVATE webcamcore)
install(TARGETS webcam-settings-apply DESTINATION ${KDE_INSTALL_BINDIR})
