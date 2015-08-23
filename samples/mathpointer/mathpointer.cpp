#include <cmath>
#include <iostream>

#include <utils.h>
#include <interception.h>

using namespace std;

namespace scancode {
    enum {
        esc   = 0x01,
        num_0 = 0x0B,
        num_1 = 0x02,
        num_2 = 0x03,
        num_3 = 0x04,
        num_4 = 0x05,
        num_5 = 0x06,
        num_6 = 0x07,
        num_7 = 0x08,
        num_8 = 0x09,
        num_9 = 0x0A,
    };
}

const double pi = 3.14159265358979323846264338327950288419716939937510582097494;
const double scale = 15;
const int screen_width = get_screen_width(), screen_height = get_screen_height();
const unsigned long milliseconds = calculate_busy_wait_millisecond();

struct point {
    double x;
    double y;
    point(double x, double y) : x(x), y(y) {}
};

typedef point (*curve)(double t);

point circle(double t) {
    double f = 10;

    return point(scale * f * cos(t), scale * f * sin(t));
}

point mirabilis(double t) {
    double f = 1. / 2., k = 1. / (2. * pi);

    return point(scale * f * (exp(k * t) * cos(t)),
                 scale * f * (exp(k * t) * sin(t)));
}

point epitrochoid(double t) {
    double f = 1, R = 6, r = 2, d = 1;
    double c = R + r;

    return point(scale * f * (c * cos(t) - d * cos((c * t) / r)),
                 scale * f * (c * sin(t) - d * sin((c * t) / r)));
}

point hypotrochoid(double t) {
    double f = 10. / 7., R = 5, r = 3, d = 5;
    double c = R - r;

    return point(scale * f * (c * cos(t) + d * cos((c * t) / r)),
                 scale * f * (c * sin(t) - d * sin((c * t) / r)));
}

point hypocycloid(double t) {
    double f = 10. / 3., R = 3, r = 1;
    double c = R - r;

    return point(scale * f * (c * cos(t) + r * cos((c * t) / r)),
                 scale * f * (c * sin(t) - r * sin((c * t) / r)));
}

point bean(double t) {
    double f = 10, c = cos(t), s = sin(t);

    return point(scale * f * ((pow(c, 3) + pow(s, 3)) * c),
                 scale * f * ((pow(c, 3) + pow(s, 3)) * s));
}

point Lissajous(double t) {
    double f = 10, a = 2, b = 3;

    return point(scale * f * (sin(a * t)), scale * f * (sin(b * t)));
}

point epicycloid(double t) {
    double f = 10. / 42., R = 21, r = 10;
    double c = R + r;

    return point(scale * f * (c * cos(t) - r * cos((c * t) / r)),
                 scale * f * (c * sin(t) - r * sin((c * t) / r)));
}

point rose(double t) {
    double f = 10, R = 1, k = 2. / 7.;

    return point(scale * f * (R * cos(k * t) * cos(t)),
                 scale * f * (R * cos(k * t) * sin(t)));
}

point butterfly(double t) {
    double f = 10. / 4., c = exp(cos(t)) - 2 * cos(4 * t) + pow(sin(t / 12), 5);

    return point(scale * f * (sin(t) * c), scale * f * (cos(t) * c));
}

void math_track(InterceptionContext context, InterceptionDevice mouse,
                curve curve, point center, double t1, double t2,
                unsigned int partitioning) {
    lower_process_priority();

    InterceptionMouseStroke mstroke;
    double delta = t2 - t1;
    point position = curve(t1);

    mstroke.flags = INTERCEPTION_MOUSE_MOVE_ABSOLUTE;

    mstroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_UP;
    mstroke.x = static_cast<int>((0xFFFF * center.x) / screen_width);
    mstroke.y = static_cast<int>((0xFFFF * center.y) / screen_height);
    interception_send(context, mouse, (InterceptionStroke *)&mstroke, 1);

    mstroke.state = 0;
    mstroke.x = static_cast<int>((0xFFFF * (center.x + position.x)) / screen_width);
    mstroke.y = static_cast<int>((0xFFFF * (center.y - position.y)) / screen_height);
    interception_send(context, mouse, (InterceptionStroke *)&mstroke, 1);

    for (unsigned int i = 0, j = 0; i <= partitioning + 2; ++i, ++j) {
        if (j % 250 == 0) {
            busy_wait(25 * milliseconds);
            mstroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_UP;
            interception_send(context, mouse, (InterceptionStroke *)&mstroke, 1);

            busy_wait(25 * milliseconds);
            mstroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN;
            interception_send(context, mouse, (InterceptionStroke *)&mstroke, 1);
            mstroke.state = 0;

            if (i > 0)
                i -= 2;
        }

        position = curve(t1 + (i * delta) / partitioning);
        mstroke.x = static_cast<int>((0xFFFF * (center.x + position.x)) / screen_width);
        mstroke.y = static_cast<int>((0xFFFF * (center.y - position.y)) / screen_height);
        interception_send(context, mouse, (InterceptionStroke *)&mstroke, 1);

        busy_wait(3 * milliseconds);
    }

    busy_wait(25 * milliseconds);
    mstroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN;
    interception_send(context, mouse, (InterceptionStroke *)&mstroke, 1);

    busy_wait(25 * milliseconds);
    mstroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_UP;
    interception_send(context, mouse, (InterceptionStroke *)&mstroke, 1);

    busy_wait(25 * milliseconds);
    mstroke.state = 0;
    mstroke.x = static_cast<int>((0xFFFF * center.x) / screen_width);
    mstroke.y = static_cast<int>((0xFFFF * center.y) / screen_height);
    interception_send(context, mouse, (InterceptionStroke *)&mstroke, 1);

    raise_process_priority();
}

int main() {
    InterceptionContext context;
    InterceptionDevice device, mouse = 0;
    InterceptionStroke stroke;
    point position(screen_width / 2, screen_height / 2);

    raise_process_priority();

    context = interception_create_context();

    interception_set_filter(context, interception_is_keyboard,
                            INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP);
    interception_set_filter(context, interception_is_mouse,
                            INTERCEPTION_FILTER_MOUSE_MOVE);

    cout << "NOTICE: This example works on real machines.\n"
         << "        Virtual machines generally work with absolute mouse\n"
         << "        positioning over the screen, which this samples isn't\n"
         << "        prepared to handle.\n\n";

    cout << "Now please, first move the mouse that's going to be impersonated." << endl;

    while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0) {
        if (interception_is_mouse(device)) {
            if (!mouse) {
                mouse = device;
                cout << "Impersonating mouse " << device - INTERCEPTION_MOUSE(0) << ".\n\n";

                cout << "Now:\n"
                     << "  - Go to Paint (or whatever place you want to draw)\n"
                     << "  - Select your pencil\n"
                     << "  - Position your mouse in the drawing board\n"
                     << "  - Press any digit (not numpad) on your keyboard to draw an equation\n"
                     << "  - Press ESC to exit." << endl;
            }

            InterceptionMouseStroke &mstroke = *(InterceptionMouseStroke *)&stroke;

            position.x += mstroke.x;
            position.y += mstroke.y;

            if (position.x < 0)
                position.x = 0;
            if (position.x > screen_width - 1)
                position.x = screen_width - 1;
            if (position.y < 0)
                position.y = 0;
            if (position.y > screen_height - 1)
                position.y = screen_height - 1;

            mstroke.flags = INTERCEPTION_MOUSE_MOVE_ABSOLUTE;
            mstroke.x = static_cast<int>((0xFFFF * position.x) / screen_width);
            mstroke.y = static_cast<int>((0xFFFF * position.y) / screen_height);

            interception_send(context, device, &stroke, 1);
        }

        if (mouse && interception_is_keyboard(device)) {
            InterceptionKeyStroke &kstroke = *(InterceptionKeyStroke *)&stroke;

            switch (kstroke.state) {
                case INTERCEPTION_KEY_DOWN:
                    switch (kstroke.code) {
                        case scancode::num_0:
                            math_track(context, mouse, circle, position, 0, 2 * pi, 200);
                            break;
                        case scancode::num_1:
                            math_track(context, mouse, mirabilis, position, -(6 * pi), 6 * pi, 200);
                            break;
                        case scancode::num_2:
                            math_track(context, mouse, epitrochoid, position, 0, 2 * pi, 200);
                            break;
                        case scancode::num_3:
                            math_track(context, mouse, hypotrochoid, position, 0, 6 * pi, 200);
                            break;
                        case scancode::num_4:
                            math_track(context, mouse, hypocycloid, position, 0, 2 * pi, 200);
                            break;
                        case scancode::num_5:
                            math_track(context, mouse, bean, position, 0, pi, 200);
                            break;
                        case scancode::num_6:
                            math_track(context, mouse, Lissajous, position, 0, 2 * pi, 200);
                            break;
                        case scancode::num_7:
                            math_track(context, mouse, epicycloid, position, 0, 20 * pi, 1000);
                            break;
                        case scancode::num_8:
                            math_track(context, mouse, rose, position, 0, 14 * pi, 500);
                            break;
                        case scancode::num_9:
                            math_track(context, mouse, butterfly, position, 0, 21 * pi, 2000);
                            break;
                        default:
                            interception_send(context, device, &stroke, 1);
                            break;
                    }
                    break;
                case INTERCEPTION_KEY_UP:
                    switch (kstroke.code) {
                        case scancode::num_0:
                        case scancode::num_1:
                        case scancode::num_2:
                        case scancode::num_3:
                        case scancode::num_4:
                        case scancode::num_5:
                        case scancode::num_6:
                        case scancode::num_7:
                        case scancode::num_8:
                        case scancode::num_9:
                            break;
                        default:
                            interception_send(context, device, &stroke, 1);
                            break;
                    }
                    break;
                default:
                    interception_send(context, device, &stroke, 1);
                    break;
            }

            if (kstroke.code == scancode::esc)
                break;
        }
    }

    interception_destroy_context(context);
}
