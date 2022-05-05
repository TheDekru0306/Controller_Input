#include <iostream>
#include <vector>
#include <sstream>
#include "Device_Detector.h"


void Device_Detector::Initialize(std::string subSystem) {
    udevObject = udev_new();
    this->subSystem = subSystem;
    if (!udevObject) {
        std::cout<<"Can't create udev\n";
    }

    monitor = udev_monitor_new_from_netlink(udevObject, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(monitor, this->subSystem.c_str(), nullptr);
    udev_monitor_enable_receiving(monitor);
    file_descriptor = udev_monitor_get_fd(monitor);
}

static Hid_Parser parse_uevent_info(const char *uevent, bool isHID)
{
    Hid_Parser parse{0};
    std::stringstream hid(uevent);
    std::string line;
    while (std::getline(hid, line)){
        auto key_value = Split(line, '=', false);

        if((key_value[0] == "HID_NAME")){
            parse.product_name_utf8 = key_value[1];
        }
        if( key_value[0] == "NAME"){
           auto hasName = Split(key_value[1], '\"', false);
           if(!hasName.empty()){
               parse.product_name_utf8 = hasName[0];
           }
        }

        if(key_value[0] == "MODALIAS"){
            if(isHID){
                auto found = key_value[1].find('v') ;
                if(found != std::string::npos){
                    parse.vendor_id = std::stoi(key_value[1].substr(found + 1, 8), nullptr, 16);
                }

                auto modalias_split = Split(line, ':', false);

                found = modalias_split[1].find('p');
                if(found != std::string::npos){
                    parse.product_id =  std::stoi(modalias_split[1].substr(found + 1, 8), nullptr, 16);
                }
            }
            else{
                auto found = key_value[1].find('v');
                if(found != std::string::npos){
                    parse.vendor_id =  std::stoi(key_value[1].substr(found + 1, 4), nullptr, 16);
                }

                auto modalias_split = Split(line, ':', false);

                found = modalias_split[1].find('p');
                if(found != std::string::npos){
                    parse.product_id =  std::stoi(modalias_split[1].substr(found + 1, 4), nullptr, 16);
                }
            }
        }
    }
    return parse;
}


std::vector<Detected_Device_Info> Device_Detector::DEVICE_LIST_HAS_CHANGED() {
    file_descriptor = udev_monitor_get_fd(monitor);
    Detected_Device_Info detectedDevice;
    std::vector<Detected_Device_Info> collection;
    ret = 1;
    while (ret) {

        FD_ZERO(&fds);
        FD_SET(file_descriptor, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(file_descriptor+1, &fds, nullptr, nullptr, &tv);
        if (ret > 0 && FD_ISSET(file_descriptor, &fds)) {
            device = udev_monitor_receive_device(monitor);
            udev_device* parent = nullptr;
            if (device) {
                if(subSystem == "hidraw"){
                    parent = udev_device_get_parent_with_subsystem_devtype(device, "hid", nullptr);

                }else if(subSystem == "input"){
                    parent = udev_device_get_parent_with_subsystem_devtype(device, subSystem.c_str(), nullptr);
                }

                if(parent){
                    detectedDevice.manufacturer =  udev_device_get_sysattr_value(parent, "uevent");
                }
                Hid_Parser parser{0};

                parser = parse_uevent_info(detectedDevice.manufacturer.c_str(), subSystem == "hidraw");

                detectedDevice.manufacturer = parser.product_name_utf8;
                detectedDevice.vendor_id = parser.vendor_id;
                detectedDevice.product_id = parser.product_id;
                detectedDevice.action = udev_device_get_action(device);
                detectedDevice.path = udev_device_get_sysname(device);

                collection.push_back(detectedDevice);
                udev_device_unref(device);
            }
        }
    }

    return collection;
}

