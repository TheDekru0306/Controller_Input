#ifndef DEBBUGJSINPUT_PS4_HID_H
#define DEBBUGJSINPUT_PS4_HID_H

enum class PS4_HID{
    PS4_LeftAxis_X = 0x30,
    PS4_LeftAxis_Y = 0x31,
    PS4_RightAxis_X = 0x32,
    PS4_RightAxis_Y = 0x35,
    PS4_L2_Axis = 0x33,
    PS4_R2_Axis = 0x34,
    PS4_Hat_Switch = 0x39,

    PS4_Rectangle = 0,
    PS4_X = 1,
    PS4_O = 2,
    PS4_Triangle = 3,
    PS4_L1 = 4,
    PS4_R1 = 5,
    PS4_L2_Button = 6,
    PS4_R2_Button = 7,
    PS4_Share = 8,
    PS4_Options = 9,
    PS4_L3 = 10,
    PS4_R3 = 11,
    PS4_Home = 12,
    PS4_TPAD_Click = 13
};

enum class PS4_HID_Hat_Switch{
    PS4_Arrow_UP = 0,
    PS4_Arrow_Right = 2,
    PS4_Arrow_Down = 4,
    Ps4_Arrow_Left = 6,

    PS4_Arrow_UP_RIGHT = 1,
    PS4_Arrow_UP_Left = 7,

    PS4_Arrow_Down_Right = 3,
    PS4_Arrow_Down_Left = 5
};

#endif //DEBBUGJSINPUT_PS4_HID_H
