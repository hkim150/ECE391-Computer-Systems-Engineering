#ifndef __RTC_H__
#define __RTC_H__

#include "i8259.h"
#include "lib.h"
#include "keyboard.h"

extern void rtc_init();
extern void rtc_handler();
extern int write_rtc(unsigned int freq);
extern int rtc_read(int fd, void* buf, int nbytes);
extern int rtc_write(int fd, const void* buf, int nbytes);
extern int rtc_open(const unsigned char* filename);
extern int rtc_close(int fd);


#endif
