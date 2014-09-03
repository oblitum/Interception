#include <windows.h>
#include <winioctl.h>

#include "interception.h"

#define IOCTL_SET_PRECEDENCE    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_PRECEDENCE    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SET_FILTER        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_FILTER        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SET_EVENT         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE             CTL_CODE(FILE_DEVICE_UNKNOWN, 0x820, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_READ              CTL_CODE(FILE_DEVICE_UNKNOWN, 0x840, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_HARDWARE_ID   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x880, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _KEYBOARD_INPUT_DATA
{
    USHORT UnitId;
    USHORT MakeCode;
    USHORT Flags;
    USHORT Reserved;
    ULONG  ExtraInformation;
} KEYBOARD_INPUT_DATA, *PKEYBOARD_INPUT_DATA;

typedef struct _MOUSE_INPUT_DATA
{
    USHORT UnitId;
    USHORT Flags;
    USHORT ButtonFlags;
    USHORT ButtonData;
    ULONG  RawButtons;
    LONG   LastX;
    LONG   LastY;
    ULONG  ExtraInformation;
} MOUSE_INPUT_DATA, *PMOUSE_INPUT_DATA;

typedef struct
{
    void *handle;
    void *unempty;
} *InterceptionDeviceArray;

InterceptionContext interception_create_context(void)
{
    InterceptionDeviceArray device_array = 0;
    char keyboard_name[] = "\\\\.\\interception00";
    char mouse_name[] = "\\\\.\\interception10";
    char *device_name;
    DWORD bytes_returned;
    InterceptionDevice i;

    device_array = (InterceptionDeviceArray)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERCEPTION_MAX_DEVICE * sizeof(*((InterceptionDeviceArray) 0)));
    if(!device_array) return 0;

    for(i = 0; i < INTERCEPTION_MAX_DEVICE; ++i)
    {
        HANDLE zero_padded_handle[2] = {0};

        if(i < INTERCEPTION_MAX_KEYBOARD)
        {
            device_name = keyboard_name;
            device_name[sizeof(keyboard_name) - 2] = '0' + (char)i;
        }
        else
        {
            device_name = mouse_name;
            device_name[sizeof(mouse_name) - 2] = '0' + (char)i - INTERCEPTION_MAX_KEYBOARD;
        }

        device_array[i].handle = CreateFile(device_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
        device_array[i].unempty = CreateEvent(NULL, TRUE, FALSE, NULL);

        if(device_array[i].handle == 0 || device_array[i].unempty == 0)
        {
            interception_destroy_context(device_array);
            return 0;
        }

        zero_padded_handle[0] = device_array[i].unempty; 

        if(!DeviceIoControl(device_array[i].handle, IOCTL_SET_EVENT, zero_padded_handle, sizeof(zero_padded_handle), NULL, 0, &bytes_returned, NULL))
        {
            interception_destroy_context(device_array);
            return 0;
        }
    }

    return device_array;
}

void interception_destroy_context(InterceptionContext context)
{
    InterceptionDeviceArray device_array = (InterceptionDeviceArray)context;
    unsigned int i;

    if(!context) return;

    for(i = 0; i < INTERCEPTION_MAX_DEVICE; ++i)
    {
        if(device_array[i].handle)
            CloseHandle(device_array[i].handle);
        else
            break;

        if(device_array[i].unempty)
            CloseHandle(device_array[i].unempty);
        else
            break;
    }

    HeapFree(GetProcessHeap(), 0, context);
}

InterceptionPrecedence interception_get_precedence(InterceptionContext context, InterceptionDevice device)
{
    InterceptionDeviceArray device_array = (InterceptionDeviceArray)context;
    InterceptionPrecedence precedence = 0;
    DWORD bytes_returned;

    if(context)
        DeviceIoControl(device_array[device - 1].handle, IOCTL_GET_PRECEDENCE, NULL, 0, (LPVOID)&precedence, sizeof(InterceptionPrecedence), &bytes_returned, NULL);

    return precedence;
}

void interception_set_precedence(InterceptionContext context, InterceptionDevice device, InterceptionPrecedence precedence)
{
    InterceptionDeviceArray device_array = (InterceptionDeviceArray)context;
    DWORD bytes_returned;

    if(context)
        DeviceIoControl(device_array[device - 1].handle, IOCTL_SET_PRECEDENCE, (LPVOID)&precedence, sizeof(InterceptionPrecedence), NULL, 0, &bytes_returned, NULL);
}

InterceptionFilter interception_get_filter(InterceptionContext context, InterceptionDevice device)
{
    InterceptionDeviceArray device_array = (InterceptionDeviceArray)context;
    InterceptionFilter filter = 0;
    DWORD bytes_returned;

    if(context)
        DeviceIoControl(device_array[device - 1].handle, IOCTL_GET_FILTER, NULL, 0, (LPVOID)&filter, sizeof(InterceptionFilter), &bytes_returned, NULL);

    return filter;
}

void interception_set_filter(InterceptionContext context, InterceptionPredicate interception_predicate, InterceptionFilter filter)
{
    InterceptionDeviceArray device_array = (InterceptionDeviceArray)context;
    InterceptionDevice i;
    DWORD bytes_returned;

    if(context)
        for(i = 0; i < INTERCEPTION_MAX_DEVICE; ++i)
            if(interception_predicate(i + 1))
                DeviceIoControl(device_array[i].handle, IOCTL_SET_FILTER, (LPVOID)&filter, sizeof(InterceptionFilter), NULL, 0, &bytes_returned, NULL);
}

InterceptionDevice interception_wait(InterceptionContext context)
{
    return interception_wait_with_timeout(context, INFINITE);
}

InterceptionDevice interception_wait_with_timeout(InterceptionContext context, unsigned long milliseconds)
{
    InterceptionDeviceArray device_array = (InterceptionDeviceArray)context;
    HANDLE wait_handles[INTERCEPTION_MAX_DEVICE];
    InterceptionDevice i;

    if(!context) return 0;

    for(i = 0; i < INTERCEPTION_MAX_DEVICE; ++i) wait_handles[i] = device_array[i].unempty;

    i = WaitForMultipleObjects(INTERCEPTION_MAX_DEVICE, wait_handles, FALSE, milliseconds);

    if(i ==  WAIT_FAILED || i == WAIT_TIMEOUT) return 0;

    return i + 1;
}

int interception_send(InterceptionContext context, InterceptionDevice device, const InterceptionStroke *stroke, unsigned int nstroke)
{
    InterceptionDeviceArray device_array = (InterceptionDeviceArray)context;
    DWORD strokeswritten = 0;

    if(context == 0 || nstroke == 0 || interception_is_invalid(device)) return 0;

    if(interception_is_keyboard(device))
    {
        PKEYBOARD_INPUT_DATA rawstrokes = (PKEYBOARD_INPUT_DATA)HeapAlloc(GetProcessHeap(), 0, nstroke * sizeof(KEYBOARD_INPUT_DATA));
        unsigned int i;

        if(!rawstrokes) return 0;

        for(i = 0; i < nstroke; ++i)
        {
            InterceptionKeyStroke *key_stroke = (InterceptionKeyStroke *) stroke;
            
            rawstrokes[i].UnitId = 0;
            rawstrokes[i].MakeCode = key_stroke[i].code;
            rawstrokes[i].Flags = key_stroke[i].state;
            rawstrokes[i].Reserved = 0;
            rawstrokes[i].ExtraInformation = key_stroke[i].information;
        }

        DeviceIoControl(device_array[device - 1].handle, IOCTL_WRITE, rawstrokes,(DWORD)nstroke * sizeof(KEYBOARD_INPUT_DATA), NULL, 0, &strokeswritten, NULL);

        HeapFree(GetProcessHeap(), 0,  rawstrokes);

        strokeswritten /= sizeof(KEYBOARD_INPUT_DATA);
    }
    else
    {
        PMOUSE_INPUT_DATA rawstrokes = (PMOUSE_INPUT_DATA)HeapAlloc(GetProcessHeap(), 0, nstroke * sizeof(MOUSE_INPUT_DATA));
        unsigned int i;

        if(!rawstrokes) return 0;

        for(i = 0; i < nstroke; ++i)
        {
            InterceptionMouseStroke *mouse_stroke = (InterceptionMouseStroke *) stroke;
            
            rawstrokes[i].UnitId = 0;
            rawstrokes[i].Flags = mouse_stroke[i].flags;
            rawstrokes[i].ButtonFlags = mouse_stroke[i].state;
            rawstrokes[i].ButtonData = mouse_stroke[i].rolling;
            rawstrokes[i].RawButtons = 0;
            rawstrokes[i].LastX = mouse_stroke[i].x;
            rawstrokes[i].LastY = mouse_stroke[i].y;
            rawstrokes[i].ExtraInformation = mouse_stroke[i].information;
        }

        DeviceIoControl(device_array[device - 1].handle, IOCTL_WRITE, rawstrokes, (DWORD)nstroke * sizeof(MOUSE_INPUT_DATA), NULL, 0, &strokeswritten, NULL);

        HeapFree(GetProcessHeap(), 0,  rawstrokes);

        strokeswritten /= sizeof(MOUSE_INPUT_DATA);
    }

    return strokeswritten;
}

int interception_receive(InterceptionContext context, InterceptionDevice device, InterceptionStroke *stroke, unsigned int nstroke)
{
    InterceptionDeviceArray device_array = (InterceptionDeviceArray)context;
    DWORD strokesread = 0;

    if(context == 0 || nstroke == 0 || interception_is_invalid(device)) return 0;

    if(interception_is_keyboard(device))
    {
        PKEYBOARD_INPUT_DATA rawstrokes = (PKEYBOARD_INPUT_DATA)HeapAlloc(GetProcessHeap(), 0, nstroke * sizeof(KEYBOARD_INPUT_DATA));
        unsigned int i;

        if(!rawstrokes) return 0;

        DeviceIoControl(device_array[device - 1].handle, IOCTL_READ, NULL, 0, rawstrokes, (DWORD)nstroke * sizeof(KEYBOARD_INPUT_DATA), &strokesread, NULL);

        strokesread /= sizeof(KEYBOARD_INPUT_DATA);

        for(i = 0; i < (unsigned int)strokesread; ++i)
        {
            InterceptionKeyStroke *key_stroke = (InterceptionKeyStroke *) stroke;
            
            key_stroke[i].code = rawstrokes[i].MakeCode;
            key_stroke[i].state = rawstrokes[i].Flags;
            key_stroke[i].information = rawstrokes[i].ExtraInformation;
        }

        HeapFree(GetProcessHeap(), 0,  rawstrokes);
    }
    else
    {
        PMOUSE_INPUT_DATA rawstrokes = (PMOUSE_INPUT_DATA)HeapAlloc(GetProcessHeap(), 0, nstroke * sizeof(MOUSE_INPUT_DATA));
        unsigned int i;

        if(!rawstrokes) return 0;

        DeviceIoControl(device_array[device - 1].handle, IOCTL_READ, NULL, 0, rawstrokes, (DWORD)nstroke * sizeof(MOUSE_INPUT_DATA), &strokesread, NULL);

        strokesread /= sizeof(MOUSE_INPUT_DATA);

        for(i = 0; i < (unsigned int)strokesread; ++i)
        {
            InterceptionMouseStroke *mouse_stroke = (InterceptionMouseStroke *) stroke;
            
            mouse_stroke[i].flags = rawstrokes[i].Flags;
            mouse_stroke[i].state = rawstrokes[i].ButtonFlags;
            mouse_stroke[i].rolling = rawstrokes[i].ButtonData;
            mouse_stroke[i].x = rawstrokes[i].LastX;
            mouse_stroke[i].y = rawstrokes[i].LastY;
            mouse_stroke[i].information = rawstrokes[i].ExtraInformation;
        }

        HeapFree(GetProcessHeap(), 0,  rawstrokes);
    }

    return strokesread;
}

unsigned int interception_get_hardware_id(InterceptionContext context, InterceptionDevice device, void *hardware_id_buffer, unsigned int buffer_size)
{
    InterceptionDeviceArray device_array = (InterceptionDeviceArray)context;
    DWORD output_size = 0;

    if(context == 0 || interception_is_invalid(device)) return 0;
    
    DeviceIoControl(device_array[device - 1].handle, IOCTL_GET_HARDWARE_ID, NULL, 0, hardware_id_buffer, buffer_size, &output_size, NULL);

    return output_size;
}

int interception_is_invalid(InterceptionDevice device)
{
    return !interception_is_keyboard(device) && !interception_is_mouse(device);
}

int interception_is_keyboard(InterceptionDevice device)
{
    return device >= INTERCEPTION_KEYBOARD(0) && device <= INTERCEPTION_KEYBOARD(INTERCEPTION_MAX_KEYBOARD - 1);
}

int interception_is_mouse(InterceptionDevice device)
{
    return device >= INTERCEPTION_MOUSE(0) && device <= INTERCEPTION_MOUSE(INTERCEPTION_MAX_MOUSE - 1);
}

int interception_enumerate_devices(InterceptionContext context, InterceptionDevice device_array[], int device_array_size, int device_type)
{
    int actual_devices = 0;
    int hardware_id_length = 0;
    wchar_t hardware_id[1024];
    InterceptionDevice i;

    if (device_array_size < 1) return 0;

    for (i = 0; i < device_array_size && i < INTERCEPTION_MAX_DEVICE; ++i)
    {
        hardware_id_length = interception_get_hardware_id(context, i, (void*)hardware_id, sizeof(hardware_id));
        if (hardware_id_length > 0 && hardware_id_length < sizeof(hardware_id))
        {
            if ((device_type & INTERCEPTION_DEVICE_KEYBOARD) && interception_is_keyboard(i)
                || (device_type & INTERCEPTION_DEVICE_MOUSE) && interception_is_mouse(i))
			{
                device_array[actual_devices] = i;
                ++actual_devices;
            }
        }
    }

    return actual_devices;
}

int interception_enumerate_keyboards(InterceptionContext context, InterceptionDevice device_array[], int device_array_size)
{
    return interception_enumerate_devices(context, device_array, device_array_size, INTERCEPTION_DEVICE_KEYBOARD);
}

int interception_enumerate_mouses(InterceptionContext context, InterceptionDevice device_array[], int device_array_size)
{
    return interception_enumerate_devices(context, device_array, device_array_size, INTERCEPTION_DEVICE_MOUSE);
}