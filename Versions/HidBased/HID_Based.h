#ifndef DEBBUGJSINPUT_HID_BASED_H
#define DEBBUGJSINPUT_HID_BASED_H

#include "HID_Related_Info.h"
#include "HID_Usages.h"
#include <string>

class HID_Based {
public:
    void Intialize();
    void Update();
    BytesAndBits getBytesAndBits(const uint32_t input);

private:
    int controller;
    unsigned char buffer[256];
    uint8_t PS4_buttons[14] = {0};
    HID_Parsed_Data parsed_data;
    hidraw_report_descriptor report_Description;
    std::map<uint16_t , Device_Info> devices;

    void Load_Device_HID_Capabilities();
    void ControllerUpdate();
    void VIBRATOR_MOD(int timeInterval,  int vibrationForceLeft = 0, int vibrationForceRight = 0, int r = 0, int g = 0, int b = 0,  bool bluetooth = false);
    uint32_t getValue(const hidraw_report_descriptor &reportDescription, int counter, uint8_t &bytesToRead);
    uint32_t Extract_Value(HID_Usage info, unsigned char buffer[]);
    bool getState(uint32_t buttons, uint8_t index);
    uint32_t CRC(unsigned char *buffer, int buffer_size);
};


#endif //DEBBUGJSINPUT_HID_BASED_H
