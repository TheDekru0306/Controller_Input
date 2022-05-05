#ifndef DEBBUGJSINPUT_DEVICE_DETECTOR_H
#define DEBBUGJSINPUT_DEVICE_DETECTOR_H

#include <libudev.h>

inline std::vector<std::string> Split(std::string s,char d,bool leaveEmpty) {
    std::vector<std::string> elems;
    std::string item;
    std::stringstream ss(s);
    while (std::getline(ss, item, d)) {
        if (!item.empty()) {//== leeaveEmpty
            elems.push_back(item);
        }
    }
    return elems;
}

struct Detected_Device_Info{
    std::string manufacturer = "";
    std::string path = "";
    std::string action = "";
    unsigned short vendor_id;
    unsigned short product_id;
};
struct Hid_Parser{
    int bus_type;
    unsigned short vendor_id;
    unsigned short product_id;
    std::string serial_number_utf8;
    std::string product_name_utf8;
};


class Device_Detector {
public:
    void Initialize(std::string subSytem);
    std::vector<Detected_Device_Info> DEVICE_LIST_HAS_CHANGED();
private:
    udev *udevObject;
    udev_monitor *monitor;
    udev_device *device;

    timeval tv;
    int file_descriptor, ret;
    fd_set fds;
    std::string subSystem;
};


#endif //DEBBUGJSINPUT_DEVICE_DETECTOR_H
