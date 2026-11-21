add_library(webcamcore STATIC v4l2camera.cpp v4l2camera.h)
set_target_properties(webcamcore PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(webcamcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(webcamcore PUBLIC Qt6::Core KF6::ConfigCore)
