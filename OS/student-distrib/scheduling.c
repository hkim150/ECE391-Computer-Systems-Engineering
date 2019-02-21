#include "scheduling.h"

task_t task_q[SIZE_OF_QUEUE];

static unsigned int front = 0;				// task queue indicies
static unsigned int back = 0;

static task_t curr;
static task_t* next;

volatile int sched_terminal = 1;


/*
 * int sched_init()
 * input: none
 * output: 0 to notify success
 * description: initialize task structures in task queue
 */
int sched_init(){
	int i;
	int ret = 0;

	for(i=0; i<SIZE_OF_QUEUE; i++){			// initialize structures in the queue
		clear_struct(&task_q[i]);
	}

	return ret;
}


/*
 * int sched_task(int ebp, int esp)
 * input: esp, ebp
 * output: 0 or -1 depending on success or failure respectively
 * description: add the state of current task to task queue
 */
int sched_task(int ebp, int esp){
	int ret = -1;
	
	curr.state = RUN_ABLE;										// copy the task structure status
	curr.ebp = (void*)ebp;
	curr.esp = (void*)esp;
	curr.pcb = (pcb_t*)(esp & PCB_MASK);
	curr.terminal_num = cursor_on_terminal;
	
	if(task_push(&curr) == -1){									// push it into the task queue
		asm volatile("movl %0, %%esp \n"
				  "movl %1, %%ebp \n"
				  :
				  : "g"(esp), "g"(ebp)
				  : "memory", "cc", "%esp", "%ebp");
		send_eoi(0);
		asm volatile("jmp task_switch_done \n");
	}
	else{
		if(task_front(&next) != -1 && cursor_on_terminal > 1){		// if there exists other running taks, make it first task
			sched_terminal = next->terminal_num;
			sched_vid();
			tss.esp0 = next->pcb->p_esp;
			asm volatile("movl %0, %%esp \n"
						  "movl %1, %%ebp \n"
						  :
						  : "g"(next->esp), "g"(next->ebp)
						  : "memory", "cc", "%esp", "%ebp");
			send_eoi(0);
			asm volatile("jmp task_switch_done \n");
		}
		asm volatile("movl %0, %%esp \n"						// restore pointers and return to current
					  "movl %1, %%ebp \n"
					  :
					  : "g"(esp), "g"(ebp)
					  : "memory", "cc", "%esp", "%ebp");
		send_eoi(0);
		asm volatile("jmp task_switch_done \n");
		ret = 0;
	}

	return ret;
}


/*
 * int switch_task(int ebp, int esp)
 * input: esp, ebp
 * output: 0 or -1 depending on success or failure respectively
 * description: take task from front of queue and switch it to current task
 */
int switch_task(int ebp, int esp){
	int ret = -1;

	if(task_pop(&curr) == -1){
		asm volatile("movl %0, %%esp \n"
				  	  "movl %1, %%ebp \n"
				  	  :
				  	  : "g"(esp), "g"(ebp)
					  : "memory", "cc", "%esp", "%ebp");
		send_eoi(0);
		asm volatile("jmp task_switch_done \n");
	}
	else{
		curr.esp = (void*)esp;										// store current state and update stack pointer
		curr.ebp = (void*)ebp;
		curr.pcb = (pcb_t*)(esp & PCB_MASK);
	
		tss.esp0 = curr.pcb->p_esp;

		if(task_front(&next) == -1){								// get task struct for next
			task_push(&curr);
			asm volatile("movl %0, %%esp \n"
					  	  "movl %1, %%ebp \n"
					  	  :
				  		  : "g"(esp), "g"(ebp)
						  : "memory", "cc", "%esp", "%ebp");
			send_eoi(0);
			asm volatile("jmp task_switch_done \n");
		}
		else{
			map_init(next->pcb->page_dir);
			if(task_push(&curr) == -1){								// push the current running struct
				asm volatile("movl %0, %%esp \n"
						  	  "movl %1, %%ebp \n"
						  	  :
						  	  : "g"(esp), "g"(ebp)
							  : "memory", "cc", "%esp", "%ebp");
				send_eoi(0);
				asm volatile("jmp task_switch_done \n");
			}
			else{
				tss.esp0 = next->pcb->p_esp;							// restore next state and update next stack pointer
				file_array = next->pcb->file_array;
				sched_terminal = next->terminal_num;
				sched_vid();

				asm volatile("movl %0, %%esp \n"					// update esp and ebp
							  "movl %1, %%ebp \n"
							  :
							  : "g"(next->esp), "g"(next->ebp)		// return to next task
							  : "memory", "cc", "%esp", "%ebp");

				send_eoi(0);
				asm volatile("jmp task_switch_done \n");
				ret = 0;
			}
		}
	}

	return ret;
}


/*
 * int clear_struct(task_t* task_p)
 * input: task struct to clear
 * output: 0 or -1 depending on success or failure respectively
 * description: clear the task struct
 */
int clear_struct(task_t* task_p){
	int ret = -1;

	if(task_p != NULL){
		task_p->state = RUN_UNABLE;
		task_p->pcb = NULL;
		task_p->esp = NULL;
		task_p->ebp = NULL;
		task_p->terminal_num = 0;
		ret = 0;
	}

	return ret;
}

/*
 * int task_push(task_t* element)
 * input: element to push to the queue
 * output: 0 or -1 depending on success or failure respectively
 * description: push the input task into the task queue
 */
int task_push(task_t* element){
	int ret = -1;

	if(element != NULL && task_q[back].state == RUN_ABLE){		// check input validity
		/* add data to the back of the stack */
		task_q[back].state =		element->state;
		task_q[back].pcb = 			element->pcb;
		task_q[back].esp = 			element->esp;
		task_q[back].ebp = 			element->ebp;
		task_q[back].terminal_num = element->terminal_num;

		back++;
		back %= SIZE_OF_QUEUE;								// increment back index
		ret = 0;
	}

	return ret;
}


/*
 * int task_front(task_t* element)
 * input: pointer to store the element at the front of the queue
 * output: 0 or -1 depending on success or failure respectively
 * description: check out the element at the front of the queue
 */
int task_front(task_t** element){
	int ret = -1;

	if(element != NULL && task_q[front].state == RUN_ABLE){		// check input validity
		*element = &(task_q[front]);								// make the element to point the front
		ret = 0;
	}

	return ret;
}


/*
 * int task_pop(task_t* element)
 * input: pointer to store the element popped from the queue
 * output: 0 or -1 depending on success or failure respectively
 * description: pop element from the task queue and store its address in the given pointer
 */
int task_pop(task_t* element){
	int ret = -1;

	if(element != NULL && task_q[front].state == RUN_ABLE){	// check input validity
		element->state = 		task_q[front].state;			// copy struct to element
		element->pcb = 			task_q[front].pcb;
		element->esp = 			task_q[front].esp;
		element->ebp = 			task_q[front].ebp;
		element->terminal_num = task_q[front].terminal_num;

		clear_struct(&task_q[front]);						// clear the front struct
		front++;												// increment front index
		front %= SIZE_OF_QUEUE;
		ret = 0;
	}

	return ret;
}

