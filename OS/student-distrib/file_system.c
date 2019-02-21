#include "file_system.h"



file_t* file_array = NULL;

/*
 * void init_file_system(block_t* pointer)
 * input: pointer - pointer to system module
 * output: none
 * initialize basic file system and structs
 */
void init_file_system(block_t* pointer) {
	//initialize stuctures
	block_t* block_array = pointer;
	boot_block = &(block_array[0].boot_block);
	inode_array = &(block_array[1].inode);
	data_block_array = &(block_array[boot_block->inodes_numbers+1].data_block);

	//initialize file system
	rtc_file_op.open = &rtc_open;
	rtc_file_op.close = &rtc_close;
	rtc_file_op.read = &rtc_read;
	rtc_file_op.write = &rtc_write;

	directory_op.open = &file_open;
	directory_op.close = &file_close;
	directory_op.read = &read_directory;
	directory_op.write = &write_directory;

	regular_file_op.open = &file_open;
	regular_file_op.close = &file_close;
	regular_file_op.read = &read_file;
	regular_file_op.write = &write_file;

	stdin_file_op.open = &file_open;
	stdin_file_op.close = &file_close;
	stdin_file_op.write = &invalid_write;
	stdin_file_op.read = &read_from_terminal;

	stdout_file_op.open = &file_open;
	stdout_file_op.close = &file_close;
	stdout_file_op.write = &write_to_terminal;
	stdout_file_op.read = &invalid_read;
}

/*
 * int file_open(const unsigned int* filename)
 * input: file_name - file name to open
 * output: 0 - when file successfully open
 */
int file_open(const unsigned char* file_name) {
	return 0;
}

/*
 * int file_close(int file_descriptor)
 * input: file_descriptor - file descriptor to close
 * output: 0 - when file successfully close
 */
int file_close(int file_descriptor) {
	return 0;
}

/*
 * int read_data(unsigned int inode, unsigned int offset, unsigned char* buf, unsigned int data)
 * input: inode - inode index
 *		  file_index - file data index
 * 		  buf - file data from reading
 *		  data - length of file to read
 * output: returns -1 if invalid, or the total data to read
 */
int read_data(unsigned int inode, unsigned int file_index, unsigned char* buf, unsigned int data){

	int index;
	if (inode > boot_block->inodes_numbers) {
		return -1;						// if inode is invalid it fails to read
	}
	int read_value = 0;
	for (index = 0; index < data; index++) {
		if (file_index+index >= inode_array[inode].data) {
			break;
		}
		buf[index] = data_block_array[inode_array[inode].data_block[(file_index+index)/FOURKB]].data[index+file_index - FOURKB*((file_index+index)/FOURKB)];
		read_value++;
	}
	//stack up the total data to read
	return read_value;
}

/*
 * int write_data(unsigned int inode, unsigned int file_index, const unsigned char* buf, unsigned int data)
 * input: inode - inode index
 *		  file_index - file data index
 * 		  buf - file data from writing
 *		  data - length of file to write
 * output: returns -1 if invlalid, or the total data to write
 */
int write_data(unsigned int inode, unsigned int file_index, const unsigned char* buf, unsigned int data){

	int index;
	if (inode >= boot_block->inodes_numbers) {
		return -1;						// if inode is invalid it fails to write
	}
	int write_value = 0;
	for (index = 0; index < data; index++) {
		if (file_index + index >= inode_array[inode].data) {
			break;
		}
		data_block_array[inode_array[inode].data_block[(file_index + index)/FOURKB]].data[index + file_index - FOURKB*((file_index+index)/FOURKB)] = buf[index];
		write_value++;
	}
	//stack up the total data to write
	return write_value;
}


/*
 * int read_file(int file_descriptor, void* buf, int data_read)
 * input: file_descriptor - file descriptor to read
 * 		  buf - data to read
 * 		  data_read - data amount to read from file
 * output: returns -1 for fail or amount of data to read from the file
 */
int read_file(int file_descriptor, void* buf, int data_read) {
	int read_value = read_data(file_array[file_descriptor].inode, file_array[file_descriptor].file_position, buf, data_read);

	if (file_descriptor < 0 || file_descriptor >= MAX_FD) {
		return -1;			//check validity
	}

	file_array[file_descriptor].file_position = file_array[file_descriptor].file_position + read_value;
	return read_value;
}

/*
 * int write_file(int file_descriptor, const void* buf, int data_write)
 * input: file_descriptor - file descriptor to write
 * 		  buf - data to write
 * 		  data_write - data amount to write
 * output: returns -1 for fail or amount of data to write
 */
int write_file(int file_descriptor, const void* buf, int data_write) {
	int write_value = write_data(file_array[file_descriptor].inode, file_array[file_descriptor].file_position, buf, data_write);

	if (file_descriptor < 0 || file_descriptor >= MAX_FD) {
		return -1;			//check validity
	}

	file_array[file_descriptor].file_position = file_array[file_descriptor].file_position + write_value;
	return write_value;
}


/*
 * int read_directory(int file_descriptor, void* buf, int data_read)
 * input: file_descriptor - file descriptor to read
 * 		  buf - data to read
 * 		  data_read - data amount to read from file
 * output: returns -1 for fail or amount of data to read from the file in directory
 */

int read_directory(int file_descriptor, void* buf, int data_read) {

	int index = file_array[file_descriptor].file_position;
	if (index >= boot_block->dentries_numbers) {
		return 0;
	}

	if (data_read > FILENAME_SIZE) {
		data_read = FILENAME_SIZE;		// cannot read data larger than the maximum filename
	}									// size. Therefore read only up to max

	strncpy(buf,boot_block->dentry[index].filename,data_read);
	file_array[file_descriptor].file_position++;
	
	return strlen(buf);
}

/*
 * int write_directory(int file_descriptor, const void* buf, int nbytes)
 * input: file_descriptor - file descriptor to write
 * 		  buf - data to write
 * 		  data_write - data amount to write
 * output: returns -1 for fail or amount of data to write in directory
 */
int write_directory(int file_descriptor, const void* buf, int data_write) {

	return write_file(file_descriptor, buf, data_write); //use the write_file function
}



/*
 * int read_from_terminal(int file_descriptor, void* buf, int data_read)
 * input: file_descriptor - file descriptor to read in terminal
 * 		  buf - data to read in terminal
 * 		  data_read - data amount to read from terminal
 * output: returns data amount read from terminal
 */
int read_from_terminal(int file_descriptor, void* buf, int data_read) {
	int ter = sched_terminal - 1;

	enter_flag[ter] = 1;
	// when read_from_terminal calls, reset the enter flag to zero
	while (enter_flag[ter]) {
	}
	strcpy((char*)buf,(char*)buffer[ter]);
	int data_amount = 0;
	while (buffer[ter][data_amount] != '\0' && data_amount < data_read) {
		((char*)buf)[data_amount] = ((char*)buffer[ter])[data_amount];
		data_amount++;
	}

	reset_buffer(buffer[ter]);
	buffer_index[ter] = 0;

	if (data_amount < MAX_BUFFER && data_amount < data_read) {
		((char*)buf)[data_amount] = '\n';
		return data_amount + 1;
	}
	return data_amount;
}

/*
 * int terminal_write(int32-t file_descriptor, const void* buf, int nbytes)
 * input: file_descriptor - file descriptor to write
 * 		  buf - data to write in terminal
 * 		  data_write - data amount to write in terminal
 * output: returns data amount write in terminal
 */
int write_to_terminal(int file_descriptor, const void* buf, int data_write) {
	int index;
	for (index = 0; index < data_write; index++) {
	putc(((char*)buf)[index]);
	}
	return data_write;
}

/*
 * int invalid_read(int file_descriptor, void* buf, int nbytes)
 * input: file_descriptor - file descriptor to write / read
 * 		  buf - data to write / read in terminal
 * 		  data_write - data amount to write / read in terminal
 * output: if terminal reads while it is in stdout, it is invalid
 */
int invalid_read(int file_descriptor, void* buf, int nbytes) {
	return -1;
}

int invalid_write(int file_descriptor, const void* buf, int nbytes) {
	return -1;
}

