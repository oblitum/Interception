#include <interception.h>
#include <utils.h>

#include <iostream>
#include <deque>

enum ScanCode
{
    SCANCODE_ESC = 0x01
};

InterceptionKeyStroke nothing = {};
InterceptionKeyStroke ctrl_down = {0x1D, INTERCEPTION_KEY_DOWN};
InterceptionKeyStroke alt_down  = {0x38, INTERCEPTION_KEY_DOWN};
InterceptionKeyStroke del_down  = {0x53, INTERCEPTION_KEY_DOWN | INTERCEPTION_KEY_E0};

bool operator == (const InterceptionKeyStroke &first, const InterceptionKeyStroke &second)
{
    return first.code == second.code && first.state == second.state;
}

bool operator != (const InterceptionKeyStroke &first, const InterceptionKeyStroke &second)
{
    return !(first == second);
}

int main()
{
    using namespace std;

    InterceptionContext context;
    InterceptionDevice device;
    InterceptionKeyStroke new_stroke, last_stroke;

    deque<InterceptionKeyStroke> stroke_sequence;

    stroke_sequence.push_back(nothing);
    stroke_sequence.push_back(nothing);
    stroke_sequence.push_back(nothing);

    raise_process_priority();

    context = interception_create_context();

    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);

    while(interception_receive(context, device = interception_wait(context), (InterceptionStroke *)&new_stroke, 1) > 0)
    {
        if(new_stroke != last_stroke)
        {
            stroke_sequence.pop_front();
            stroke_sequence.push_back(new_stroke);
        }

        if(stroke_sequence[0] == ctrl_down && stroke_sequence[1] == alt_down && stroke_sequence[2] == del_down)
            cout << "ctrl-alt-del pressed" << endl;
        else if(stroke_sequence[0] == alt_down && stroke_sequence[1] == ctrl_down && stroke_sequence[2] == del_down)
            cout << "alt-ctrl-del pressed" << endl;
        else
            interception_send(context, device, (InterceptionStroke *)&new_stroke, 1);

        if(new_stroke.code == SCANCODE_ESC) break;

        last_stroke = new_stroke;
    }

    interception_destroy_context(context);

    return 0;
}
