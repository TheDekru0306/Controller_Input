#include <regex>
#include <algorithm>
#include <list>

#include "Event_Based.h"
#include "XBOX_CODES.h"
#include "PS4_CODES.h"

int Event_Based::ReadEvent(int controller, struct input_event* event) {
    size_t bytes;
    bytes = read(controller, event, sizeof(*event));

    if (bytes == sizeof(*event))
        return 0;

    return -1;
}

void Event_Based::Initialize() {
    InitializeControllers();
    deviceDetector.Initialize("input");
}

void Event_Based::InitializeControllers() {
    GetDevicesPath();

    std::vector<std::string> foundControllers;

    std::vector<std::list<int>::const_iterator> matches;
    for (auto & device : devices){
        if (device.name == "Sony Interactive Entertainment Wireless Controller"){
            foundControllers.push_back(device.devicePath);
        }
    }

    for (const auto& controllerPath : foundControllers) {
        int controller = open(controllerPath.c_str(), O_RDWR | O_NONBLOCK);

        if(controller != -1){
            Controller device = {0};
            device.event = controllerPath;
            device.controller_file_descriptor = controller;

            controllers.insert(std::pair<int, Controller>(controller, device));
        }
        else {
            std::cout<<"\nNo permissions for \"Controller Slut\" with path("<<controllerPath<<").\nAlso Controller is gay.\n";
        }
    }

    this->touchpad = open("/dev/input/event13", O_RDONLY);
    this->motionSensor = open("/dev/input/event20", O_RDONLY);

}

void Event_Based::Update() {
    VerifyOtherDevices();

    UpdateControllerList();

    ControllerDevicesUpdate();

}

void Event_Based::UpdateControllerList() {

    collection = deviceDetector.DEVICE_LIST_HAS_CHANGED();
    if(collection.empty()){
        return;
    }

    int controllerMatch = 0;

    for (auto& device : collection) {
        if(device.action == "add" && device.manufacturer == "Sony Interactive Entertainment Wireless Controller"){

            for (auto& controller : controllers) {
                if(controller.second.event == device.path){
                    controllerMatch = controller.first;
                    break;
                }
            }

            if(!controllerMatch && device.path.find("event") != std::string::npos){
                int fb = open(device.path.c_str(), O_RDWR | O_NONBLOCK);
                Controller controller = {0};
                controller.event = device.path;
                controller.controller_file_descriptor = fb;

                controllers.insert(std::pair<int, Controller>(fb, controller));
                std::cerr<<"Some Gay Controller Was Just Added Don't Buy A New Controller You Dumb Idiot. { path = "<<controllers[fb].event<<" }"<<"\n";
            }
        }
        else if(device.action == "remove" && device.manufacturer == "Sony Interactive Entertainment Wireless Controller"){
            for (auto& controller : controllers) {
                if(controller.second.event == device.path){
                    controllerMatch = controller.first;
                    break;
                }
            }
            if(controllerMatch){
                std::cerr<<"Some Gay Controller Just Died Buy A New Controller You Dumb Idiot. { path = "<<controllers[controllerMatch].event<<" }"<<"\n";
                close(controllerMatch);
                controllers.erase(controllerMatch);
            }
        }
        controllerMatch = 0;
    }
}


#pragma region Done
void Event_Based::ControllerDevicesUpdate() {
    for (auto controller : this->controllers) {
        while(ReadEvent(controller.first, &this->event) == 0){
            if(this->event.type == EV_SYN && this->event.code == SYN_REPORT) {
                //std::cout<<"Syncing controller and buffer.\n";
                fsync(controller.first);
            }
            switch(this->event.type){
                case EV_KEY:
                case EV_ABS: {
                    //ReadXBOX360Input(this->event.code, this->event.value, controller);
                    ReadPS4Input(this->event.code, this->event.value, controller.first);
                    break;
                }
            }
            if(controllers[controller.first].R2 == 255){
                VIBRATOR_MOD(controller.first, 100,0x7fff);
            }
            else{
                VIBRATOR_MOD(controller.first, 100,controllers[controller.first].R2 * 128);
            }
        }
    }

    for (auto& effect : vibrationRequests) {
        if(!effect.second.empty()){
            if(effect.second.front().Update()){
                effect.second.pop();
            }
        }
    }
}

int Event_Based::ReadPS4Input(uint16_t input, int value, int controller) {
    switch((PS4_CODES)input){
        case PS4_CODES::PS4_L1:{
            std::cout<<"L1 ";
            std::cout<<"Controller With Value: "<<controller<<"\n";
            break;
        }
        case PS4_CODES::PS4_R1:{
            std::cout<<"R1 ";
            std::cout<<"Controller With Value: "<<controller<<"\n";
            break;
        }
        case PS4_CODES::PS4_X:{
            std::cout<<"X ";
            std::cout<<"Controller With Value: "<<controller<<"\n";
            break;
        }
        case PS4_CODES::PS4_O:{
            std::cout<<"O ";
            std::cout<<"Controller With Value: "<<controller<<"\n";
            break;
        }
        case PS4_CODES::PS4_Triangle:{
            std::cout<<"Triangle ";
            std::cout<<"Controller With Value: "<<controller<<"\n";
            break;
        }
        case PS4_CODES::PS4_Rectangle:{
            std::cout<<"Rectangle ";
            std::cout<<"Controller With Value: "<<controller<<"\n";
            break;
        }
        case PS4_CODES::PS4_Home:{
            std::cout<<"Home ";
            std::cout<<"Controller With Value: "<<controller<<"\n";
            break;
        }
        case PS4_CODES::PS4_Options:{
            std::cout<<"Options ";
            std::cout<<"Controller With Value: "<<controller<<"\n";
            break;
        }
        case PS4_CODES::PS4_Share:{
            std::cout<<"Share ";
            std::cout<<"Controller With Value: "<<controller<<"\n";
            break;
        }
        case PS4_CODES::PS4_R3:{
            std::cout<<"R3 ";
            std::cout<<"Controller With Value: "<<controller<<"\n";
            break;
        }
        case PS4_CODES::PS4_L3:{
            std::cout<<"L3 ";
            std::cout<<"Controller With Value: "<<controller<<"\n";
            break;
        }
        case PS4_CODES::PS4_R2_Button:{
            std::cout<<"R2: "<<value<<" ";
            std::cout<<"Controller With Value: "<<controller<<"\n";
            break;
        }
        case PS4_CODES::PS4_L2_Button:{
            std::cout<<"L2: "<<value<<" ";
            std::cout<<"Controller With Value: "<<controller<<"\n";
            break;
        }
        case PS4_CODES::PS4_R1_UD: {
            if (value <= 20 || value >= 150) {
                controllers[controller].R1.y = value;

                std::cout << "PS4_R1: { " << controllers[controller].R1.x << ", " << value << " } ";
                std::cout<<"Controller With Value: "<<controller<<"\n";
            }
            break;
        }
        case PS4_CODES::PS4_R1_LR: {
            if (value <= 20 || value >= 150) {
                controllers[controller].R1.x = value;

                std::cout << "PS4_R1: { " << value << ", " <<  controllers[controller].R1.y << " } ";
                std::cout<<"Controller With Value: "<<controller<<"\n";
            }
            break;
        }
        case PS4_CODES::PS4_L1_LR: {
            if (value <= 20 || value >= 150) {
                controllers[controller].L1.x = value;

                std::cout << "PS4_L1: { " <<  value << ", " << controllers[controller].L1.y << " } ";
                std::cout<<"Controller With Value: "<<controller<<"\n";
            }
            break;
        }
        case PS4_CODES::PS4_L1_UD: {
            if (value <= 20 || value >= 150) {
                controllers[controller].L1.y = value;
                std::cout << "PS4_L1: { " << controllers[controller].L1.x << ", " << value << " } ";
                std::cout<<"Controller With Value: "<<controller<<"\n";
            }
            break;
        }
        case PS4_CODES::PS4_ARROW_UD: {
            if (value == 1) {
                std::cout << "PS4_Arrow: Down" << "\n";
                std::cout<<"Controller With Value: "<<controller<<"\n";
            } else if (value == -1) {
                std::cout << "PS4_Arrow: UP" << "\n";
                std::cout<<"Controller With Value: "<<controller<<"\n";
            }
            break;
        }
        case PS4_CODES::PS4_ARROW_LR: {
            if (value == 1) {
                std::cout << "PS4_Arrow: Right" << "\n";
                std::cout<<"Controller With Value: "<<controller<<"\n";
            } else if (value == -1) {
                std::cout << "PS4_Arrow: Left" << "\n";
                std::cout<<"Controller With Value: "<<controller<<"\n";
            }
            break;
        }
        case PS4_CODES::PS4_R2:{
            controllers[controller].R2 = value;
            std::cout<<"R2: "<<value<<"\n";
            std::cout<<"Controller With Value: "<<controller<<"\n";
            break;
        }
        case PS4_CODES::PS4_L2:{
            controllers[controller].L2 = value;
            std::cout<<"L2: "<<value<<"\n";
            std::cout<<"Controller With Value: "<<controller<<"\n";
            break;
        }
    }
    return 0;
}

int Event_Based::ReadXBOX360Input(uint16_t input, int value, int controller) {
    switch((XBOX_CODES)input){
        case XBOX_CODES::XBOX_L1:{
            std::cout<<"L1: "<<value<<"\n";
            break;
        }
        case XBOX_CODES::XBOX_R1:{
            std::cout<<"R1: "<<value<<"\n";
            break;
        }
        case XBOX_CODES::XBOX_X:{
            std::cout<<"X: "<<value<<"\n";
            break;
        }
        case XBOX_CODES::XBOX_Y:{
            std::cout<<"Y: "<<value<<"\n";
            break;
        }
        case XBOX_CODES::XBOX_B:{
            std::cout<<"B: "<<value<<"\n";
            break;
        }
        case XBOX_CODES::XBOX_A:{
            std::cout<<"A: "<<value<<"\n";
            break;
        }
        case XBOX_CODES::XBOX_Home:{
            std::cout<<"Home: "<<value<<"\n";
            break;
        }
        case XBOX_CODES::XBOX_Start:{
            std::cout<<"Start: "<<value<<"\n";
            break;
        }
        case XBOX_CODES::XBOX_Back:{
            std::cout<<"Back: "<<value<<"\n";
            break;
        }
        case XBOX_CODES::XBOX_R3:{
            std::cout<<"R3: "<<value<<"\n";
            break;
        }
        case XBOX_CODES::XBOX_L3:{
            std::cout<<"L3: "<<value<<"\n";
            break;
        }
        case XBOX_CODES::XBOX_R2_Button:{
            std::cout<<"R2: "<<value<<"\n";
            break;
        }
        case XBOX_CODES::XBOX_L2_Button:{
            std::cout<<"L2: "<<value<<"\n";
            break;
        }
        case XBOX_CODES::XBOX_R1_UD: {
            controllers[controller].R1.y = value;
            std::cout << "PS4_R1: { " << controllers[controller].R1.x << ", " << value << " }\n";
            break;
        }
        case XBOX_CODES::XBOX_R1_LR: {
            controllers[controller].R1.x = value;

            std::cout << "XBOX_R1: { " << value << ", " << controllers[controller].R1.y << " }\n";
            break;
        }
        case XBOX_CODES::XBOX_L1_LR: {
            controllers[controller].L1.x = value;

            std::cout << "XBOX_L1: { " << value << ", " << controllers[controller].L1.y << " }\n";
            break;
        }
        case XBOX_CODES::XBOX_L1_UD: {
            controllers[controller].L1.y = value;
            std::cout << "PS4_L1: { " << controllers[controller].L1.x << ", " << value << " }\n";
            break;
        }
        case XBOX_CODES::XBOX_ARROW_UD: {
            if (value == 1) {
                std::cout << "XBOX_Arrow: Down" << "\n";
            } else if (value == -1) {
                std::cout << "XBOX_Arrow: UP" << "\n";
            }
            break;
        }
        case XBOX_CODES::XBOX_ARROW_LR: {
            if (value == 1) {
                std::cout << "XBOX_Arrow: Right" << "\n";
            } else if (value == -1) {
                std::cout << "XBOX_Arrow: Left" << "\n";
            }
            break;
        }
        case XBOX_CODES::XBOX_R2:{
            std::cout<<"R2: "<<value<<"\n";
            break;
        }
        case XBOX_CODES::XBOX_L2:{
            std::cout<<"L2: "<<value<<"\n";
            break;
        }
    }
    return 0;
}

void Event_Based::GetDevicesPath(){
    std::ifstream file_input;
    std::string current_line;
    std::string device_list_file = "/proc/bus/input/devices";

    std::regex base_regex("((Vendor)=(.{4}))|((Product)=(.{4}))|((Name)=\\\"(.*?)\\\")|((event)([0-9]*))|((EV)=(.+))", std::regex_constants::extended);
    std::smatch base_match;
    this->devices.clear();

    file_input.open(device_list_file.c_str());
    if (!file_input.is_open())
    {
        std::cerr << "file_input.open >> " << std::strerror(errno) << std::endl;
        throw -2;
    }
    Event_Devices_Info info;
    while (getline(file_input, current_line))
    {
        std::string current_str = current_line;
        while (std::regex_search(current_str, base_match, base_regex))
        {
            for (int i = 1; i < base_match.size(); ++i)
            {
                if (!base_match[i].str().empty())
                {
                    if(base_match[i+1] == "Vendor")
                    {
                        info.vendor_id = std::stoi(base_match[i+2], 0, 16);
                    }
                    else if(base_match[i+1] == "Product")
                    {
                        info.product_id = std::stoi(base_match[i+2], 0, 16);
                    }
                    else if(base_match[i+1] == "Name")
                    {
                        info.name = base_match[i + 2];
                    }
                    else if(base_match[i+1] == "EV")
                    {
                        info.ev = std::stoi(base_match[i+2], 0, 16);
                    }
                    else if(base_match[i+1] == "event")
                    {
                        info.devicePath = std::string("/dev/input/") + std::string(base_match[i]);
                    }
                    i+=2;
                }
            }
            current_str = base_match.suffix();
        }
        if(current_line.empty()){
            this->devices.push_back(info);
        }
    }

}

void Event_Based::VIBRATOR_MOD(int controller, uint16_t timeInterval, uint16_t vibrationForce, int r, int g, int b) {
    if(vibrationRequests[controller].size() <= requestedEffectsSize_Max){
        vibrationRequests[controller].emplace(controller, timeInterval, vibrationForce);
    }
}

void Event_Based::VerifyOtherDevices() {
    if(touchpad != -1){
        /* while(ReadEvent(this->touchpad, &this->event) == 0){
           switch(this->event.type){
               case EV_ABS:{
                   std::cout<<"Event Code: "<<this->event.code<<"\n";
                   std::cout<<"Event Value: "<<this->event.value<<"\n";
                   break;
               }
           }
       }*/
    }
    if(motionSensor != -1){
        /* while(ReadEvent(this->motionSencor, &this->event) == 0){
           switch(this->event.type){
               case EV_ABS:{
                   std::cout<<"Event Code: "<<this->event.code<<"\n";
                   std::cout<<"Event Value: "<<this->event.value<<"\n";
                   break;
               }
           }
       }*/
    }
}

#pragma endregion


