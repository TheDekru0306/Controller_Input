#ifndef DEBBUGJSINPUT_JS_ZERO_H
#define DEBBUGJSINPUT_JS_ZERO_H
#include <linux/joystick.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

struct axis_state {
    short x, y;
};

class JS_ZERO {

public:
    void Update();

private:
    size_t get_axis_count(int fd);
    int read_event(int fd, struct js_event *event);
    size_t get_axis_state(struct js_event *event, struct axis_state axes[3]);

    struct js_event event;
    struct axis_state axes[3] = {0};
    size_t axis;
};


#endif //DEBBUGJSINPUT_JS_ZERO_H
