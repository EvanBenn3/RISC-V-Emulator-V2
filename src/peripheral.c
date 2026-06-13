#include "..\include\peripheral.h"

#include <stdlib.h>
#include <string.h>

void init_timer(timer_t* timer) {
    timer->mtime = 0;
    timer->mtimecmp = ~0;
    timer->stimecmp = ~0;
}

void step_timer(timer_t* timer, memory_t* mem, hart* cpu) {
    if (check_dirty(mem, 0x02000000) || check_dirty(mem, 0x02000004)) {
        if (mem->last_address_type == 'w') {
            write_word(mem, 0x2000000, (uint32_t)(timer->mtime & 0xffffffff));
            write_word(mem, 0x2000004, (uint32_t)(timer->mtime >> 32));
            mem->last_address = 0;
        }
    }
    if (check_dirty(mem, 0x02004000) || check_dirty(mem, 0x02004004)) {
        if (mem->last_address_type == 'w') {
            uint64_t val;
            read_word(mem, 0x2004000, (uint32_t*)&val);
            read_word(mem, 0x2004004, ((uint32_t*)&val)+1);
            timer->mtimecmp = val;
            mem->last_address = 0;
        }
    }
    if (check_dirty(mem, 0x2004008) || check_dirty(mem, 0x200400c)) {
        if (mem->last_address_type == 'w') {
            uint64_t val;
            read_word(mem, 0x2004008, (uint32_t*)&val);
            read_word(mem, 0x200400c, ((uint32_t*)&val)+1);
            timer->stimecmp = val;
            mem->last_address = 0;
        }
    }
    if (timer->mtime >= timer->mtimecmp) {
        interrupt(cpu, IRQ_M_TIMER);
    } else {
        clear_interrupt(cpu, IRQ_M_TIMER);
    }
    if (timer->mtime >= timer->stimecmp) {
        interrupt(cpu, IRQ_S_TIMER);
    } else {
        clear_interrupt(cpu, IRQ_S_TIMER);
    }
    timer->mtime += 1;
}

void init_PLIC(PLIC_t* plic, uint32_t address) {
    plic->device_count = 0;
    for (int i = 0; i < 1024; i++) plic->pending[i] = false;
    for (int i = 0; i < 1024; i++) plic->asserted[i] = false;
    for (int i = 0; i < 1024; i++) plic->blocked[i] = false;
    plic->address = address;
    plic->processing = 0;
    plic->curr_ID = 0;
}

int add_PLIC(PLIC_t* plic) {
    if (plic->device_count >= 1024) return 0;
    plic->device_count += 1;
    return plic->device_count;
}

void interrupt_PLIC(PLIC_t* plic, uint32_t ID) {
    if (ID == 0) return;
    plic->asserted[ID] = true;
}

void clear_PLIC(PLIC_t* plic, uint32_t ID) {
    if (ID == 0) return;
    plic->asserted[ID] = false;
}

void step_PLIC(PLIC_t* plic, memory_t* mem, hart* cpu) {
    uint32_t priority[1024];
    bool enable0[1024];
    bool enable1[1024];
    uint8_t pass[1024];
    uint32_t m_priority;
    uint32_t s_priority;
    bool is_m_dirty = false;
    bool is_s_dirty = false;
    if (check_dirty(mem, plic->address + 0x200004)) {
        is_m_dirty = true;
        mem->last_address = 0;
    } else if (check_dirty(mem, plic->address + 0x201004)) {
        is_s_dirty = true;
        mem->last_address = 0;
    }

    if (is_m_dirty && (mem->last_address_type == 'r')) { // checks claim
        plic->pending[plic->curr_ID] = false;
        plic->processing = true;
    } else if (is_s_dirty && (mem->last_address_type == 'r')) {
        plic->pending[plic->curr_ID] = false;
        plic->processing = true;
    }

    for (int i = 0; i < 128; i++) { // updates pending
        uint8_t byte = 0;
        for (int j = 0; j < 8; j++) byte |= (plic->pending[i+j] << j);
        write_byte(mem, plic->address + 0x1000 + i, byte); 
    }
    if (plic->processing) {
        if (is_m_dirty && (mem->last_address_type == 'w')) { // checks complete
            uint32_t ret_ID;
            read_word(mem, plic->address + 0x200004, &ret_ID);
            if (ret_ID == plic->curr_ID) {
                plic->blocked[plic->curr_ID] = false;
                plic->processing = false;
                clear_interrupt(cpu, IRQ_M_EXTERNAL);
            }
        } else if (is_s_dirty && (mem->last_address_type == 'w')) {
            uint32_t ret_ID;
            read_word(mem, plic->address + 0x201004, &ret_ID);
            if (ret_ID == plic->curr_ID) {
                plic->blocked[plic->curr_ID] = false;
                plic->processing = false;
                clear_interrupt(cpu, IRQ_S_EXTERNAL);
            }
        }
    }

    for (int i = 1; i < 1024; i++) {
        if (!plic->blocked[i] && plic->asserted[i]) {
            plic->pending[i] = true; // sets pending bits
            plic->blocked[i] = true;
        }
        if (!plic->processing) {
            read_word(mem, plic->address + 4*i, &priority[i]); // reads priority
            if (priority[i] > 7) {
                priority[i] = 0;
                write_word(mem, plic->address + 4*i, 0);
            }
        }

    }
    if (!plic->processing) {
        for (int i = 0; i < 1024; i++) { // reads enable signals
            read_byte(mem, plic->address + 0x2000 + i, &enable0[i]);
            read_byte(mem, plic->address + 0x2080 + i, &enable1[i]); 
        }
        read_byte(mem, plic->address + 0x200000, &m_priority);
        read_byte(mem, plic->address + 0x201000, &s_priority);
        if (m_priority > 7) {
            m_priority = 0;
            write_word(mem, plic->address + 0x200000, 0);
        }
        if (s_priority > 7) {
            s_priority = 0;
            write_word(mem, plic->address + 0x201000, 0);
        }

        for (int i = 0; i < 1024; i++) {
            pass[i] = 0;
            if (plic->pending[i] && enable0[i] && (priority[i] >= m_priority)) {
                pass[i] = priority[i];
            }
            if (plic->pending[i] && enable1[i] && (priority[i] >= s_priority)) {
                pass[i] = priority[i] | (1 << 7);
            }
        }
    
        for (int i = 7; i > 0; i--) {
            for(int j = 0; j < 1024; j++) {
                if ((pass[j] & 0x7) == i) {
                    if (pass[j] & ~0x7) {
                        interrupt(cpu, IRQ_S_EXTERNAL);
                        write_word(mem, plic->address + 0x201004, j);
                        plic->curr_ID = j;
                    } else {
                        interrupt(cpu, IRQ_M_EXTERNAL);
                        write_word(mem, plic->address + 0x200004, j);
                        plic->curr_ID = j;
                    }
                    return;
                }
            }
        }
    }
}


void init_UART(UART_t* uart, PLIC_t* plic, uint32_t address) {
    uart->address = address;
    uart->txp = 0;
    uart->rxp = 0;
    uart->TXFULL = false;
    uart->RXEMPTY = false;
    uart->TX_IE = false;
    uart->RX_IE = false;
    uart->ID = add_PLIC(plic);
}

void step_UART(UART_t* uart, memory_t* mem) {
    bool is_TX_dirty = false, is_RX_dirty = false, is_TX_enable = false, is_RX_enable = false;

    if (check_dirty(mem, uart->address)) {
        is_TX_dirty = true;
        mem->last_address = 0;
    } else if (check_dirty(mem, uart->address + 0x4)) {
        is_RX_dirty = true;
        mem->last_address = 0;
    }
    uint32_t val;
    read_word(mem, uart->address + 0x10, &val);

    // TX: CPU wrote data to send
    if (is_TX_dirty && (mem->last_address_type == 'w') && !uart->TXFULL && is_TX_enable) {
        read_word(mem, uart->address, &val);
        // TODO: clear TX interrupt pending
    }

    // RX: CPU read received data
    if (is_RX_dirty && (mem->last_address_type == 'r') && !uart->RXEMPTY && is_RX_enable) {
        // TODO: clear RX interrupt pending
    }

    // IE: interrupt enable changed
    // TODO: re-evaluate pending interrupts

    // CTRL: enable bits changed
    // TODO: enable/disable TX/RX paths

    // Check if we should raise/lower interrupts to PLIC
    // TODO: if (TX empty && TX IE) -> PLIC_interrupt
    // TODO: if (RX has data && RX IE) -> PLIC_interrupt

}

int write_UART(UART_t* uart, uint8_t data) {
    if (uart->rxp >= 63) return 1;
    uart->rx[++uart->rxp] = data;
    return 0;
}

int read_UART(UART_t* uart, uint8_t* data) {
    if (uart->txp == 0) return 1;
    uint8_t val = uart->tx[0];
    memmove(uart->tx, uart->tx+1, --uart->txp);
    *data = val;
    return 0;
}