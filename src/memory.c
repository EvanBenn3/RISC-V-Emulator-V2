#include <stdlib.h>

#include "../include/memory.h"
#include <string.h>

void init_memory(memory_t* mem, uint32_t bytes) {
    mem->size = 0;
    mem->max_size = bytes / 4096;
    for (int i = 0; i < 1048576; i++) {
        mem->page_table[i] = (page_t*)NULL;
    }
}

void allocate_page(memory_t* mem, uint32_t page) {
    if (mem->size + 1 > mem->max_size) {
        return;
    } else {
        mem->page_table[page] = (page_t*)malloc(sizeof(page_t));
        memset(mem->page_table[page], 0, 4096);
        mem->size++;
    }
}

int read_byte(memory_t* mem, uint32_t address, uint8_t* dest) {
    page_t* page = mem->page_table[address >> 12];
    int error;
    if (!page) {
        allocate_page(mem, address >> 12);
        page = mem->page_table[address >> 12];
    }
    if (!page) {
        return 1;
    } else {
        *dest = page->data[address & 0xfff];
    }
    mem->last_address = address;
    mem->last_address_size = 1;
    mem->last_address_type = 'r';
    return 0;
}

int read_halfword(memory_t* mem, uint32_t address, uint16_t* dest) {
    if (address % 2) {
        return 2;
    }
    int error = 0;
    uint8_t high, low;
    if (read_byte(mem, address+1, &high) == 1) return 1;
    if (read_byte(mem, address, &low) == 1) return 1;
    *dest = low | (high << 8);
    mem->last_address = address;
    mem->last_address_size = 2;
    mem->last_address_type = 'r';
    return 0;
}

int read_word(memory_t* mem, uint32_t address, uint32_t* dest) {
    if (address % 4) {
        return 2;
    }
    int error = 0;
    uint8_t byte0, byte1, byte2, byte3;
    if (read_byte(mem, address, &byte0) == 1) return 1;
    if (read_byte(mem, address+1, &byte1) == 1) return 1;
    if (read_byte(mem, address+2, &byte2) == 1) return 1;
    if (read_byte(mem, address+3, &byte3) == 1) return 1;
    *dest = byte0 | (byte1 << 8) | (byte2 << 16) | (byte3 << 24);
    mem->last_address = address;
    mem->last_address_size = 4;
    mem->last_address_type = 'r';
    return 0;
}

int write_byte(memory_t* mem, uint32_t address, uint8_t byte) {
    page_t* page = mem->page_table[address >> 12];
    if (!page) {
        allocate_page(mem, address >> 12);
        page = mem->page_table[address >> 12];
    }
    if (page) {
        page->data[address & 0xfff] = byte;
    } else {
        return 1;
    }
    mem->last_address = address;
    mem->last_address_size = 1;
    mem->last_address_type = 'w';
    return 0;
}

int write_halfword(memory_t* mem, uint32_t address, uint16_t halfword) {
    if (address % 2) {
        return 2;
    }
    if (write_byte(mem, address, halfword & 0xff) == 1) return 1;
    if (write_byte(mem, address + 1, halfword >> 8) == 1) return 1;
    mem->last_address = address;
    mem->last_address_size = 2;
    mem->last_address_type = 'w';
    return 0;
}

int write_word(memory_t* mem, uint32_t address, uint32_t word) {
    if (address % 4) {
        return 2;
    }
    if (write_byte(mem, address, word & 0xff) == 1) return 1;
    if (write_byte(mem, address + 1, (word >> 8) & 0xff) == 1) return 1;
    if (write_byte(mem, address + 2, (word >> 16) & 0xff) == 1) return 1;
    if (write_byte(mem, address + 3, (word >> 24) & 0xff) == 1) return 1;
    mem->last_address = address;
    mem->last_address_size = 4;
    mem->last_address_type = 'w';
    return 0;
}

int write_region(memory_t* mem, uint8_t* array, uint32_t address, uint32_t offset) {
    for (int i = 0; i < offset; i++) {
        if (write_byte(mem, address+i, array[i]) == 1) return 1;
    }
    mem->last_address = address;
    mem->last_address_size = offset;
    mem->last_address_type = 'w';
    return 0;
}

int read_region(memory_t* mem, uint8_t* array, uint32_t address, uint32_t offset) {
    for (int i = 0; i < offset; i++) {
        if (read_byte(mem, address+i, &array[i]) == 1) return 1;
    }
    mem->last_address = address;
    mem->last_address_size = offset;
    mem->last_address_type = 'r';
    return 0;
}

bool check_dirty(memory_t* mem, uint32_t address) {
    for (int i = 0; i < mem->last_address_size; i++) {
        if (mem->last_address + i == address) return true;
    }
    return false;
}