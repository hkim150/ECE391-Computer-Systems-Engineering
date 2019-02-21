#include "system_call.h"
#include "lib.h"
#include "x86_desc.h"
#include "paging.h"
#include "function_func.h"

static	char available_pid[SIZE_OF_QUEUE] = {};
static	int task_num = 1;
		int ret_value = 0;

/*
 * int system_open(const unsigned char* filename)
 * input: name of the file
 * output: file descriptor value or -1 depending on success or failure respectively
 * description: activate and initialize the file descriptor 
 */
int system_open(const unsigned char* filename){
	int ret, check;
	unsigned int i;
	dentry_t d;

	ret = -1;
	check = 0;

	if(read_dentry_by_name(filename, &d) != -1){	// get file info
		for(i=0; i<MAX_FD; i++){
			if(file_array[i].flag == 0){
				file_array[i].file_position = 0;	// file info init
				file_array[i].flag = 1;
				switch(d.file_type){				// make file op structure
				case 0:
					file_array[i].file_op = &rtc_file_op;
					file_array[i].inode = NULL;
					break;
				case 1:
					file_array[i].file_op = &directory_op;
					file_array[i].inode = NULL;
					break;
				default:
					file_array[i].file_op = &regular_file_op;
					file_array[i].inode = d.inode;
					break;
				}
				file_array[i].file_op->open(filename);
				ret = i;							// return value becomes file array index
				check = 1;
				break;
			}
		}
		if(check == 0){ret = -1;}					// if file_array is fully occupied, return error
	}

	return ret;
}

/*
 * int system_close(int fd)
 * input: file descriptor value
 * output: 0 or -1 depending on success or failure respectively
 * description: close the file and deactivate the file descriptor
 */
int system_close(int fd){
	int ret = -1;

	if(fd>=2 && fd<MAX_FD && file_array[fd].flag){
		file_array[fd].file_op->close(fd);			// close the file
		file_array[fd].flag = 0;					// clear flags
		file_array[fd].file_op = NULL;				// clear pointers
		ret = 0;
	}

	return ret;
}

/*
 * int system_read(int fd, void* buf, int nbytes)
 * input: file descriptor value, data buffer address, data bytes
 * output: number of bytes or -1 depending on success or failure respectively
 * description: read the file and fill buffer with data
 */
int system_read(int fd, void* buf, int nbytes){
	int ret = -1;
	// call read function and return the value
	if(fd>=0 && fd<MAX_FD && file_array[fd].flag){
		ret = file_array[fd].file_op->read(fd, buf, nbytes);
	}

	return ret;
}

/*
 * int system_write(int fd, const void* buf, int nbytes)
 * input: file descriptor value, data buffer address, data bytes
 * output: number of bytes or -1 depending on success or failure respectively
 * description: write data from buffer to file
 */
int system_write(int fd, const void* buf, int nbytes){
	int ret = -1;
	// call write function and return the value
	if(fd>=0 && fd<MAX_FD && file_array[fd].flag){
		ret = file_array[fd].file_op->write(fd, buf, nbytes);
	}

	return ret;
}

/*
 * int system_execute(const unsigned char* command)
 * input: user program file name and arg string
 * output: 0 or -1 depending on success or failure respectively
 * description: handle system call execution
 */
int system_execute(const unsigned char* command){
	unsigned long flags;
	cli_and_save(flags);

	int i, j;
	char num[NUM_COUNT] = {0x7F, 0x45, 0x4C, 0x46};
	unsigned char fn[STR_MAX];
	unsigned char arg[STR_MAX];

	dentry_t d;

	task_num = next_pid();
	if(task_num == -1){return 1;}					// return if too many tasks
	task_num++;

	for(i=0; i<STR_MAX && command[i] != SPACE_CHAR && command[i] != EMPTY_CHAR && command[i] != '\n'; i++){	// copy command until end of str
		fn[i] = command[i];
	}
	fn[i] = EMPTY_CHAR;
	
	if(command[i] != SPACE_CHAR){arg[0] = EMPTY_CHAR;}		// if no arg, set arg to empty str
	else{
		i++;
		for(j=0; i<STR_MAX && command[i] != EMPTY_CHAR && command[i] != '\n'; j++){	// copy the remaining command str to the arg buffer
			arg[j] = command[i];
			i++;
		}
		arg[j] = EMPTY_CHAR;
	}
	
	if(read_dentry_by_name(fn, &d) == -1){return -1;}		// return error if error occurs while reading

	unsigned char buffer[EXECUTE_HD_BYTE];
	for(i = 0; i < EXECUTE_HD_BYTE; i++){buffer[i] = 0;}	// clear the buffer
	
	// read header
	if(read_data(d.inode, 0, buffer, EXECUTE_HD_BYTE) == -1){return -1;}	// return error if error occurs while reading

	int start_address = buffer[24] | (buffer[25] << 8) | (buffer[26] << 16) | (buffer[27] << 24);	// get starting virtual address

	if(strncmp(num, (char*)buffer, NUM_COUNT)){return -1;}	// check magic #

	int page_index = allocate();							// paging set up

	if(page_index == -1){return -1;}

	map_init(page_index);

	read_data(d.inode, 0, (void*)PROG_ADDRESS, inode_array[d.inode].data);		// get program image copied to the memory

	unsigned int esp;
	asm volatile ("movl %%esp, %0 \n"
	    		  : "=g"(esp)
	    		  :
	    		  : "memory");

	pcb_t* newPCB = (pcb_t*)(EIGHT_MEGA_BYTES - EIGHT_KILO_BYTES*task_num);		// kernel set up and PCB init

	if(base_flag){
		newPCB->parent = (pcb_t*)EIGHT_MEGA_BYTES;
		base_flag = 0;
	}
	else{newPCB->parent = (pcb_t*)(esp & PCB_MASK);}

	file_array = newPCB->file_array;
	newPCB->page_dir = page_index;
	newPCB->pid = task_num;
	newPCB->p_esp = (((unsigned int)newPCB + EIGHT_KILO_BYTES) & PCB_MASK) - FOUR_BYTES;

	strncpy((char*)newPCB->arg, (char*)arg, STR_MAX);

	tss.esp0 = newPCB->p_esp;

	for(i = 0; i < MAX_FD; i++){						// clear file array
		file_array[i].flag = 0;
	}

	file_array[0].flag = 1;
	file_array[0].file_op = &stdin_file_op;
	file_array[0].inode = NULL;
	file_array[0].file_position = 0;

	file_array[1].flag = 1;
	file_array[1].file_op = &stdout_file_op;
	file_array[1].inode = NULL;
	file_array[1].file_position = 0;

	unsigned int currEsp, currEbp;					// get esp and ebp
	asm volatile("movl %%esp, %0 \n"
				"movl %%ebp, %1 \n"
				: "=g"(currEsp), "=g"(currEbp));

	newPCB->p_esp = currEsp;						// restore esp in PCB
	newPCB->p_ebp = currEbp;						// restore ebp in PCB

	asm volatile("movl %2, %%eax \n"
				  "movw %%ax, %%ds \n"
				  "movw %%ax, %%es \n"
				  "movw %%ax, %%fs \n"
				  "movw %%ax, %%gs \n"
				  "pushl %2 \n"
				  "pushl %3 \n"
				  "pushfl \n"
				  "pushl %1 \n"
				  "pushl %0 \n"
				  "iret\n"
				  :
				  : "g"(start_address), "g"(USER_CS_SELECT), "g"(USER_DS_SELECT), "g"(USER_STACK)
				  : "memory", "cc", "sp");

	asm volatile("halting:");
	
	return ret_value;
}

/*
 * int system_halt(unsigned char status)
 * input: process status
 * output: extended value in ebx reg
 * description: handle system call halt
 */
int system_halt(unsigned char status){
	unsigned long flags;
	cli_and_save(flags);
	
	unsigned int esp, ebp, ret;
	ret = 0;

	asm volatile("movl %%esp, %0 \n"
	    		  : "=g"(esp)
	    		  :
	    		  : "memory");

	pcb_t * PCB = (pcb_t*)(esp & PCB_MASK);		// get PCB block address

	available_pid[(PCB->pid) - 1] = 0;

	ret_value = status;

	if(PCB->parent == (pcb_t*)EIGHT_MEGA_BYTES){		// if first process then execute shell prior to halting
		base_flag = 1;
		execute((unsigned char*)"shell");
		ret = 0;
	}
	else{												// parent process
	    tss.esp0 = (unsigned int)PCB->parent->p_esp;
	    tss.ss0 = KERNEL_DS;
		map_init(PCB->parent->page_dir);
		file_array = PCB->parent->file_array;
	    esp = PCB->p_esp;
	    ebp = PCB->p_ebp;

	    asm volatile("movb %%bl, %0 \n"
	    			  "movl %1, %%esp \n"				// update esp
	      			  "movl %2, %%ebp \n"				// update ebp
	      			  "movb %%bl, %%al \n"
	      			  : "=g"(ret)
	      			  : "g"(esp), "g"(ebp)
	      			  : "memory", "cc"
	      			  );
	}

	restore_flags(flags);

	asm volatile("jmp halting");						// jump to halt

	return ret;
}

/*
 * int system_getargs(unsigned char* buf, int nbytes){
 * input: PCB arg holding buffer address, number of bytes of arg
 * output: 0 or -1 depending on success or failure respectively
 * description: handle system call get arg
 */
int system_getargs(unsigned char* buf, int nbytes){
	int i, arg_length, ret;
	unsigned int esp;

	ret = 0;

	if(nbytes > STR_MAX){nbytes = STR_MAX;}			// cap nbytes to maximum length if exceeds

	for(i=0; i<nbytes; i++){				// clear buffer
		buf[i] = EMPTY_CHAR;
	}

	asm volatile ("movl %%esp, %0 \n"		// get esp
				  : "=r"(esp));

	pcb_t* PCB = (pcb_t*)(esp & PCB_MASK);		// get arg str length from PCB
	arg_length = strlen((char*)PCB->arg);

	if(nbytes < arg_length){ret = -1;}				// if nbytes is shorter, return error
    else{strncpy((char*)buf, (char*)PCB->arg, arg_length);}	// copy arg str to buffer

	return ret;
}

/*
 * int system_vidmap(unsigned char** screen_start)
 * input: video memory starting virtual adress
 * output: 0 or -1 depending on success or failure respectively
 * description: handle system call video mapping
 */
int system_vidmap(unsigned char** screen_start){
	int ret = -1;

	// check validity of screen_start address
	if(screen_start != NULL && ((unsigned int)screen_start < PD_SIZE * FOURKB || (unsigned int)screen_start >= 2 * PD_SIZE * FOURKB)){
		page_directory[VIDEO_ADDRESS >> BIT_22] = (unsigned int)page_vid | RWFLAG;
	  	page_vid[(VIDEO_ADDRESS >> BIT_12) & TEN_BIT_MASK] = page_table[VID_MEM+cursor_on_terminal] | RWFLAG;

  		asm volatile("movl %0, %%eax \n"
         	        "movl %%eax, %%cr3 \n"
        	        "movl %%cr4, %%eax \n"
           		    "orl  $0x00000010, %%eax \n"
                	"movl %%eax, %%cr4 \n"
                  	"movl %%cr0, %%eax \n"
                  	"orl  $0x80000001, %%eax \n"
                  	"movl %%eax, %%cr0 \n"
                  	:
                 	: "g"(page_directory)
                 	: "%eax");

	  	*screen_start = (unsigned char*)VIDEO_ADDRESS;
	  	ret = VIDEO_ADDRESS;								// return the new virtual memory
  	}

  	return ret;
}

/*
 * int read_dentry_by_name(const unsigned char* fname, dentry_t* dentry)
 * input: file name, dentry struct
 * output: 0 or -1 depending on success or failure respectively
 * description: find dentry and store in dentry struct
 */
int read_dentry_by_name(const unsigned char* fname, dentry_t* dentry){
	int i, ret;
	ret = -1;

	if(strlen((char*)fname) <= FILENAME_SIZE){									// return error if file is not found
		for(i=0; i<boot_block->dentries_numbers; i++){									// check if the files match
			if(!strncmp((char*)fname, boot_block->dentry[i].filename, 32)){			// if match, copy data
				strcpy((char*)dentry->filename, boot_block->dentry[i].filename);
				dentry->file_type = boot_block->dentry[i].file_type;
				dentry->inode = boot_block->dentry[i].inode;
				ret = 0;
				break;
			}
		}
	}

	return ret;
}

/*
 * int next_pid()
 * input: none
 * output: next pid index or -1 depending on success or failure respectively
 * description: find next unavailable pid and switch it to available
 */
int next_pid(){
	int i;
	int ret = -1;

	for(i=0; i<SIZE_OF_QUEUE; i++){
		if(available_pid[i] != 0){continue;}
		available_pid[i] = 1;
		ret = i;
		break;
	}
	return ret;
}

/*
 * void sched_vid()
 * input: none
 * output: none
 * description: schedule video memory
 */
void sched_vid(){
	page_vid[(VIDEO_ADDRESS >> BIT_12) & TEN_BIT_MASK] = (page_table[VID_MEM + sched_terminal] | RWFLAG);

	asm volatile ("movl %0,					%%eax \n"
                  "movl %%eax,				%%cr3 \n"
                  "movl %%cr4,				%%eax \n"
                  "orl  $0x00000010,		%%eax \n"
                  "movl %%eax,				%%cr4 \n"
                  "movl %%cr0,				%%eax \n"
                  "orl  $0x80000001,		%%eax \n"
                  "movl %%eax,				%%cr0 \n"
                  :
                  : "g"(page_directory)
                  : "%eax"
    );
}

