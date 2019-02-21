#ifndef _PAGING_H
#define _PAGING_H

#include "lib.h"
#include "types.h"

#define PD_SIZE    		1024
#define RWFLAG			0x07
#define USER_FLAG		0x06
#define VID_MEM  		0xB8
#define BITWISE			0x1
#define VIRTUAL_ADDR	0x08000000
#define USER_PRE 		0x87
#define FREE_INIT 		0x03
#define SIZE			32
#define BIT_22			22
#define BIT_12			12
#define SIZE_FLAG 		0x81
#define PRESENT 		0x02


extern unsigned int page_directory[PD_SIZE] __attribute__((aligned(FOURKB)));
extern unsigned int page_table[PD_SIZE] __attribute__((aligned(FOURKB)));
extern unsigned int page_vid[PD_SIZE] __attribute__((aligned(FOURKB)));

/*functions used for paging declared*/
extern void page_enable(); // enable paging by loading the page directory
extern void paging_init(); // initialize paging
extern int allocate(); // allocate new page
extern void map_init(int idx); // intialize the map to the address to start the program
void video_map(int terminal_num, int curr); 
void next_terminal(int next, int curr);
extern void flush(); //flush the TLB

#endif
