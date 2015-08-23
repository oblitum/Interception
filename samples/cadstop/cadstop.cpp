#include <iostream>

#include <utils.h>
#include <interception.h>

using namespace std;

namespace scancode {
    enum {
        esc  = 0x01,
        ctrl = 0x1D,
        alt  = 0x38,
        del  = 0x53,
    };
}

InterceptionKeyStroke ctrl_down = {scancode::ctrl, INTERCEPTION_KEY_DOWN                      , 0};
InterceptionKeyStroke alt_down  = {scancode::alt , INTERCEPTION_KEY_DOWN                      , 0};
InterceptionKeyStroke del_down  = {scancode::del , INTERCEPTION_KEY_DOWN | INTERCEPTION_KEY_E0, 0};
InterceptionKeyStroke ctrl_up   = {scancode::ctrl, INTERCEPTION_KEY_UP                        , 0};
InterceptionKeyStroke alt_up    = {scancode::alt , INTERCEPTION_KEY_UP                        , 0};
InterceptionKeyStroke del_up    = {scancode::del , INTERCEPTION_KEY_UP | INTERCEPTION_KEY_E0  , 0};

bool operator==(const InterceptionKeyStroke &first,
                const InterceptionKeyStroke &second) {
    return first.code == second.code && first.state == second.state;
}

bool shall_produce_keystroke(const InterceptionKeyStroke &kstroke) {
    static int ctrl_is_down = 0, alt_is_down = 0, del_is_down = 0;

    if (ctrl_is_down + alt_is_down + del_is_down < 2) {
        if (kstroke == ctrl_down) { ctrl_is_down = 1; }
        if (kstroke == ctrl_up  ) { ctrl_is_down = 0; }
        if (kstroke == alt_down ) { alt_is_down = 1;  }
        if (kstroke == alt_up   ) { alt_is_down = 0;  }
        if (kstroke == del_down ) { del_is_down = 1;  }
        if (kstroke == del_up   ) { del_is_down = 0;  }
        return true;
    }

    if (ctrl_is_down == 0 && (kstroke == ctrl_down || kstroke == ctrl_up)) {
        return false;
    }

    if (alt_is_down == 0 && (kstroke == alt_down || kstroke == alt_up)) {
        return false;
    }

    if (del_is_down == 0 && (kstroke == del_down || kstroke == del_up)) {
        return false;
    }

    if (kstroke == ctrl_up) {
        ctrl_is_down = 0;
    } else if (kstroke == alt_up) {
        alt_is_down = 0;
    } else if (kstroke == del_up) {
        del_is_down = 0;
    }

    return true;
}

int main() {
    InterceptionContext context;
    InterceptionDevice device;
    InterceptionKeyStroke kstroke;

    raise_process_priority();

    context = interception_create_context();

    interception_set_filter(context, interception_is_keyboard,
                            INTERCEPTION_FILTER_KEY_ALL);

    while (interception_receive(context, device = interception_wait(context),
                                (InterceptionStroke *)&kstroke, 1) > 0) {
        if (!shall_produce_keystroke(kstroke)) {
            cout << "ctrl-alt-del pressed" << endl;
            continue;
        }

        interception_send(context, device, (InterceptionStroke *)&kstroke, 1);

        if (kstroke.code == scancode::esc)
            break;
    }

    interception_destroy_context(context);

    return 0;
}
