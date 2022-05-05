#ifndef DEBBUGJSINPUT_HID_RELATED_INFO_H
#define DEBBUGJSINPUT_HID_RELATED_INFO_H

#include "hidapi.h"
#include <map>
#include <string>
#include <linux/input.h>
#include <fcntl.h>
#include <iostream>
#include <linux/hidraw.h>
#include <vector>

enum class Device_VenderId: uint16_t {
    SONY = 0x054c,
    MS = 0x045e
};
enum class Devide_ProductId: uint16_t {
    PS4 = 0x09cc,
    XBOX_DICK = 0x028e
};

struct Device_Info{
    //std::string manufacturer;
    //std::string product;
    std::string path;
    uint16_t productID;
};

struct BytesAndBits{
    uint8_t bytes = 0;
    uint8_t bits = 0;

    uint8_t toBits(){
        return this->bits + (this->bytes * 8);
    }

    friend BytesAndBits operator+(BytesAndBits first, BytesAndBits second){
        BytesAndBits result;

        if(first.bits + second.bits >= 8){
            result.bits = first.bits + second.bits - 8;
            result.bytes++;
        }
        else{
            result.bits = first.bits + second.bits;
        }

        result.bytes += first.bytes + second.bytes;
        return result;
    }
    friend BytesAndBits operator-(BytesAndBits first, BytesAndBits second){
        BytesAndBits result;

        if(first.bits - second.bits < 0){
            result.bits = first.bits - second.bits + 8;
            if(result.bytes > 0){
                result.bytes--;
            }
        }
        else{
            result.bits = first.bits - second.bits;
        }

        result.bytes += first.bytes - second.bytes;
        return result;
    }
    BytesAndBits operator+=(BytesAndBits second){

        if(this->bits + second.bits >= 8){
            this->bits = this->bits + second.bits - 8;
            this->bytes++;
        }
        else{
            this->bits = this->bits + second.bits;
        }

        this->bytes += second.bytes;
        return *this;

    }
};

struct HID_Usage{
    uint32_t page = 0;
    uint32_t usage_min = 0;
    uint32_t usage_max = 0;
    uint32_t physical_min = 0;
    uint32_t physical_max = 0;
    uint32_t unit_exponent = 0;
    uint32_t unit = 0;
    uint32_t logical_min = 0;
    uint32_t logical_max = 0;
    BytesAndBits size;
    BytesAndBits position_in_report_buffer;
    uint32_t reportId = 0;
    bool isRange = false;
};

struct HID_Parsed_Data{
    //g - Global
    uint32_t g_Usage_Page;
    uint32_t g_Usage;

    std::vector<HID_Usage> input;
    std::vector<HID_Usage> feature;
    std::vector<HID_Usage> output;
};

#pragma pack(push,1)
struct Sony_Slut_Face_With_A_Dick{
    uint16_t timestamp;
    uint8_t batteryLevel;
    int16_t gyroX;
    int16_t gyroY;
    int16_t gyroZ;
    int16_t accelX;
    int16_t accelY;
    int16_t accelZ;
};
#pragma pack(pop)

enum class HID_Commands{
    //Local
    Usage = 0x8,
    Usage_Minimum = 0x18,
    Usage_Maximum = 0x28,
    Designator_Index = 0x38,
    Designator_Minimum = 0x48,
    Designator_Maximum = 0x58,
    String_Index = 0x78,
    String_Minimum = 0x88,
    String_Maximum = 0x98,
    Delimiter = 0xA8,

    //Global
    Usage_Page = 0x4,
    Logical_Minimum = 0x14,
    Logical_Maximum = 0x24,
    Physical_Minimum = 0x34,
    Physical_Maximum = 0x44,
    Unit_Exponent = 0x54,
    Unit = 0x64,
    Report_Size = 0x74,
    Report_Id = 0x84,
    Report_Count = 0x94,
    Push = 0xA4,
    Pop = 0xB4,

    //Main Items
    Input = 0x80,
    Output = 0x90,
    Feature = 0xB0,
    Collection = 0xA0,
    End_Collection = 0xC0,
};


#endif //DEBBUGJSINPUT_HID_RELATED_INFO_H
