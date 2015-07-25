#include <interception.h>
#include <utils.h>

#include <iostream>

enum ScanCode
{
    SCANCODE_ESC = 0x01
};

int main()
{
    using namespace std;

    InterceptionContext context;
    InterceptionDevice device;
    InterceptionStroke stroke;

    raise_process_priority();

    context = interception_create_context();

    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP);
    interception_set_filter(context, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_LEFT_BUTTON_DOWN);

    while(interception_receive(context, device = interception_wait(context), &stroke, 1) > 0)
    {
        if(interception_is_keyboard(device))
        {
            InterceptionKeyStroke &keystroke = *(InterceptionKeyStroke *) &stroke;

            cout << "INTERCEPTION_KEYBOARD(" << device - INTERCEPTION_KEYBOARD(0) << ")" << endl;

            if(keystroke.code == SCANCODE_ESC) break;
        }

        if(interception_is_mouse(device))
        {
            cout << "INTERCEPTION_MOUSE(" << device - INTERCEPTION_MOUSE(0) << ")" << endl;
        }

        interception_send(context, device, &stroke, 1);
    }

    interception_destroy_context(context);

    return 0;
}
