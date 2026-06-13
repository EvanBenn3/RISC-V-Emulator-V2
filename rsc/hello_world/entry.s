.global _start
.text

.equ STACK_BASE, 0x8ffffff0
.equ EXIT_REGISTER, 0x52

_start:
    li sp, STACK_BASE
    li fp, STACK_BASE

    call main

    li t0, EXIT_REGISTER
    li t1, 1
    sb t1, 0(t0)
