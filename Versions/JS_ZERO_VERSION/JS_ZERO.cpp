#include "JS_ZERO.h"
size_t JS_ZERO::get_axis_count(int fd)
{
    __u8 axes;

    if (ioctl(fd, JSIOCGAXES, &axes) == -1)
        return 0;

    return axes;
}
int JS_ZERO::read_event(int fd, struct js_event *event)
{
    size_t bytes;

    bytes = read(fd, event, sizeof(*event));

    if (bytes == sizeof(*event))
        return 0;

    return -1;
}
size_t JS_ZERO::get_axis_state(struct js_event *event, struct axis_state axes[3])
{
    size_t axis = event->number / 2;
    printf("Currnt Axis: %d     %d\n", event->number, event->value);
    if (axis < 3)
    {
        if (event->number % 2 == 0)
            axes[axis].x = event->value;
        else
            axes[axis].y = event->value;
    }

    return axis;
}

void JS_ZERO::Update() {

    auto js = open("/dev/input/js0", O_RDONLY);

    if (js == -1)
        perror("Could not open joystick");

    printf("%d",get_axis_count(js));

    while (read_event(js, &event) == 0)
    {
        switch (event.type)
        {
            case JS_EVENT_BUTTON:
                printf("Button %u %s\n", event.number + 1, event.value ? "pressed" : "released");
                break;
            case JS_EVENT_AXIS:
                axis = get_axis_state(&event, axes);
                /*if (axis < 3)
                    printf("Axis %zu at (%6d, %6d)\n", axis, axes[axis].x, axes[axis].y);*/
                break;
            default:
                break;
        }

        fflush(stdout);
    }
}

