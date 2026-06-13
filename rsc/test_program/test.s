.global _start
.text

.equ EXIT_REGISTER, 0x52
.equ STACK_BASE, 0x8ffffff0

_start:
    li sp, STACK_BASE
    
    li a0, 20
    call square

_exit:
    ebreak
    li t0, EXIT_REGISTER
    li t1, 1
    sb t1, 0(t0)

square:
    mv t0, a0
    mul a0, t0, t0
    mulh a1, t0, t0
    ret

