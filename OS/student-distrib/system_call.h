#ifndef __SYSTEM_CALL_H__
#define __SYSTEM_CALL_H__

#include "file_system.h"
#include "types.h"
#include "lib.h"
#include "scheduling.h"

#define MAX_TASK_NUM		2
#define EXECUTE_HD_BYTE		40
#define NUM_COUNT 			4
#define FOUR_BYTES			4
#define PROG_ADDRESS		0x08048000
#define USER_CS_SELECT 		0x23
#define USER_DS_SELECT 		0x2B
#define PCB_MASK	 		0xFFFFE000
#define EIGHT_KILO_BYTES 	8192
#define EIGHT_MEGA_BYTES 	0x800000
#define TEN_BIT_MASK		0x3FF
#define VIDEO_ADDRESS		0x10000000
#define EMPTY_CHAR			'\0'
#define SPACE_CHAR			' '
#define USER_STACK          0x080FFFFF

extern int read_dentry_by_name (const unsigned char* fname, dentry_t* dentry);
extern int read_dentry_by_index (unsigned int index, dentry_t* dentry);
extern int read_data (unsigned int inode, unsigned int offset, unsigned char* buf, unsigned int length);
extern int system_read(int fd, void* buf, int nbytes);
extern int system_write(int fd, const void* buf, int nbytes);
extern int system_open(const unsigned char* filename);
extern int system_close(int fd);
extern int system_execute(const unsigned char* command);
extern int system_halt(unsigned char status);
extern int system_getargs(unsigned char* buf, int nbytes);
extern int system_vidmap(unsigned char** screen_start);

int next_pid();
void sched_vid();

#endif
