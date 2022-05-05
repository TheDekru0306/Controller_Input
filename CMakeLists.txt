cmake_minimum_required(VERSION 3.17)
project(DebbugJSInput)

set(CMAKE_CXX_STANDARD 14)

add_executable(DebbugJSInput main.cpp Versions/JS_ZERO_VERSION/JS_ZERO.cpp Versions/JS_ZERO_VERSION/JS_ZERO.h Versions/EVENT_BASED_VERSION/Event_Based.cpp Versions/EVENT_BASED_VERSION/Event_Based.h Versions/EVENT_BASED_VERSION/PS4_CODES.h Versions/HidBased/HID_Based.cpp Versions/HidBased/HID_Based.h Versions/HidBased/hidapi.h Versions/HidBased/HID_Related_Info.h Versions/HidBased/PS4_HID.h Versions/HidBased/HID_Usages.h Versions/HidBased/CRCTable.h Versions/EVENT_BASED_VERSION/XBOX_CODES.h Versions/Device_Detector.cpp Versions/Device_Detector.h Versions/Timer.cpp Versions/Timer.h Versions/EVENT_BASED_VERSION/Event_Based_Structs.h Versions/EVENT_BASED_VERSION/VibrationEffect.cpp Versions/EVENT_BASED_VERSION/VibrationEffect.h)
target_link_libraries(DebbugJSInput udev)