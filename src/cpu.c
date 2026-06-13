#include "../include/cpu.h"
#include "../include/memory.h"

#include <stdio.h>
#include <string.h>

void init_hart(hart* processor, memory_t* mem, uint32_t resetvector, uint32_t hartid) {
    processor->pc = resetvector;
    processor->mem = mem;
    processor->debug = false;
    processor->pause = false;
    processor->mode = 3;
    memset(processor->csr, 0, sizeof(processor->csr));
    processor->csr[CSR_MISA] = 0x40011220;
    processor->csr[CSR_MSTATUS] = 0x00001848;
    processor->csr[CSR_MHARTID] = hartid;
    for (int i = 0; i < 4096; i++) {
        processor->csrt[i] = CSR_TYPE_NE;
    }
    processor->csrt[CSR_MSTATUS] = CSR_TYPE_RW;
    processor->csrt[CSR_MISA] = CSR_TYPE_RO;
    processor->csrt[CSR_MIDELEG] = CSR_TYPE_RW;
    processor->csrt[CSR_MTVEC] = CSR_TYPE_RW;
    processor->csrt[CSR_MEPC] = CSR_TYPE_RW;
    processor->csrt[CSR_MCAUSE] = CSR_TYPE_RW;
    processor->csrt[CSR_MTVAL] = CSR_TYPE_RW;
    processor->csrt[CSR_MIE] = CSR_TYPE_RW;
    processor->csrt[CSR_MIP] = CSR_TYPE_W1C;
    processor->csrt[CSR_MEDELEG] = CSR_TYPE_RW;
    processor->csrt[CSR_SSTATUS] = CSR_TYPE_RW;
    processor->csrt[CSR_STVEC] = CSR_TYPE_RW;
    processor->csrt[CSR_SEPC] = CSR_TYPE_RW;
    processor->csrt[CSR_SCAUSE] = CSR_TYPE_RW;
    processor->csrt[CSR_STVAL] = CSR_TYPE_RW;
    processor->csrt[CSR_SIP] = CSR_TYPE_RO;
    processor->csrt[CSR_SIE] = CSR_TYPE_RW;
    processor->csrt[CSR_SATP] = CSR_TYPE_RW;
    processor->csrt[CSR_CYCLE] = CSR_TYPE_RO;
    processor->csrt[CSR_TIME] = CSR_TYPE_RO;
    processor->csrt[CSR_INSTRET] = CSR_TYPE_RO;
    processor->csrt[CSR_FFLAGS] = CSR_TYPE_W1C;
    processor->csrt[CSR_FRM] = CSR_TYPE_RW;
    processor->csrt[CSR_FCSR] = CSR_TYPE_RW;
    processor->csrt[CSR_CYCLEH] = CSR_TYPE_RO;
    processor->csrt[CSR_TIMEH] = CSR_TYPE_RO;
    processor->csrt[CSR_INSTRETH] = CSR_TYPE_RO;
    processor->csrt[CSR_MSTATUSH] = CSR_TYPE_RO; //little endian is always on
    processor->csrt[CSR_MEDELEGH] = CSR_TYPE_RO; //hardwired to 0
    processor->csrt[CSR_MSCRATCH] = CSR_TYPE_RW;
    processor->csrt[CSR_SSCRATCH] = CSR_TYPE_RW;
    processor->csrt[CSR_MCOUNTEREN] = CSR_TYPE_RW;
    processor->csrt[CSR_SCOUNTEREN] = CSR_TYPE_RW;
    processor->csrt[CSR_MHARTID] = CSR_TYPE_RO;
}

bool is_legal_access(hart* cpu, csr_t csr_address) {
    if (cpu->csrt[csr_address] == CSR_TYPE_NE) return false;

    if (((csr_address >> 8) & 0x3) > cpu->mode) return false;

    return true;
}

void write_csr(hart* cpu, csr_t csr_address, uint32_t value) {
    if (cpu->csrt[csr_address] == CSR_TYPE_RO) return;
    switch (csr_address) {
        case CSR_FFLAGS:
            cpu->csr[CSR_FCSR] &= ~(value & 0x1f);
        break;
        case CSR_FRM:
            if (((value >> 5) & 0x7) <= 4) {
                cpu->csr[CSR_FCSR] &= ~(0x7 << 5);
                cpu->csr[CSR_FCSR] |= (((value >> 5) & 0x7) << 5);
            } else {
                cpu->csr[CSR_FCSR] &= ~(0x7 << 5); // sets to 0
            }
        break;
        case CSR_FCSR:
            write_csr(cpu, CSR_FFLAGS, value);
            write_csr(cpu, CSR_FRM, value);
        break;
        case CSR_MSTATUS:
            cpu->csr[CSR_MSTATUS] = value & 0x000773AA;
            cpu->csr[CSR_MSTATUS] &= ~(1 << 31);
            cpu->csr[CSR_MSTATUS] |= ((cpu->csr[CSR_MSTATUS] >> 13) & 0x3) == 3 ? (1 << 31) : 0;
        break;
        case CSR_MIDELEG:
            cpu->csr[CSR_MIDELEG] = value & 0x222;
        break;
        case CSR_MTVEC:
            if ((value & 0x3) >= 2) value &= ~0x3;
            cpu->csr[CSR_MTVEC] = value;
        break;
        case CSR_STVEC:
            if ((value & 0x3) >= 2) value &= ~0x3;
            cpu->csr[CSR_STVEC] = value;
        break;
        case CSR_SSTATUS:
            cpu->csr[CSR_MSTATUS] = (cpu->csr[CSR_MSTATUS] & ~0x000C61E2) | (value & 0x000C61E2);
            cpu->csr[CSR_MSTATUS] &= ~(1 << 31);
            cpu->csr[CSR_MSTATUS] |= ((cpu->csr[CSR_MSTATUS] >> 13) & 0x3) == 3 ? (1 << 31) : 0;
        break;
        case CSR_MEDELEG:
            cpu->csr[CSR_MEDELEG] = value & 0x0000B3FF;
        break;
        case CSR_MIE:
            cpu->csr[CSR_MIE] = value & 0xAAA;
        break;
        case CSR_MIP:
            cpu->csr[CSR_MIP] &= ~(value & 0xa);
        break;
        case CSR_SATP:
            //hardwired to 0, here for future implementation
        break;
        case CSR_SIP:
            cpu->csr[CSR_MIP] &= ~(value & 0x2);
        break;
        case CSR_SIE:
            cpu->csr[CSR_MIE] &= ~0x222;
            cpu->csr[CSR_MIE] |= value & 0x222;
        break;
        default:
            cpu->csr[csr_address] = value;
        break;
    }
    return;
}

uint32_t read_csr(hart* cpu, csr_t csr_address) {
    switch (csr_address) {
        case CSR_SSTATUS:
            return cpu->csr[CSR_MSTATUS] & 0x000C61E2;
        break;
        case CSR_SIP:
            return cpu->csr[CSR_MIP] & cpu->csr[CSR_MIDELEG];
        break;
        case CSR_SIE:
            return cpu->csr[CSR_MIE] & cpu->csr[CSR_MIDELEG];
        break;
        default:
            return cpu->csr[csr_address];
        break;
    }
}

void trap_handler(hart* cpu, trap_cause cause, uint32_t trap_value) {
    bool is_interrupt = (cause >> 31) & 1;
    bool deleg;
    if (is_interrupt) {
        deleg = (1 << (cause & ~(1 << 31))) & cpu->csr[CSR_MIDELEG];
    } else {
        deleg = (1 << (cause & ~(1 << 31))) & cpu->csr[CSR_MEDELEG];
    }

    bool mie = (cpu->csr[CSR_MSTATUS] >> 3) & 1;
    bool sie = (cpu->csr[CSR_MSTATUS] >> 1) & 1;
    bool vector;
    if (!deleg) {
        cpu->csr[CSR_MEPC] = cpu->pc;
        cpu->csr[CSR_MCAUSE] = cause;
        cpu->csr[CSR_MTVAL] = trap_value;
        cpu->csr[CSR_MSTATUS] &= ~0x1888;
        cpu->csr[CSR_MSTATUS] |= (mie << 7) | (cpu->mode << 11);
        vector = (cpu->csr[CSR_MTVEC] & 0x3);
        cpu->mode = 3;
        if (!vector || !is_interrupt) {
            cpu->pc = cpu->csr[CSR_MTVEC] & ~0x3 - 4;
        } else if (is_interrupt) {
            cpu->pc = (cpu->csr[CSR_MTVEC] & ~0x3) + 4*(cause & ~(1 << 31)) - 4;
        }
    } else {
        cpu->csr[CSR_SEPC] = cpu->pc;
        cpu->csr[CSR_SCAUSE] = cause;
        cpu->csr[CSR_STVAL] = trap_value;
        cpu->csr[CSR_MSTATUS] &= ~0x122;
        cpu->csr[CSR_MSTATUS] |= (sie << 5) | (cpu->mode << 8);
        vector = (cpu->csr[CSR_STVEC] & 0x3);
        cpu->mode = 1;
        if (!vector || !is_interrupt) {
            cpu->pc = cpu->csr[CSR_STVEC] & ~0x3 - 4;
        } else if (is_interrupt) {
            cpu->pc = (cpu->csr[CSR_STVEC] & ~0x3) + 4*(cause & ~(1 << 31)) - 4;
        }
    }
}

void systemtype(hart* cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1f;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1f;
    uint32_t csr = instruction >> 20;
    uint32_t old;

    if (!is_legal_access(cpu, csr)) {
        trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
        return;
    }
    switch (funct3) {
        case 1:
            cpu->reg[rd] = read_csr(cpu, csr);
            write_csr(cpu, csr, cpu->reg[rs1]);
        break;
        case 2:
            old = read_csr(cpu, csr);
            cpu->reg[rd] = old;
            if (rs1 != 0) {
                write_csr(cpu, csr, old | cpu->reg[rs1]);
            }
        break;
        case 3:
            old = read_csr(cpu, csr);
            cpu->reg[rd] = old;
            if (rs1 != 0) {
                write_csr(cpu, csr, old & ~cpu->reg[rs1]);
            }
        break;
        case 5:
            cpu->reg[rd] = read_csr(cpu, csr);
            write_csr(cpu, csr, rs1);
        break;
        case 6:
            old = read_csr(cpu, csr);
            cpu->reg[rd] = old;
            if (rs1 != 0) {
                write_csr(cpu, csr, old | rs1);
            }
        break;
        case 7:
            old = read_csr(cpu, csr);
            cpu->reg[rd] = old;
            if (rs1 != 0) {
                write_csr(cpu, csr, old & ~rs1);
            }
        break;
        default:
            trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
        break;
    }
}


void rtype(hart* cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1f;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1f;
    uint32_t rs2 = (instruction >> 20) & 0x1f;
    uint32_t funct7 = instruction >> 25;

    switch (funct3) {
        case 0:
            if (funct7 == 0x0) { // add
                cpu->reg[rd] = cpu->reg[rs1] + cpu->reg[rs2];
            } else if (funct7 == 0x20) { // sub
                cpu->reg[rd] = cpu->reg[rs1] - cpu->reg[rs2];
            } else if (funct7 == 0x1) { // mul
                cpu->reg[rd] = (uint32_t)((int64_t)cpu->reg[rs1] * (int64_t)cpu->reg[rs2]);
            } else {
                trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
            }
        break;
        case 1:
            if (funct7 == 0x0) { // sll
                cpu->reg[rd] = cpu->reg[rs1] << (cpu->reg[rs2] & 0x1f);
            } else if (funct7 == 0x1) { // mulh
                cpu->reg[rd] = (uint32_t)((uint64_t)((int64_t)cpu->reg[rs1] * (int64_t)cpu->reg[rs2]) >> 32);
            } else {
                trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
            }
        break;
        case 2:
            if (funct7 == 0x0) { // slt
                cpu->reg[rd] = (int32_t)cpu->reg[rs1] < (int32_t)cpu->reg[rs2];
            } else if (funct7 == 0x1) { // mulhsu
                cpu->reg[rd] = (uint32_t)((uint64_t)((int64_t)cpu->reg[rs1] * (uint64_t)cpu->reg[rs2]) >> 32);
            } else {
                trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
            }
        break;
        case 3:
            if (funct7 == 0x0) { // sltu
                cpu->reg[rd] = cpu->reg[rs1] < cpu->reg[rs2];
            } else if (funct7 == 0x1) { // mulhu
                cpu->reg[rd] = (uint32_t)(((uint64_t)cpu->reg[rs1] * (uint64_t)cpu->reg[rs2]) >> 32);
            } else {
                trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
            }
        break;
        case 4:
            if (funct7 == 0x0) { // xor
                cpu->reg[rd] = cpu->reg[rs1] ^ cpu->reg[rs2];
            } else if (funct7 == 0x1) { // div
                if (cpu->reg[rs2] == 0) {
                    cpu->reg[rd] = -1;
                } else {
                    cpu->reg[rd] = (int32_t)cpu->reg[rs1] / (int32_t)cpu->reg[rs2];
                }
            } else {
                trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
            }
        break;
        case 5:
            if (funct7 == 0x0) { // srl
                cpu->reg[rd] = cpu->reg[rs1] >> cpu->reg[rs2];
            } else if (funct7 == 0x20) {
                cpu->reg[rd] = (int32_t)cpu->reg[rs1] >> cpu->reg[rs2]; // sra, MinGW assumes SRA with signed integers
            } else if (funct7 == 0x1) { //divu
                if (cpu->reg[rs2] == 0) {
                    cpu->reg[rd] = -1;
                } else {
                    cpu->reg[rd] = cpu->reg[rs1] / cpu->reg[rs2];
                }
            } else {
                trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
            }
        break;
        case 6:
            if (funct7 == 0x0) { // or
                cpu->reg[rd] = cpu->reg[rs1] | cpu->reg[rs2];
            } else if (funct7 == 0x1) { // rem
                cpu->reg[rd] = (int32_t)cpu->reg[rs1] % (int32_t)cpu->reg[rs2];
            } else {
                trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
            }
        break;
        case 7:
            if (funct7 == 0x0) { // and
                cpu->reg[rd] = cpu->reg[rs1] & cpu->reg[rs2];
            } else if (funct7 == 0x1) { // remu
                cpu->reg[rd] = cpu->reg[rs1] % cpu->reg[rs2];
            } else {
                trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
            }
        break;
    }
}

void itype(hart* cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1f;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1f;
    uint32_t imm12 = instruction >> 20;
    if (imm12 & 0x800) {
        imm12 |= 0xfffff000;
    }
    uint32_t shamt = (instruction >> 20) & 0x1f;
    uint32_t shtyp = instruction >> 25;

    switch (funct3) {
        case 0:
            cpu->reg[rd] = cpu->reg[rs1] + imm12;
        break;
        case 1:
            if (shtyp == 0x0) {
                cpu->reg[rd] = cpu->reg[rs1] << shamt;
            } else {
                trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
            }
        break;
        case 2:
            cpu->reg[rd] = (int32_t)cpu->reg[rs1] < (int32_t)imm12;
        break;
        case 3:
            cpu->reg[rd] = cpu->reg[rs1] < imm12;
        break;
        case 4:
            cpu->reg[rd] = cpu->reg[rs1] ^ imm12;
        break;
        case 5:
            if (shtyp == 0x0) {
                cpu->reg[rd] = cpu->reg[rs1] >> shamt;
            } else if (shtyp == 0x20) {
                cpu->reg[rd] = (int32_t)cpu->reg[rs1] >> shamt;
            } else {
                trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
            }
        break;
        case 6:
            cpu->reg[rd] = cpu->reg[rs1] | imm12;
        break;
        case 7:
            cpu->reg[rd] = cpu->reg[rs1] & imm12;
        break;
    }
}

void itypemem(hart* cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1f;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1f;
    uint32_t imm12 = instruction >> 20;
    if (imm12 & 0x800) {
        imm12 |= 0xfffff000;
    }
    uint32_t address = cpu->reg[rs1] + imm12;
    uint32_t data;
    int err;
    if (!address) {
        trap_handler(cpu, EXC_LOAD_ACCESS, address);
        return;
    }
    switch (funct3) {
        case 0:
            err = read_byte(cpu->mem, address, (uint8_t*)&data);
            if (err == 1) {trap_handler(cpu, EXC_LOAD_ACCESS, address); return;}
            if (data & 0x80) {
                data |= 0xffffff00;
            }
            cpu->reg[rd] = data;
        break;
        case 1:
            err = read_halfword(cpu->mem, address, (uint16_t*)&data);
            if (err == 1) {trap_handler(cpu, EXC_LOAD_ACCESS, address); return;}
            if (err == 2) {trap_handler(cpu, EXC_LOAD_MISALIGN, address); return;}
            if (data & 0x8000) {
                data |= 0xffff0000;
            }
            cpu->reg[rd] = data;
        break;
        case 2:
            err = read_word(cpu->mem, address, &data);
            if (err == 1) {trap_handler(cpu, EXC_LOAD_ACCESS, address); return;}
            if (err == 2) {trap_handler(cpu, EXC_LOAD_MISALIGN, address); return;}
            cpu->reg[rd] = data;
        break;
        case 4:
            err = read_byte(cpu->mem, address, (uint8_t*)&data);
            if (err == 1) {trap_handler(cpu, EXC_LOAD_ACCESS, address); return;}
            cpu->reg[rd] = data;
        break;
        case 5:
            err = read_halfword(cpu->mem, address, (uint16_t*)&data);
            if (err == 1) {trap_handler(cpu, EXC_LOAD_ACCESS, address); return;}
            if (err == 2) {trap_handler(cpu, EXC_LOAD_MISALIGN, address); return;}
            cpu->reg[rd] = data;
        break;
        default:
            trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
        break;
    }
}

void stype(hart* cpu, uint32_t instruction) {
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1f;
    uint32_t rs2 = (instruction >> 20) & 0x1f;
    uint32_t imm12 = ((instruction >> 7) & 0x1f) | ((instruction >> 25) << 5);
    if (imm12 & 0x800) {
        imm12 |= 0xfffff000;
    }
    uint32_t address = cpu->reg[rs1] + imm12;
    int err;
    if (!address) {
        trap_handler(cpu, EXC_STORE_ACCESS, address);
        return;
    }
    switch (funct3) {
        case 0:
            err = write_byte(cpu->mem, address, cpu->reg[rs2] & 0xff);
            if (err == 1) {trap_handler(cpu, EXC_STORE_ACCESS, address); return;}
        break;
        case 1:
            err = write_halfword(cpu->mem, address, cpu->reg[rs2] & 0xffff);
            if (err == 1) {trap_handler(cpu, EXC_STORE_ACCESS, address); return;}
            if (err == 2) {trap_handler(cpu, EXC_STORE_MISALIGN, address); return;}
        break;
        case 2:
            err = write_word(cpu->mem, address, cpu->reg[rs2]);
            if (err == 1) {trap_handler(cpu, EXC_STORE_ACCESS, address); return;}
            if (err == 2) {trap_handler(cpu, EXC_STORE_MISALIGN, address); return;}
        break;
        default:
            trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
        break;
    }
}

void utype(hart* cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1f;
    uint32_t imm20 = (instruction & 0xfffff000);
    cpu->reg[rd] = imm20;
}

void utypepc(hart* cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1f;
    uint32_t imm20 = (instruction & 0xfffff000);
    cpu->reg[rd] = imm20 + cpu->pc;
}

void jtype(hart* cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1f;
    uint32_t imm20 = (((instruction >> 21) & 0x3ff) << 1)
                    | (((instruction >> 20) & 1) << 11)
                    | (((instruction >> 12) & 0xff) << 12)
                    | ((instruction >> 31) << 20);
    if (imm20 & 0x10000) {
        imm20 |= 0xffe00000;
    }
    
    cpu->reg[rd] = cpu->pc + 4;
    cpu->pc += imm20 - 4;
}

void itypej(hart* cpu, uint32_t instruction) {
    uint32_t rd = (instruction >> 7) & 0x1f;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1f;
    uint32_t imm12 = instruction >> 20;
    if (imm12 & 0x800) {
        imm12 |= 0xfffff000;
    }

    if (funct3 == 0) {
        uint32_t target = cpu->reg[rs1] + imm12;
        cpu->reg[rd] = cpu->pc + 4;
        cpu->pc = target - 4;
    } else {
        trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
    }
}

void btype(hart* cpu, uint32_t instruction) {
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1f;
    uint32_t rs2 = (instruction >> 20) & 0x1f;
    uint32_t imm12 = (((instruction >> 8) & 0xf) << 1)
                    | (((instruction >> 25) & 0x3f) << 5)
                    | (((instruction >> 7) & 1) << 11)
                    | (((instruction >> 31) & 1) << 12);
    if (imm12 & 0x800) {
        imm12 |= 0xfffff000;
    }
    uint32_t rs1v = cpu->reg[rs1];
    uint32_t rs2v = cpu->reg[rs2];
    uint32_t target = cpu->pc + imm12;

    switch (funct3) {
        case 0:
            if (rs1v == rs2v) {
                cpu->pc = target - 4;
            }
        break;
        case 1:
            if (rs1v != rs2v) {
                cpu->pc = target - 4;
            }
        break;
        case 4:
            if ((int32_t)rs1v < (int32_t)rs2v) {
                cpu->pc = target - 4;
            }
        break;
        case 5:
            if ((int32_t)rs1v >= (int32_t)rs2v) {
                cpu->pc = target - 4;
            }
        break;
        case 6:
            if (rs1v < rs2v) {
                cpu->pc = target - 4;
            }
        break;
        case 7:
            if (rs1v >= rs2v) {
                cpu->pc = target - 4;
            }
        break;
        default:
            trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
        break;
    }
}

void ecall(hart* cpu, uint32_t instruction) {
    if (cpu->mode == 3) {
        trap_handler(cpu, EXC_ECALL_M, 0);
    } else if (cpu->mode == 1) {
        trap_handler(cpu, EXC_ECALL_S, 0);
    } else if (cpu->mode == 0) {
        trap_handler(cpu, EXC_ECALL_U, 0);
    }
}

void ebreak(hart* cpu, uint32_t instruction) {
    cpu->debug = true;
}

void fence(hart*cpu, uint32_t instruction) {
    if (instruction & 0xf00ffff0) {
        trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
    }
}

void debug_handler(hart* cpu) {
    char command[32];
    while (1) {
        uint32_t data;
        read_word(cpu->mem, cpu->pc, &data);
        printf("PC: 0x%08x\n", cpu->pc);
        printf("INST: 0x%08x\n", data);
        printf("> ");
        scanf("%s", command);
        getchar();
        if (strcmp(command, "regdump") == 0) {
            for (int i = 0; i < 32; i++) {
                printf("x%d: 0x%08x (%d)\n", i, cpu->reg[i], cpu->reg[i]);
            }
        } else if (strcmp(command, "memdump") == 0) {
            uint32_t op1;
            uint32_t op2;
            printf("Enter Lower Range: ");
            scanf("%i", &op1);
            getchar();
            printf("Enter Higher Range: ");
            scanf("%i", &op2);
            getchar();

            if (op1 >= op2) {
                printf("Error, invalid range!\n");
            } else {
                uint32_t alignlow = op1 & 0xfffffff0;
                uint32_t padlow = op1 - alignlow;
                uint32_t padhigh = (16 - ((op2 - 1) % 16)) % 16;
                uint32_t alignhigh = op2 + padhigh;
                uint32_t data;
                printf("          00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f\n");
                for (int i = alignlow; i <= alignhigh; i++) {
                    if (i == alignlow) {
                        if (i - alignlow < padlow) {
                            printf("%08x: ?? ", i);
                        } else {
                            read_byte(cpu->mem, i, (uint8_t*)&data);
                            printf("%08x: %02x ", i, data);
                        }
                    } else if (i % 16 == 0) {
                        read_byte(cpu->mem, i, (uint8_t*)&data);
                        printf("\n%08x: %02x ", i, data);
                    } else if (i % 8 == 0) {
                        if (((i - alignlow) < padlow) || ((alignhigh - i) < padhigh)) {
                            printf(" ?? ");
                        } else {
                            read_byte(cpu->mem, i, (uint8_t*)&data);
                            printf(" %02x ", data);
                        }
                    } else {
                        if (((i - alignlow) < padlow) || ((alignhigh - i) < padhigh)) {
                            printf("?? ");
                        } else {
                            read_byte(cpu->mem, i, (uint8_t*)&data);
                            printf("%02x ", data);
                        }
                    }
                }
                printf("\n");
            }
        } else if (strcmp(command, "floatdump") == 0) {
            for (int i = 0; i < 32; i++) {
                printf("f%d: 0x%08x (%f)\n", i, cpu->freg[i], cpu->freg[i]);
            }
        } else if (strcmp(command, "csrdump") == 0) {
            uint32_t op;
            printf("Enter CSR Address: ");
            scanf("%i", &op);
            getchar();
            if (op > 0xffffff) {
                printf("Error, invalid CSR Address!\n");
            } else {
                printf("CSR[%03x]: %08x\n", op, cpu->csr[op]);
            }
        } else if (strcmp(command, "step") == 0) {
            break;
        } else if (strcmp(command, "quit") == 0) {
            cpu->debug = false;
            break;
        } else if (strcmp(command, "statedump") == 0) {
            printf("Priv: %d\n", cpu->mode);
        } else {
            printf("Error, invalid debug command!\n");
        }
    }
}

void mret(hart* cpu, uint32_t instruction) {
    if (cpu->mode < 3) {
        trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
        return;
    }
    uint32_t mstatus = cpu->csr[CSR_MSTATUS];
    uint8_t mpp = (mstatus >> 11) & 3;
    bool mpie = (mstatus >> 7) & 1;
    
    cpu->mode = mpp;
    cpu->pc = cpu->csr[CSR_MEPC];
    cpu->csr[CSR_MSTATUS] &= ~0x1888;
    cpu->csr[CSR_MSTATUS] |= mpie << 3;
}

void sret(hart* cpu, uint32_t instruction) {
    if (cpu->mode < 1) {
        trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
        return;
    }
    uint32_t mstatus = cpu->csr[CSR_MSTATUS];
    uint8_t spp = (mstatus >> 8) & 1;
    bool spie = (mstatus >> 5) & 1;
    
    cpu->mode = spp;
    cpu->pc = cpu->csr[CSR_MEPC];
    cpu->csr[CSR_MSTATUS] &= ~0x122;
    cpu->csr[CSR_MSTATUS] |= spie << 1;
}

void interrupt_detector(hart* cpu) {
    bool MIE = (cpu->csr[CSR_MSTATUS] >> 3) & 1;
    bool SIE = (cpu->csr[CSR_MSTATUS] >> 1) & 1;
    uint32_t pins = cpu->csr[CSR_MIE] & cpu->csr[CSR_MIP];
    if (((pins >> 11) & 1) && ((cpu->mode != 3) || MIE)) { trap_handler(cpu, IRQ_M_EXTERNAL | (1<<31), 0); return; }
    if (((pins >> 3) & 1) && ((cpu->mode != 3) || MIE)) { trap_handler(cpu, IRQ_M_SOFTWARE | (1<<31), 0); return; }
    if (((pins >> 7) & 1) && ((cpu->mode != 3) || MIE)) { trap_handler(cpu, IRQ_M_TIMER | (1<<31), 0); return; }
    if (((pins >> 9) & 1) && ((cpu->mode == 0) || (cpu->mode == 1 && SIE))) { trap_handler(cpu, IRQ_S_EXTERNAL | (1<<31), 0); return; }
    if (((pins >> 1) & 1) && ((cpu->mode == 0) || (cpu->mode == 1 && SIE))) { trap_handler(cpu, IRQ_S_SOFTWARE | (1<<31), 0); return; }
    if (((pins >> 5) & 1) && ((cpu->mode == 0) || (cpu->mode == 1 && SIE))) { trap_handler(cpu, IRQ_S_TIMER | (1<<31), 0); return; }
}

void interrupt(hart* cpu, trap_cause cause) {
    if (cause == IRQ_M_EXTERNAL) {
        cpu->csr[CSR_MIP] |= 1 << 11;
    } else if (cause == IRQ_M_SOFTWARE) {
        cpu->csr[CSR_MIP] |= 1 << 3;
    } else if (cause == IRQ_M_TIMER) {
        cpu->csr[CSR_MIP] |= 1 << 7;
    } else if (cause == IRQ_S_EXTERNAL) {
        cpu->csr[CSR_MIP] |= 1 << 9;
    } else if (cause == IRQ_S_SOFTWARE) {
        cpu->csr[CSR_MIP] |= 1 << 1;
    } else if (cause == IRQ_S_TIMER) {
        cpu->csr[CSR_MIP] |= 1 << 5;
    }
    cpu->pause = false;
}

void clear_interrupt(hart* cpu, trap_cause cause) {
    if (cause == IRQ_M_EXTERNAL) {
        cpu->csr[CSR_MIP] &= ~(1 << 11);
    } else if (cause == IRQ_M_SOFTWARE) {
        cpu->csr[CSR_MIP] &= ~(1 << 3);
    } else if (cause == IRQ_M_TIMER) {
        cpu->csr[CSR_MIP] &= ~(1 << 7);
    } else if (cause == IRQ_S_EXTERNAL) {
        cpu->csr[CSR_MIP] &= ~(1 << 9);
    } else if (cause == IRQ_S_SOFTWARE) {
        cpu->csr[CSR_MIP] &= ~(1 << 1);
    } else if (cause == IRQ_S_TIMER) {
        cpu->csr[CSR_MIP] &= ~(1 << 5);
    }
}

void step(hart* cpu) {
    memory_t* memory = cpu->mem;
    uint32_t instruction;
    uint32_t err = read_word(memory, cpu->pc, &instruction);
    uint32_t opcode = instruction & 0x7f;

    interrupt_detector(cpu);
    if (cpu->debug) {
        debug_handler(cpu);
    }

    if (err == 1) {trap_handler(cpu, EXC_INST_ACCESS, cpu->pc); return;}
    if (err == 2) {trap_handler(cpu, EXC_INST_MISALIGN, cpu->pc); return;}

    if (cpu->pause) {
        return;
    }

    switch (opcode) {
    case 0x33:
        rtype(cpu, instruction);
    break;
    case 0x13:
        itype(cpu, instruction);
    break;
    case 0x3:
        itypemem(cpu, instruction);
    break;
    case 0x23:
        stype(cpu, instruction);
    break;
    case 0x37:
        utype(cpu, instruction);
    break;
    case 0x17:
        utypepc(cpu, instruction);
    break;
    case 0x6f:
        jtype(cpu, instruction);
    break;
    case 0x67:
        itypej(cpu, instruction);
    break;
    case 0x63:
        btype(cpu, instruction);
    break;
    case 0x73:
        if (instruction == 0x00000073) {
            ecall(cpu, instruction);
        } else if (instruction == 0x00100073) {
            printf("Entered Debug Mode at 0x%08x\n", cpu->pc);
            ebreak(cpu, instruction);
        } else if (instruction == 0x10200073) {
            mret(cpu, instruction);
        } else if (instruction == 0x30200073) {
            sret(cpu, instruction);
        } else if (instruction == 0x10500073) {
            cpu->pause = true;
        } else {
            systemtype(cpu, instruction);
        }
    break;
    case 0xf:
        fence(cpu, instruction);
    break;
    default:
        trap_handler(cpu, EXC_ILLEGAL_INST, instruction);
    break;
    }
    cpu->reg[0] = 0;
    cpu->pc += 4;
}