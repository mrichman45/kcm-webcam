kcmutils_add_qml_kcm(kcm_webcam SOURCES webcamkcm.cpp webcamkcm.h)

target_link_libraries(kcm_webcam PRIVATE
    webcamcore
    Qt6::Quick
    KF6::KCMUtilsQuick
    KF6::CoreAddons
    KF6::I18n
)
