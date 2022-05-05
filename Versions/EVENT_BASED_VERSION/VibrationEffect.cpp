#include <cstdio>
#include <unistd.h>
#include <cstring>
#include "VibrationEffect.h"

VibrationEffect::VibrationEffect(int controller, uint16_t timeInterval, uint16_t vibrationForce) {
    this->controller = controller;
    effect.type = FF_PERIODIC;
    effect.id = -1;
    effect.u.periodic.waveform = FF_SQUARE;
    effect.u.periodic.period = 100;
    effect.u.periodic.magnitude = vibrationForce;
    effect.u.periodic.offset = 0;
    effect.u.periodic.phase = 0;
    effect.direction = 0x4000;
    effect.u.periodic.envelope.attack_length = 1000;
    effect.u.periodic.envelope.attack_level = vibrationForce;
    effect.u.periodic.envelope.fade_length = 1000;
    effect.u.periodic.envelope.fade_level = vibrationForce;
    effect.trigger.button = 0;
    effect.trigger.interval = 0;
    effect.replay.length = 1;
    effect.replay.delay = 0;

    if (ioctl(this->controller, EVIOCSFF, &effect) == -1) {
        perror("Error:");
    }

    timer.Start(timeInterval, [&](){
        memset(&play, 0, sizeof(play));
        play.type = EV_FF;
        play.code = effect.id;
        play.value = 1;

        if (write(this->controller, (const void *) &play, sizeof(play)) == -1) {
        }

    });
    timer.onFinish([&](){
        ioctl(this->controller, EVIOCRMFF, effect.id);
        finished = true;
    });
}

bool VibrationEffect::Update() {
    timer.Update();
    return finished;
}
