#ifndef DEBBUGJSINPUT_EVENT_BASED_H
#define DEBBUGJSINPUT_EVENT_BASED_H
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <set>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstring>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <queue>

#include "Event_Based_Structs.h"
#include "../Device_Detector.h"
#include "../Timer.h"
#include "VibrationEffect.h"


class Event_Based {
public:
    void Update();
    void Initialize();

private:
    int touchpad;
    int motionSensor;
    input_event event;
    std::vector<Event_Devices_Info> devices;
    std::unordered_map<int, Controller> controllers;
    std::unordered_map<int, std::queue<VibrationEffect>> vibrationRequests;
    std::vector<Detected_Device_Info> collection;
    int requestedEffectsSize_Max = 10;

    Device_Detector deviceDetector;

    int ReadEvent(int controller, struct input_event* event);
    int ReadPS4Input(uint16_t input, int value, int controller);
    int ReadXBOX360Input(uint16_t input, int value, int controller);
    void GetDevicesPath();
    void VIBRATOR_MOD(int controller, uint16_t timeInterval, uint16_t vibrationForce = 0, int r = 0, int g = 0, int b = 0);
    void InitializeControllers();
    void UpdateControllerList();
    void ControllerDevicesUpdate();
    void VerifyOtherDevices();
};


#endif //DEBBUGJSINPUT_EVENT_BASED_H
