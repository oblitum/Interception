#include <interception.h>
#include <utils.h>

enum ScanCode
{
    SCANCODE_ESC = 0x01
};

int main()
{
    InterceptionContext context;
    InterceptionDevice device;
    InterceptionStroke stroke;

    raise_process_priority();

    context = interception_create_context();

    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP);
    interception_set_filter(context, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_MOVE);

    while(interception_receive(context, device = interception_wait(context), &stroke, 1) > 0)
    {
        if(interception_is_mouse(device))
        {
            InterceptionMouseStroke &mstroke = *(InterceptionMouseStroke *) &stroke;

            if(!(mstroke.flags & INTERCEPTION_MOUSE_MOVE_ABSOLUTE)) mstroke.y *= -1;

            interception_send(context, device, &stroke, 1);
        }

        if(interception_is_keyboard(device))
        {
            InterceptionKeyStroke &kstroke = *(InterceptionKeyStroke *) &stroke;

            interception_send(context, device, &stroke, 1);

            if(kstroke.code == SCANCODE_ESC) break;
        }
    }

    interception_destroy_context(context);

    return 0;
}
