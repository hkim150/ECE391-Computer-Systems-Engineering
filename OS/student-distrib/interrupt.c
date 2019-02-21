#include "interrupt.h"

#define SYSTEM 0x80
#define KEYBOARD 0x21
#define REAL_TIME_CLOCK 0x28

/*
 * void idt_setup(void)
 * input: none
 * output: none
 * result : set up interrupt description table
 */
void idt_setup() {
	set_trap_gate(0,&divide_error_func);
	set_trap_gate(1,&debug_exception);
	set_intr_gate(2,&nmi_interrupt);
	set_system_gate(3,&breakpoint_exception);
	set_system_gate(4,&overflow_exception);
	set_system_gate(5,&bound_exception);
	set_trap_gate(6,&invalid_opcode_exception);
	set_trap_gate(7,&device_not_available_exception);
	set_trap_gate(8,&double_fault_exception);
	set_trap_gate(9,&coprocessor_segment_overrun);
	set_trap_gate(10,&invalid_TSS_exception);
	set_trap_gate(11,&segment_not_present);
	set_trap_gate(12,&stack_fault_exception);
	set_trap_gate(13,&general_protection_exception);
	set_intr_gate(14,&page_fault_func);
	set_trap_gate(16,&floating_point_error);
	set_trap_gate(17,&alignment_check_exception);
	set_trap_gate(18,&machine_check_exception);
	set_trap_gate(19,&simd_coprocessor_error);
	set_intr_gate(KEYBOARD,&keyboard_func); 
	set_intr_gate(REAL_TIME_CLOCK,&rtc_func); 	 
	set_system_gate(SYSTEM,&syscall_linkage); 
}

/*
 * void idt_insert(unsigned char idx, int type, unsigned char dpl, void *handler)
 * input: unsigned char idx : interrupt description table index
					unsigned char dpl : Descriptor privilege level
					void* handler : pointer to handler
 * output: none
 */
void idt_insert(unsigned char idx, int type, unsigned char dpl, void *handler) {
	SET_IDT_ENTRY(idt[idx],handler);
	idt[idx].seg_selector = KERNEL_CS;
	idt[idx].dpl = dpl;
	idt[idx].reserved4 = 0;
	idt[idx].reserved3 = type;
	idt[idx].reserved2 = 1;
	idt[idx].reserved1 = 1;
	idt[idx].size = 1;
	idt[idx].reserved0 = 0;
	idt[idx].present = 1;
}

/*
 * void set_trap_gate(unsigned char idx, void *handler)
 * input: unsigned char idx : index in interrupt description table
					void* handler : pointer to handler
 * output: none
 * result : fill entry for each type
 */
void set_trap_gate(unsigned char idx, void *handler) {
	idt_insert(idx, 1, 0, handler);
}

/*
 * void set_intr_gate(unsigned char idx, void *handler)
 * input: unsigned char idx : index in interrupt description table
					void* handler : pointer to handler
 * output: none
 */
void set_intr_gate(unsigned char idx, void *handler) {
	idt_insert(idx, 0, 0, handler);
}

/*
 * void set_system_gate(unsigned char idx, void *handler)
 * input: unsigned char idx : index in interrupt description table
					void* handler : pointer to handler
 * output: none
 */
void set_system_gate(unsigned char idx, void *handler) {
	idt_insert(idx, 0, 3, handler);
}
