/* Windows version of caps2esc: https://github.com/alexandre/caps2esc.
 *
 * For building check repository README.
 *
 * When building from WDK, the setting UMTYPE=windows in the 'sources' file
 * will make this application a "Windows" application without a console window
 * attached. As it also doesn't create any windows, you should check it's
 * executing from task manager, and if so, terminate it from there.
 *
 * This sample may not behave well from inside a VM since VM software sometimes
 * mess with generated keystrokes, specially the CTRL key.
 */

#include <vector>

#include <utils.h>
#include <interception.h>

using namespace std;

namespace scancode {
    enum {
        esc       = 0x01,
        ctrl      = 0x1D,
        capslock  = 0x3A
    };
}

InterceptionKeyStroke
    esc_down      = {scancode::esc     , INTERCEPTION_KEY_DOWN, 0},
    ctrl_down     = {scancode::ctrl    , INTERCEPTION_KEY_DOWN, 0},
    capslock_down = {scancode::capslock, INTERCEPTION_KEY_DOWN, 0},
    esc_up        = {scancode::esc     , INTERCEPTION_KEY_UP  , 0},
    ctrl_up       = {scancode::ctrl    , INTERCEPTION_KEY_UP  , 0},
    capslock_up   = {scancode::capslock, INTERCEPTION_KEY_UP  , 0};

bool operator==(const InterceptionKeyStroke &first,
                const InterceptionKeyStroke &second) {
    return first.code == second.code && first.state == second.state;
}

vector<InterceptionKeyStroke> caps2esc(const InterceptionKeyStroke &kstroke) {
    static bool capslock_is_down = false, esc_give_up = false;

    vector<InterceptionKeyStroke> kstrokes;

    if (capslock_is_down) {
        if (kstroke == capslock_down || kstroke.code == scancode::ctrl) {
            return kstrokes;
        }
        if (kstroke == capslock_up) {
            if (esc_give_up) {
                esc_give_up = false;
                kstrokes.push_back(ctrl_up);
            } else {
                kstrokes.push_back(esc_down);
                kstrokes.push_back(esc_up);
            }
            capslock_is_down = false;
            return kstrokes;
        }
        if (!esc_give_up) {
            esc_give_up = true;
            kstrokes.push_back(ctrl_down);
        }

        if (kstroke == esc_down)
            kstrokes.push_back(capslock_down);
        else if (kstroke == esc_up)
            kstrokes.push_back(capslock_up);
        else
            kstrokes.push_back(kstroke);

        return kstrokes;
    }

    if (kstroke == capslock_down) {
        capslock_is_down = true;
        return kstrokes;
    }

    if (kstroke == esc_down)
        kstrokes.push_back(capslock_down);
    else if (kstroke == esc_up)
        kstrokes.push_back(capslock_up);
    else
        kstrokes.push_back(kstroke);

    return kstrokes;
}

int main() {
    void *program_instance = try_open_single_program("407631B6-78D3-4EFC-A868-40BBB7204CF1");
    if (!program_instance) {
        return 0;
    }

    InterceptionContext context;
    InterceptionDevice device;
    InterceptionKeyStroke kstroke;

    raise_process_priority();

    context = interception_create_context();

    interception_set_filter(context, interception_is_keyboard,
                            INTERCEPTION_FILTER_KEY_DOWN |
                            INTERCEPTION_FILTER_KEY_UP);

    while (interception_receive(context, device = interception_wait(context),
                                (InterceptionStroke *)&kstroke, 1) > 0) {
        vector<InterceptionKeyStroke> kstrokes = caps2esc(kstroke);

        if (kstrokes.size() > 0) {
            interception_send(context, device,
                              (InterceptionStroke *)&kstrokes[0],
                              kstrokes.size());
        }
    }

    interception_destroy_context(context);

    close_single_program(program_instance);
}
