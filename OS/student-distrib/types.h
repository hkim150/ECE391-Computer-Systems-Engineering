/* types.h - Defines to use the familiar explicitly-sized types in this
 * OS (uint32_t, int8_t, etc.).  This is necessary because we don't want
 * to include <stdint.h> when building this OS
 * vim:ts=4 noexpandtab
 */

#ifndef _TYPES_H
#define _TYPES_H



#define NULL 0
#define FILENAME_SIZE 		32
#define DENTRY_RESERVED 	6
#define BOOT_BLOCK_RESERVED 13
#define NUM_DENTRIES 		63
#define NUM_DATA_BLOCK 		1023
#define FOURKB 4096
#define MAX_FD 				8
#define STR_MAX 			128


#ifndef ASM

/* Types defined here just like in <stdint.h> */
typedef int int32_t;
typedef unsigned int uint32_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef char int8_t;
typedef unsigned char uint8_t;

//structure given in appendix

typedef struct file_op {
	int32_t (*open)(const uint8_t* filename);
	int32_t (*close)(int32_t fd);
	int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
	int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
} file_op_t;

typedef struct file {
	file_op_t *file_op;				
	uint32_t inode;					
	uint32_t file_position;			
	uint32_t flag;					
} file_t;

// 64 byte directory entry 
typedef struct dentry {
	char filename[FILENAME_SIZE];	
	uint32_t file_type;				
	uint32_t inode;					
	uint32_t reserved[DENTRY_RESERVED];			
} dentry_t;

// 4kB boot block 
typedef struct boot_block {
	uint32_t dentries_numbers;					
	uint32_t inodes_numbers;					
	uint32_t num_data_blocks;				
	uint32_t reserved[BOOT_BLOCK_RESERVED];	
	dentry_t dentry[NUM_DENTRIES];			
} boot_block_t;

// 4kB inode 
typedef struct inode {
	uint32_t data;						
	uint32_t data_block[NUM_DATA_BLOCK];	
} inode_t;


typedef struct data_block {
	union {
		dentry_t dentry[NUM_DENTRIES+1];	
		uint8_t data[FOURKB];			
	};
} data_block_t;

typedef struct block {
	union {
		boot_block_t boot_block;	
		inode_t inode;				
		data_block_t data_block;	
	};
} block_t;

typedef struct pcb {
	uint32_t pid;				
	uint32_t page_dir;		
	struct pcb * parent;		
	uint32_t p_ebp;			
	uint32_t p_esp;				
	uint8_t status;				
	uint32_t priority;			
	file_t file_array[MAX_FD];	
	uint8_t arg[STR_MAX];
} pcb_t;

#endif /* ASM */

#endif /* _TYPES_H */
