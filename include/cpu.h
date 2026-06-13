#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

#include "memory.h"

typedef enum {
    CSR_MSTATUS = 0x300, //
    CSR_MISA = 0x301, //
    CSR_MIDELEG = 0x303, //
    CSR_MTVEC = 0x305, //
    CSR_MEPC = 0x341, //
    CSR_MCAUSE = 0x342, //
    CSR_MTVAL = 0x343, //
    CSR_MIE = 0x304, //
    CSR_MIP = 0x344, //
    CSR_MEDELEG  = 0x302, //
    CSR_SSTATUS = 0x100, //
    CSR_STVEC = 0x105, //
    CSR_SEPC = 0x141, //
    CSR_SCAUSE = 0x142, //
    CSR_STVAL = 0x143, //
    CSR_SIP = 0x144, 
    CSR_SIE = 0x104, 
    CSR_SATP = 0x180, 
    CSR_CYCLE = 0xc00, //
    CSR_CYCLEH = 0xc80, //
    CSR_TIME = 0xc01, //
    CSR_TIMEH = 0xc80, //
    CSR_INSTRET = 0xc02, //
    CSR_INSTRETH = 0xc82, //
    CSR_FFLAGS = 0x001, //
    CSR_FRM = 0x002, //
    CSR_FCSR = 0x003, //
    CSR_MSTATUSH = 0x310, //
    CSR_MEDELEGH = 0x312, //
    CSR_MSCRATCH = 0x340, //
    CSR_SSCRATCH = 0x140, //
    CSR_MCOUNTEREN = 0x306, //
    CSR_SCOUNTEREN = 0x106, //
    CSR_MHARTID = 0xF14 //
} csr_t;

typedef enum {
    CSR_TYPE_NE,
    CSR_TYPE_RW,
    CSR_TYPE_RO,
    CSR_TYPE_W1C
} csr_type_t;

typedef enum {
    IRQ_S_SOFTWARE     = 1,  // Supervisor software interrupt
    IRQ_M_SOFTWARE     = 3,  // Machine software interrupt
    IRQ_S_TIMER        = 5,  // Supervisor timer interrupt
    IRQ_M_TIMER        = 7,  // Machine timer interrupt
    IRQ_S_EXTERNAL     = 9,  // Supervisor external interrupt
    IRQ_M_EXTERNAL     = 11, // Machine external interrupt

    EXC_INST_MISALIGN  = 0,  // Instruction address misaligned
    EXC_INST_ACCESS    = 1,  // Instruction access fault
    EXC_ILLEGAL_INST   = 2,  // Illegal instruction
    EXC_BREAKPOINT     = 3,  // Breakpoint
    EXC_LOAD_MISALIGN  = 4,  // Load address misaligned
    EXC_LOAD_ACCESS    = 5,  // Load access fault
    EXC_STORE_MISALIGN = 6,  // Store/AMO address misaligned
    EXC_STORE_ACCESS   = 7,  // Store/AMO access fault
    EXC_ECALL_U        = 8,  // Environment call from U-mode
    EXC_ECALL_S        = 9,  // Environment call from S-mode
    EXC_ECALL_M        = 11, // Environment call from M-mode
    EXC_INST_PAGE      = 12, // Instruction page fault
    EXC_LOAD_PAGE      = 13, // Load page fault
    EXC_STORE_PAGE     = 15, // Store/AMO page fault
} trap_cause;

typedef struct {
    uint32_t reg[32];
    uint32_t pc;
    float freg[32];
    uint32_t csr[4096];
    uint8_t csrt[4096];
    memory_t* mem;
    uint8_t mode;
    bool debug;
    bool pause;
} hart;

void init_hart(hart* processor, memory_t* mem, uint32_t resetvector, uint32_t hartid);
void step(hart*processor);
void interrupt(hart* cpu, trap_cause cause);
void clear_interrupt(hart* cpu, trap_cause cause);

#endif