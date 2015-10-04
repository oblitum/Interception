#include <time.h>
#include <string.h>
#include <windows.h>

#include "utils.h"


void raise_process_priority(void)
{
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
}

void lower_process_priority(void)
{
    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
}

int get_screen_width(void)
{
    return GetSystemMetrics(SM_CXSCREEN);
}

int get_screen_height(void)
{
    return GetSystemMetrics(SM_CYSCREEN);
}

#pragma optimize("", off)

void busy_wait(unsigned long count)
{
    while(--count);
}

unsigned long calculate_busy_wait_millisecond(void)
{
    unsigned long count = 2000000000;
    time_t start = time(NULL);
    while(--count);
    return (unsigned long) (2000000 / difftime(time(NULL), start));
}

#pragma optimize("", on)

void *try_open_single_program(const char *name) {
    char full_name[255];
    HANDLE program_instance;

    strcpy(full_name, "Global\\{");
    strcat(full_name, name);
    strcat(full_name, "}");
    program_instance = CreateMutexA(NULL, FALSE, full_name);
    if (GetLastError() == ERROR_ALREADY_EXISTS || program_instance == NULL) {
        return NULL;
    }
    return program_instance;
}

void close_single_program(void *program_instance) {
    CloseHandle(program_instance);
}
