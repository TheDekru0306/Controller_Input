#ifndef DEBBUGJSINPUT_VIBRATIONEFFECT_H
#define DEBBUGJSINPUT_VIBRATIONEFFECT_H


#include <cstdint>
#include <linux/input.h>
#include "../Timer.h"

class VibrationEffect {
public:
    VibrationEffect(int controller, uint16_t timeInterval, uint16_t vibrationForce);
    bool Update();
    bool valid = false;

private:
    ff_effect effect;
    input_event play;
    int controller;
    Timer timer;
    bool finished = false;


};


#endif //DEBBUGJSINPUT_VIBRATIONEFFECT_H
