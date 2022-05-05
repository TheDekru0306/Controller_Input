#ifndef DEBBUGJSINPUT_EVENT_BASED_STRUCTS_H
#define DEBBUGJSINPUT_EVENT_BASED_STRUCTS_H
struct Axis{
public:
    int x = 0, y = 0;
};

struct Controller{
    int controller_file_descriptor;
    std::string event;
    Axis R1;
    Axis L1;
    int R2;
    int L2;
};


struct Event_Devices_Info{
    std::string name;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t ev;
    std::string devicePath;
};


template<typename T>
std::string number_to_hex(T i)
{
    std::stringstream stream;
    stream<<std::setfill ('0') << std::setw(sizeof(T)*2)<<std::hex << i;
    return stream.str();
}



#endif //DEBBUGJSINPUT_EVENT_BASED_STRUCTS_H
