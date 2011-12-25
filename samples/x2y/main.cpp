#include <interception.h>
#include "utils.h"

enum ScanCode
{
    SCANCODE_X   = 0x2D,
    SCANCODE_Y   = 0x15,
    SCANCODE_ESC = 0x01
};

int main()
{
    InterceptionContext context;
    InterceptionDevice device;
    InterceptionKeyboardStroke stroke;
    
    raise_process_priority();
    
    context = interception_create_context();
    
    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_KEY_FILTER_DOWN | INTERCEPTION_KEY_FILTER_UP);

    while(interception_receive(context, device = interception_wait(context), (InterceptionStroke *)&stroke, 1) > 0)
    {
        if(stroke.code == SCANCODE_X) stroke.code = SCANCODE_Y;

        interception_send(context, device, (const InterceptionStroke *)&stroke, 1);

        if(stroke.code == SCANCODE_ESC) break;
    }

    interception_destroy_context(context);

    return 0;
}
