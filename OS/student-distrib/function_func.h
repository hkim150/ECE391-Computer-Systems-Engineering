#ifndef FUNCTION_FUNC
#define FUNCTION_FUNC

extern void divide_error_func();
extern void page_fault_func();
extern void rtc_func();
extern void keyboard_func();
extern void syscall_linkage();

extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open(const uint8_t* filename);
extern int32_t close(int32_t fd);
extern int32_t execute(const uint8_t* command);
extern int32_t halt(uint8_t status);
extern int32_t getargs(uint8_t* buf, int32_t nbytes);
extern int32_t vidmap(uint8_t** screen_start);

#endif
