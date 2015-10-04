#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

void raise_process_priority(void);
void lower_process_priority(void);
int get_screen_width(void);
int get_screen_height(void);
void busy_wait(unsigned long count);
unsigned long calculate_busy_wait_millisecond(void);
void *try_open_single_program(const char *name);
void close_single_program(void *program_instance);

#ifdef __cplusplus
}
#endif

#endif
