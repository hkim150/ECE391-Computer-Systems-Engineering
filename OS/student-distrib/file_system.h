#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "types.h"
#include "lib.h"
#include "keyboard.h"
#include "system_call.h"
#include "rtc.h"
//#include "scheduling.h"


extern void init_file_system(block_t* pointer);

int read_data(unsigned int inode, unsigned int file_index, unsigned char* buf, unsigned int data);
int write_data(unsigned int inode, unsigned int file_index, const unsigned char* buf, unsigned int data);

int file_open(const unsigned char* filename);
int file_close(int file_descriptor);

int read_file(int file_descriptor, void* buf, int data_read);
int write_file(int file_descriptor, const void* buf, int data_write);

int read_directory(int file_descriptor, void* buf, int data_read);
int write_directory(int file_descriptor, const void* buf, int data_write);

int read_from_terminal(int file_descriptor, void* buf, int data_read);
int write_to_terminal(int file_descriptor, const void* buf, int data_write);

int invalid_read(int file_descriptor, void* buf, int nbytes);
int invalid_write(int file_descriptor, const void* buf, int nbytes);

file_op_t rtc_file_op;
file_op_t directory_op;
file_op_t regular_file_op;
file_op_t stdin_file_op;
file_op_t stdout_file_op;
file_op_t terminal_file_op;

boot_block_t* boot_block;
block_t* block_array;
data_block_t* data_block_array;
inode_t* inode_array;
file_t* file_array;

#endif
