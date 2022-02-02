.section .text
.globl _start
_start:

la sp, INITIAL_STACK
la t0, 16384
add sp, sp, t0
la s0, 0
jal kernel_main

endless_loop:
nop
jal endless_loop
