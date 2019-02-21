#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "lib.h"
#include "i8259.h"
#include "file_system.h"
//#include "scheduling.h"
//#include "system_call.h"
#include "rtc.h"
#include "paging.h"
#include "function_func.h"
#define KEYBOARD_INPUT 0x3E
#define IOPORT 0x60
#define LEFT_SHIFT 0x2A
#define RIGHT_SHIFT 0x36
#define LEFT_SHIFT_OFF 0xAA
#define RIGHT_SHIFT_OFF 0xB6
#define ENTER_KEY 0x1C
#define CAPS_KEY 0x3A
#define CTRL_KEY 0x1D
#define CTRL_OFF 0x9D
#define BACKSPACE_KEY 0x0E
#define MAX_RTC 1024
#define MAX_BUFFER 128
#define MAX_LENGTH 7
#define F1_KEY 0x3B
#define F2_KEY 0x3C
#define F3_KEY 0x3D
#define ALT_KEY 0x38
#define ALT_OFF  0xB8

extern void keyboard_handler();
extern void keyboard_init();
extern unsigned char buffer[3][MAX_BUFFER];
extern unsigned int buffer_index[3];
extern volatile unsigned int enter_flag[3];
extern int rtc_freq;

extern unsigned char keyboard_interpret(unsigned char input);
extern void reset_buffer(unsigned char* buffer_array);
extern void display_file_list();


#endif
