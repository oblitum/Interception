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

    wchar_t hardware_id[500];

    raise_process_priority();

    context = interception_create_context();

    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP);
    interception_set_filter(context, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_LEFT_BUTTON_DOWN);

    while(interception_receive(context, device = interception_wait(context), &stroke, 1) > 0)
    {
        if(interception_is_keyboard(device))
        {
            InterceptionKeyStroke &keystroke = *(InterceptionKeyStroke *) &stroke;

            if(keystroke.code == SCANCODE_ESC) break;
        }

        size_t length = interception_get_hardware_id(context, device, hardware_id, sizeof(hardware_id));

        if(length > 0 && length < sizeof(hardware_id))
            wcout << hardware_id << endl;

        interception_send(context, device, &stroke, 1);
    }

    interception_destroy_context(context);

    return 0;
}
