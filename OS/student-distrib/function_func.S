#define ASM 1
#include "x86_desc.h"

.globl divide_error_func, page_fault_func, keyboard_func, rtc_func, task_switch_done, syscall_linkage
.globl read, write, open, close, execute, halt, getargs, vidmap


divide_error_func:
    # push registers on stack
    pushal
    pushl   %es
    pushl   %ds
    # call handler function
    call    divide_error_exception
    # pop registers from stack
    popl    %ds
    popl    %es
    popal
    iret


page_fault_func:
    # push registers on stack
    pushal
    pushl   %es
    pushl   %ds
    # call handler function
    call    page_fault_exception
    # pop registers from stack
    popl    %ds
    popl    %es
    popal
    iret


keyboard_func:
    # push registers on stack
    pushal
    pushl   %es
    pushl   %ds
    # call handler function
    call    keyboard_handler
    # pop registers from stack
    popl    %ds
    popl    %es
    popal
    iret


rtc_func:
    # push registers on stack
    pushal
    pushl   %es
    pushl   %ds
    # call handler function
    call    rtc_interrupt_handler
    # pop registers from stack
    popl    %ds
    popl    %es
    popal
    iret

  task_switch_done:
    popw    %ds
    popw    %es
    popw    %fs
    popal
    sti
    iret

syscall_linkage:
    cmpl $1, %eax
    jl invalid_input
    cmpl $10, %eax
    jg invalid_input

    pushl %esi
    pushl %edi
    pushl %ebx

    pushw %fs
    pushw %es
    pushw %ds
    pushfl

    pushl %edx
    pushl %ecx
    pushl %ebx

    sti

    call *jump_table(, %eax, 4)

    cli

    addl $12, %esp

    popfl
    popw %ds
    popw %es
    popw %fs

    popl %ebx
    popl %edi
    popl %esi

    iret

invalid_input:
    movl $-1, %eax
    iret

# x86 code given in course note

halt:
    pushl   %ebx
    movl    $0x01,%eax
    movl    0x08(%esp),%ebx
    movl    0x0C(%esp),%ecx
    movl    0x10(%esp),%edx
    int     $0x80
    popl    %ebx
    ret

execute:
    pushl   %ebx
    movl    $0x02,%eax
    movl    0x08(%esp),%ebx
    movl    0x0C(%esp),%ecx
    movl    0x10(%esp),%edx
    int     $0x80
    popl    %ebx
    ret

read:
    pushl   %ebx
    movl    $0x03,%eax
    movl    0x08(%esp),%ebx
    movl    0x0C(%esp),%ecx
    movl    0x10(%esp),%edx
    int     $0x80
    popl    %ebx
    ret

write:
    pushl   %ebx
    movl    $0x04,%eax
    movl    0x08(%esp),%ebx
    movl    0x0C(%esp),%ecx
    movl    0x10(%esp),%edx
    int     $0x80
    popl    %ebx
    ret

open:
    pushl   %ebx
    movl    $0x05,%eax
    movl    0x08(%esp),%ebx
    movl    0x0C(%esp),%ecx
    movl    0x10(%esp),%edx
    int     $0x80
    popl    %ebx
    ret

close:
    pushl   %ebx
    movl    $0x06,%eax
    movl    0x08(%esp),%ebx
    movl    0x0C(%esp),%ecx
    movl    0x10(%esp),%edx
    int     $0x80
    popl    %ebx
    ret

getargs:
    pushl   %ebx
    movl    $0x07,%eax
    movl    0x08(%esp),%ebx
    movl    0x0C(%esp),%ecx
    movl    0x10(%esp),%edx
    int     $0x80
    popl    %ebx
    ret

vidmap:
    pushl   %ebx
    movl    $0x08,%eax
    movl    0x08(%esp),%ebx
    movl    0x0C(%esp),%ecx
    movl    0x10(%esp),%edx
    int     $0x80
    popl    %ebx
    ret




jump_table:
    .long 0, system_halt, system_execute, system_read, system_write, system_open, system_close
    .long system_getargs, system_vidmap, 0, 0
