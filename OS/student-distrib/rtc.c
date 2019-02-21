#include "rtc.h"
#include "types.h"

#define RTC_IRQ 		8
#define REG_A			0x8A
#define REG_B			0x8B
#define RTC_REG_C 		0x0C
#define RTC_PORT		0x70
#define CMOS_PORT		0x71
#define TURN_VAL 		0x40
#define MASK_BIT 		0xF0
#define FREQ_2HZ 		0x0F
#define MAX_FREQ 		1024
#define REG_A_FREQ 		16

int rtc_flag = 0;

/*
* void rtc_init()
*   input: none
*   output: none
*	description: initialize rtc interrupt
*/
void rtc_init(){
	unsigned char prev;
	outb(REG_B, RTC_PORT);
	prev = inb(CMOS_PORT);
	outb(REG_B, RTC_PORT);
	outb(prev | TURN_VAL, CMOS_PORT);
}

/*
* void rtc_handler()
*   input: none
*   output: none
*	description: handle rtc interrupt and send end of interrupt signal
*/
void rtc_interrupt_handler(){
	cli();							// enter critical session

	outb(RTC_REG_C, RTC_PORT);
	inb(CMOS_PORT);
	send_eoi(RTC_IRQ);				// send end of interrupt
	rtc_flag = 0;

	sti();							// end critical session
}


/*
* void write_rtc(unsigned int freq)
*   inputs: frequency
*   output: none
*	description: set RTC frequency to input frequency
*/
int write_rtc(unsigned int freq){
	int ret = -1;
	unsigned char freq_value = REG_A_FREQ;

	if (freq >= 1 && freq <= MAX_FREQ){
		while (freq != 1){
			freq /= 2;
			freq_value--;
		}

	 	/* set Reg_A value to set frequency of interrupt */
		outb(REG_A, RTC_PORT);
		unsigned char a_reg = inb(CMOS_PORT);
		outb(REG_A, RTC_PORT);
		outb((a_reg & MASK_BIT) | freq_value, CMOS_PORT);

		ret = 0;
	}

	return ret;
}


/*
 * int rtc_read(int fd, void* buf, int nbytes)
 * input: file descriptor, data store buffer, number of bytes
 * output: 0 for success
 * description : read data from rtc file
 */
int rtc_read(int fd, void* buf, int nbytes){
	rtc_flag = 1;

	while (rtc_flag){}

	return 0;
}

/*
 * int rtc_write(int fd, const void* buf, int nbytes)
 * input: file descriptor, data store buffer, number of bytes
 * output: 0 for success
 * description : write to rtc file
 */
int rtc_write(int fd, const void* buf, int nbytes){
	write_rtc((unsigned int)buf);

	return 0;
}

/*
 * int rtc_open(const unsigned char* filename)
 * input: name of file
 * output: 0 for success
 * description : open rtc file
 */
int rtc_open(const unsigned char* filename){
	write_rtc(2);
	rtc_freq = 1;
	enable_irq(RTC_IRQ);

	return 0;
}

/*
 * int rtc_close(const unsigned char* filename)
 * input: file descriptor
 * output: 0 for success
 * description : close rtc file
 */
int rtc_close(int fd){
	write_rtc(2);

	return 0;
}
