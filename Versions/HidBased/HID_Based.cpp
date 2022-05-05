#include "HID_Based.h"
#include "PS4_HID.h"
#include "CRCTable.h"

void HID_Based::Intialize() {
    struct hid_device_info *devs, *cur_dev;
    std::wstring ws_manufacturer;
    std::wstring ws_product;

    std::string prev_manufacturer;
    std::string prev_product;


    devs = hid_enumerate(0x0, 0x0);
    cur_dev = devs;
    while (cur_dev) {
        Device_Info deviceInfo;

        deviceInfo.path = cur_dev->path;
        deviceInfo.productID = cur_dev->product_id;

        devices.insert(std::pair<uint16_t, Device_Info>(cur_dev->vendor_id, deviceInfo));

        ws_manufacturer = cur_dev->manufacturer_string;
        ws_product = cur_dev->product_string;

        std::string manufacturer(ws_manufacturer.begin(), ws_manufacturer.end());
        std::string product(ws_product.begin(), ws_product.end());

        if (prev_manufacturer != manufacturer && prev_product != product) {
            prev_manufacturer = manufacturer;
            prev_product = product;

            std::cout << "Manufacturer: " << manufacturer << "\n";
            std::cout << "Product: " << product << "\n";
        }

        cur_dev = cur_dev->next;
    }

    if (devices.count(static_cast<const unsigned short>(Device_VenderId::SONY)) > 0) {
        std::string devicePath = devices.find(static_cast<const unsigned short>(Device_VenderId::SONY))->second.path;

        this->controller = open(devicePath.c_str(), O_RDWR | O_NONBLOCK);

        if (this->controller < 0) {
            std::cout << "Buy a controller you idiot!!!\n";
        }

        Load_Device_HID_Capabilities();
        VIBRATOR_MOD(5, 255, 0, 255, 0, 0, true);
    }

    if (devices.count(static_cast<const unsigned short>(Device_VenderId::MS)) > 0) {
        std::string devicePath = devices.find(static_cast<const unsigned short>(Device_VenderId::MS))->second.path;

        this->controller = open(devicePath.c_str(), O_RDWR | O_NONBLOCK);

        if (this->controller < 0) {
            std::cout << "Buy a controller you idiot!!!\n";
        }

        Load_Device_HID_Capabilities();
        while (true){}
        int a;
        //VIBRATOR_MOD(5, 255, 0, 255, 0, 0, true);
    }
}

void HID_Based::Update() {

    ControllerUpdate();
}


void HID_Based::ControllerUpdate() {
    int result = hid_read(this->controller, buffer, sizeof(buffer));
    int value;

    for (int i = 0; i < result; i++) {
        printf("%02hhx ", buffer[i]);
    }
    if(result){
        std::cout<<"\n";
        for (int i = 0; i < parsed_data.input.size(); ++i) {
            //std::cout<<buffer
            if(buffer[0] == 0x11)
            {
                value = Extract_Value(parsed_data.input[i], &buffer[2]);
            }
            else if(buffer[0] == 1)
            {
                value = Extract_Value(parsed_data.input[i], buffer);
            }
            switch(parsed_data.input[i].page){
                case HID_USAGE_PAGE_GENERIC:{
                    switch((PS4_HID)parsed_data.input[i].usage_min){
                        case PS4_HID::PS4_LeftAxis_X:{
                            std::cout<<"Left Axis X : "<< value<<"\n";
                            break;
                        }
                        case PS4_HID::PS4_LeftAxis_Y:{
                            std::cout<<"Left Axis Y : "<< value<<"\n";
                            break;
                        }
                        case PS4_HID::PS4_RightAxis_X:{
                            std::cout<<"Right Axis X : "<< value<<"\n";
                            break;
                        }
                        case PS4_HID::PS4_RightAxis_Y:{
                            std::cout<<"Right Axis Y : "<< value<<"\n";
                            break;
                        }
                        case PS4_HID::PS4_L2_Axis:{
                            VIBRATOR_MOD(1, value, value, 255,0,0);
                            std::cout<<"L2 : "<< value<<"\n";
                            break;
                        }
                        case PS4_HID::PS4_R2_Axis:{
                            std::cout<<"R2 : "<< value<<"\n";
                            break;
                        }
                        case PS4_HID::PS4_Hat_Switch:{
                            switch ((PS4_HID_Hat_Switch)value){
                                case PS4_HID_Hat_Switch::PS4_Arrow_UP:{
                                    std::cout<<"Arrow UP : "<< value<<"\n";
                                    break;
                                }
                                case PS4_HID_Hat_Switch::PS4_Arrow_Right:{
                                    std::cout<<"Arrow Right : "<< value<<"\n";
                                    break;
                                }
                                case PS4_HID_Hat_Switch::PS4_Arrow_Down:{
                                    std::cout<<"Arrow Down : "<< value<<"\n";
                                    break;
                                }
                                case PS4_HID_Hat_Switch::Ps4_Arrow_Left:{
                                    std::cout<<"Arrow Left : "<< value<<"\n";
                                    break;
                                }
                                case PS4_HID_Hat_Switch::PS4_Arrow_UP_RIGHT:{
                                    std::cout<<"Arrow UP Right : "<< value<<"\n";
                                    break;
                                }
                                case PS4_HID_Hat_Switch::PS4_Arrow_UP_Left:{
                                    std::cout<<"Arrow UP Left : "<< value<<"\n";
                                    break;
                                }
                                case PS4_HID_Hat_Switch::PS4_Arrow_Down_Right:{
                                    std::cout<<"Arrow Down Right : "<< value<<"\n";
                                    break;
                                }
                                case PS4_HID_Hat_Switch::PS4_Arrow_Down_Left:{
                                    std::cout<<"Arrow Down Left : "<< value<<"\n";
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
                case HID_USAGE_PAGE_BUTTON:{
                    for(int index = 0; index < 14; index++){
                        switch((PS4_HID)index){
                            case PS4_HID::PS4_Rectangle:{
                                if(getState(value, index)){
                                    std::cout<<"◻ \n";
                                }
                                break;
                            }
                            case PS4_HID::PS4_X:{
                                if(getState(value, index)){
                                    std::cout<<"X \n";
                                }
                                break;
                            }
                            case PS4_HID::PS4_O:{
                                if(getState(value, index)){
                                    std::cout<<"O \n";
                                }
                                break;
                            }
                            case PS4_HID::PS4_Triangle:{
                                if(getState(value, index)){
                                    std::cout<<"△ \n";
                                }
                                break;
                            }
                            case PS4_HID::PS4_L1:{
                                if(getState(value, index)){
                                    std::cout<<"L1 \n";
                                }
                                break;
                            }
                            case PS4_HID::PS4_R1:{
                                if(getState(value, index)){
                                    std::cout<<"R1 \n";
                                }
                                break;
                            }
                            case PS4_HID::PS4_L2_Button:{
                                if(getState(value, index)){
                                    std::cout<<"L2 \n";
                                }
                                break;
                            }
                            case PS4_HID::PS4_R2_Button:{
                                if(getState(value, index)){
                                    std::cout<<"R2 \n";
                                }
                                break;
                            }
                            case PS4_HID::PS4_Share:{
                                if(getState(value, index)){
                                    std::cout<<"Share \n";
                                }
                                break;
                            }
                            case PS4_HID::PS4_Options:{
                                if(getState(value, index)){
                                    std::cout<<"Options \n";
                                }
                                break;
                            }
                            case PS4_HID::PS4_L3:{
                                if(getState(value, index)){
                                    std::cout<<"L3 \n";
                                }
                                break;
                            }
                            case PS4_HID::PS4_R3:{
                                if(getState(value, index)){
                                    std::cout<<"R3 \n";
                                }
                                break;
                            }
                            case PS4_HID::PS4_Home:{
                                if(getState(value, index)){
                                    std::cout<<"Home \n";
                                }
                                break;
                            }
                            case PS4_HID::PS4_TPAD_Click:{
                                if(getState(value, index)){
                                    std::cout<<"Touch Pad Click \n";
                                }
                                break;
                            }
                        }
                    }
                    break;
                }
                case 65280:{
                    if(parsed_data.input[i].usage_min == 33){
                        Sony_Slut_Face_With_A_Dick dickSlut = *((Sony_Slut_Face_With_A_Dick*)(&buffer[parsed_data.input[i].position_in_report_buffer.bytes]));

                        std::cout<<"\n\n";
                        std::cout<<"Battery: "<<(int)dickSlut.batteryLevel<<"\n";

                        std::cout<<"Gyro X: "<<dickSlut.gyroX<<"\n";
                        std::cout<<"Gyro Y: "<<dickSlut.gyroY<<"\n";
                        std::cout<<"Gyro Z: "<<dickSlut.gyroZ<<"\n";

                        std::cout<<"Accel X: "<<dickSlut.accelX<<"\n";
                        std::cout<<"Accel Y: "<<dickSlut.accelY<<"\n";
                        std::cout<<"Accel Z: "<<dickSlut.accelZ<<"\n";
                    }
                }
            }
        }

        usleep(10000);
        std::system("clear");

        /*puts("\n");*/
    }
}


uint32_t HID_Based::getValue(const hidraw_report_descriptor &reportDescription, int counter, uint8_t &bytesToRead) {
    if(bytesToRead == 1){
        return report_Description.value[counter + 1];
    }
    else if(bytesToRead == 2){
        return report_Description.value[counter + 2] << 8 | report_Description.value[counter + 1];
    }
    else if(bytesToRead == 3){
        bytesToRead = 4;
        return report_Description.value[counter + 4] << 8 | report_Description.value[counter + 3]<< 8 | report_Description.value[counter + 2]<< 8 | report_Description.value[counter + 1];
    }
    return 0;
}

void HID_Based::Load_Device_HID_Capabilities() {
    int descriptionSize;
    int response = ioctl(this->controller, HIDIOCGRDESCSIZE, &descriptionSize);

    if (response < 0)
        std::cout<<"Program says fuck you :D!\n";
    else
        printf("Report Descriptor Size: %d\n", descriptionSize);

    report_Description.size = descriptionSize;
    response = ioctl(this->controller, HIDIOCGRDESC, &report_Description);
    if (response < 0) {
        std::cout<<"Device Did Not Loaded!\n";
    } else {
        printf("Report Descriptor:\n");
        for (int i = 0; i < report_Description.size; i++) {
            printf("%hhx ", report_Description.value[i]);
        }
        puts("\n");
        //while(true){}
        std::vector<HID_Usage> temps;

        uint32_t usagePage = 0;
        uint32_t logicalMinimum = 0;
        uint32_t logicalMaximum = 0;
        uint32_t physicalMinimum = 0;
        uint32_t physicalMaximum = 0;
        uint32_t unitExponent = 0;
        uint32_t unit = 0;
        uint32_t reportSize = 0;
        uint32_t reportId = 0;
        uint32_t reportCount = 0;

        BytesAndBits counter;

        bool hasReceivedUsagePage = false;
        bool hasReceivedUsage = false;
        bool reachedEndCollection = false;

        for (int i = 0; i < report_Description.size && !reachedEndCollection; i++) {
            uint8_t value = report_Description.value[i] & 0x3;
            uint8_t dataType = report_Description.value[i] & 0xfc;

            switch ((HID_Commands)dataType) {
                case HID_Commands::Feature:{
                    uint32_t result = getValue(report_Description, i, value);
                    BytesAndBits localCounter = counter;

                    for(auto temp : temps){
                        temp.page = usagePage;
                        temp.logical_min = logicalMinimum;
                        temp.logical_max = logicalMaximum;
                        temp.physical_min = physicalMinimum;
                        temp.physical_max = physicalMaximum;
                        temp.unit_exponent = unitExponent;
                        temp.unit = unit;
                        temp.reportId = reportId;
                        temp.size = getBytesAndBits(reportSize);
                        temp.position_in_report_buffer = localCounter;

                        if(temp.usage_min != temp.usage_max){
                            temp.size = getBytesAndBits(reportSize * reportCount);
                            localCounter += getBytesAndBits(reportSize * reportCount);
                        }
                        else{
                            localCounter += getBytesAndBits(reportSize);
                        }
                        if(temps.size() == 1){
                            temp.size = getBytesAndBits(reportSize * reportCount);
                        }
                        /*temp.re = reportCount;*/
                        parsed_data.feature.push_back(temp);
                    }
                    counter += getBytesAndBits(reportCount * reportSize);
                    temps.clear();
                    i += value;
                    break;
                }
                case HID_Commands::Input:{
                    uint32_t result = getValue(report_Description, i, value);
                    BytesAndBits localCounter = counter;

                    for(auto temp : temps){
                        temp.page = usagePage;
                        temp.logical_min = logicalMinimum;
                        temp.logical_max = logicalMaximum;
                        temp.physical_min = physicalMinimum;
                        temp.physical_max = physicalMaximum;
                        temp.unit_exponent = unitExponent;
                        temp.unit = unit;
                        temp.reportId = reportId;
                        temp.size = getBytesAndBits(reportSize);
                        temp.position_in_report_buffer = localCounter;

                        if(temp.usage_min != temp.usage_max){
                            temp.size = getBytesAndBits(reportSize * reportCount);
                            localCounter += getBytesAndBits(reportSize * reportCount);
                        }
                        else{
                            localCounter += getBytesAndBits(reportSize);
                        }
                        /*temp.re = reportCount;*/
                        if(temps.size() == 1){
                            temp.size = getBytesAndBits(reportSize * reportCount);
                        }
                        parsed_data.input.push_back(temp);
                    }
                    counter += getBytesAndBits(reportCount * reportSize);
                    temps.clear();

                    i += value;
                    break;
                }
                case HID_Commands::Output:{
                    uint32_t result = getValue(report_Description, i, value);
                    BytesAndBits localCounter = counter;

                    for(auto temp : temps){
                        temp.page = usagePage;
                        temp.logical_min = logicalMinimum;
                        temp.logical_max = logicalMaximum;
                        temp.physical_min = physicalMinimum;
                        temp.physical_max = physicalMaximum;
                        temp.unit_exponent = unitExponent;
                        temp.unit = unit;
                        temp.reportId = reportId;
                        temp.size = getBytesAndBits(reportSize);
                        temp.position_in_report_buffer = localCounter;

                        if(temp.usage_min != temp.usage_max){
                            temp.size = getBytesAndBits(reportSize * reportCount);
                            localCounter += getBytesAndBits(reportSize * reportCount);
                        }
                        else{
                            localCounter += getBytesAndBits(reportSize);
                        }
                        if(temps.size() == 1){
                            temp.size = getBytesAndBits(reportSize * reportCount);
                        }
                        /*temp.re = reportCount;*/
                        parsed_data.output.push_back(temp);
                    }
                    counter += getBytesAndBits(reportCount * reportSize);
                    temps.clear();

                    i += value;
                    break;
                }
                case HID_Commands::Collection:{
                    uint32_t result = getValue(report_Description, i, value);
                    i += value;
                    break;
                }
                case HID_Commands::Report_Count:{
                    uint32_t result = getValue(report_Description, i, value);
                    reportCount = result;
                    i += value;
                    break;
                }
                case HID_Commands::Usage_Page:{
                    uint32_t result = getValue(report_Description, i, value);

                    if(!hasReceivedUsagePage){
                        parsed_data.g_Usage_Page = result;
                        hasReceivedUsagePage = true;
                    }

                    usagePage = result;

                    i += value;
                    break;
                }
                case HID_Commands::Usage:{
                    uint32_t result = getValue(report_Description, i, value);

                    if(!hasReceivedUsage){
                        parsed_data.g_Usage = result;
                        hasReceivedUsage = true;
                    }
                    else{
                        HID_Usage newTemp;

                        newTemp.usage_min = result;
                        newTemp.usage_max = result;

                        temps.push_back(newTemp);
                    }

                    i += value;
                    break;
                }
                case HID_Commands::Usage_Minimum:{
                    uint32_t result = getValue(report_Description, i, value);

                    HID_Usage newTemp;
                    newTemp.usage_min = result;
                    temps.push_back(newTemp);

                    i += value;
                    break;
                }
                case HID_Commands::Usage_Maximum:{
                    uint32_t result = getValue(report_Description, i, value);
                    temps.back().usage_max = result;
                    i += value;
                    break;
                }
                case HID_Commands::Logical_Minimum:{
                    uint32_t result = getValue(report_Description, i, value);
                    logicalMinimum = result;
                    i += value;
                    break;
                }
                case HID_Commands::Logical_Maximum:{
                    uint32_t result = getValue(report_Description, i, value);
                    logicalMaximum = result;
                    i += value;
                    break;
                }
                case HID_Commands::Report_Size:{
                    uint32_t result = getValue(report_Description, i, value);
                    reportSize = result;
                    i += value;
                    break;
                }
                case HID_Commands::Report_Id:{
                    uint32_t result = getValue(report_Description, i, value);
                    counter.bits = 0;
                    counter.bytes = 1;
                    reportId = result;
                    i += value;
                    break;
                }
                case HID_Commands::Physical_Maximum:{
                    uint32_t result = getValue(report_Description, i, value);
                    physicalMaximum = result;
                    i += value;
                    break;
                }
                case HID_Commands::Physical_Minimum:{
                    uint32_t result = getValue(report_Description, i, value);
                    physicalMinimum = result;
                    i += value;
                    break;
                }
                case HID_Commands::Unit_Exponent:{
                    uint32_t result = getValue(report_Description, i, value);
                    unitExponent = result;
                    i += value;
                    break;
                }
                case HID_Commands::Unit:{
                    uint32_t result = getValue(report_Description, i, value);
                    unit = result;
                    i += value;
                    break;
                }
                case HID_Commands::Designator_Index:{
                    uint32_t result = getValue(report_Description, i, value);
                    i += value;
                    break;
                }
                case HID_Commands::Designator_Minimum:{
                    uint32_t result = getValue(report_Description, i, value);
                    i += value;
                    break;
                }
                case HID_Commands::Designator_Maximum:{
                    uint32_t result = getValue(report_Description, i, value);
                    i += value;
                    break;
                }
                case HID_Commands::String_Index:{
                    uint32_t result = getValue(report_Description, i, value);
                    i += value;
                    break;
                }
                case HID_Commands::String_Minimum:{
                    uint32_t result = getValue(report_Description, i, value);
                    i += value;
                    break;
                }
                case HID_Commands::String_Maximum:{
                    uint32_t result = getValue(report_Description, i, value);
                    i += value;
                    break;
                }
                case HID_Commands::Delimiter:{
                    uint32_t result = getValue(report_Description, i, value);
                    i += value;
                    break;
                }
                case HID_Commands::End_Collection:{
                    uint32_t result = getValue(report_Description, i, value);
                    //reachedEndCollection = true;
                    i += value;
                    break;
                }
            }
        }
    }
}

BytesAndBits HID_Based::getBytesAndBits(const uint32_t input) {
    BytesAndBits result;

    result.bits = input % 8;
    result.bytes = input / 8;

    return result;
}

void HID_Based::VIBRATOR_MOD(int timeInterval, int vibrationForceLeft, int vibrationForceRight, int r, int g, int b, bool bluetooth) {

    memset(buffer, 0, sizeof(buffer));
    if(!bluetooth) {
        buffer[0] = 0x05;
        buffer[1] = 0xFF;
        buffer[4] = vibrationForceRight; // 0-255 // Vibrator
        buffer[5] = vibrationForceLeft; // 0-255 // Vibrator
        buffer[6] = r; // 0-255
        buffer[7] = g;
        buffer[8] = b;
    }
    else{
        buffer[0] = 0x11;
        buffer[3] = 0xF3;
        buffer[6] = vibrationForceRight; // 0-255 // Vibrator
        buffer[7] = vibrationForceLeft; // 0-255 // Vibrator
        buffer[8] = r; // 0-255
        buffer[9] = g;
        buffer[10] = b;
        buffer[74] = CRC(buffer, 74);//CRC bitch
    }

    //while(timeInterval > 0){
        hid_write(this->controller, buffer, 79);
       // sleep(1);
        //timeInterval--;
   // }
   /* buffer[4] = 0;
    buffer[5] = 0;
    hid_write(this->controller, buffer, 32);*/
}

uint32_t HID_Based::Extract_Value(HID_Usage info, unsigned char *buffer) {
    uint32_t value = 0;
    HID_Usage count = info;
    BytesAndBits toRemove;
    int mydick = 0;

    if(count.position_in_report_buffer.bits != 0){
        toRemove.bits = count.position_in_report_buffer.bits;
        uint8_t mask = ~((1 << count.position_in_report_buffer.bits) - 1);
        value = (buffer[count.position_in_report_buffer.bytes] & mask) >> count.position_in_report_buffer.bits;
        (void)value;
        count.size = count.size - toRemove;
        toRemove.bytes++;
    }

    while(count.size.bytes){

        value |= buffer[count.position_in_report_buffer.bytes + toRemove.bytes] << toRemove.bits;
        toRemove.bytes++;
        count.size.bytes--;
        (void)value;
        mydick += 2;
    }

    if(count.size.bits){
        uint8_t mask = ((1 << count.size.bits) - 1);
        auto fuckYou = info.size - count.size;
        value |= (buffer[count.position_in_report_buffer.bytes + toRemove.bytes] & mask) << fuckYou.toBits();
        (void)value;
        mydick += 2;
    }

    return value;
}

bool HID_Based::getState(uint32_t buttons, uint8_t index) {
    return buttons & (1 << index);
}

uint32_t HID_Based::CRC(unsigned char *buffer, int buffer_size) {

    uint32_t crc = 0xFFFFFFFF;

    for (int i = 0; i < buffer_size; ++i) {
        auto index = (crc ^ buffer[i]) & 0xff;
        crc = BZ2_crc32Table[index] ^ (crc >> 8);
    }

    return crc ^ 0xFFFFFFFF;
}
