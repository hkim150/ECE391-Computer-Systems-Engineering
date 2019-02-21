#ifndef __SCHEDULING_H__
#define __SCHEDULING_H__

#include "types.h"
#include "system_call.h"
#include "file_system.h"
#include "paging.h"
#include "x86_desc.h"
#include "lib.h"

#define SIZE_OF_QUEUE	16
#define RUN_ABLE		0
#define RUN_UNABLE		-1

typedef struct task{
	char state;
	pcb_t* pcb;
	void* esp;
	void* ebp;
	unsigned int terminal_num;
} task_t;

extern task_t task_queue[SIZE_OF_QUEUE];
extern volatile int sched_terminal;

int sched_init();
int sched_task(int ebp, int esp);
int switch_task(int ebp, int esp);
int clear_struct(task_t* task_p);

int task_push(task_t* element);
int task_front(task_t** element);
int task_pop(task_t* element);

#endif

