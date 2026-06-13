#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "..\include\memory.h"
#include "..\include\cpu.h"

typedef struct {
    uint64_t mtime;
    uint64_t mtimecmp;
    uint64_t stimecmp;
} timer_t;

typedef struct {
    uint32_t address;
    bool pending[1024];
    bool asserted[1024];
    bool blocked[1024];
    bool processing;
    uint32_t curr_ID;
    int device_count;
} PLIC_t;

typedef struct {
    uint32_t address;
    uint8_t tx[64];
    uint32_t txp;
    uint8_t rx[64];
    uint32_t rxp;
    bool TXFULL;
    bool RXEMPTY;
    bool TX_IE, RX_IE;
    bool TX_E, RX_E;
    uint32_t ID;
} UART_t;

void init_timer(timer_t* timer);
void step_timer(timer_t* timer, memory_t* mem, hart* cpu);

void init_PLIC(PLIC_t* plic, uint32_t address);
int add_PLIC(PLIC_t* plic);
void interrupt_PLIC(PLIC_t* plic, uint32_t ID);
void clear_PLIC(PLIC_t* plic, uint32_t ID);
void step_PLIC(PLIC_t* plic, memory_t* mem, hart* cpu);

void init_UART(UART_t* uart, PLIC_t* plic, uint32_t address);

#endif