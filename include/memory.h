#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t data[4096];
} page_t;

typedef struct {
    page_t* page_table[1048576];
    uint32_t size;
    uint32_t max_size;
    uint32_t last_address;
    uint32_t last_address_size;
    char last_address_type;
} memory_t;

void init_memory(memory_t* mem, uint32_t bytes);
//int allocate_page(memory_t* mem, uint32_t page);

int read_byte(memory_t* mem, uint32_t address, uint8_t* dest);
int read_halfword(memory_t* mem, uint32_t address, uint16_t* dest);
int read_word(memory_t* mem, uint32_t address, uint32_t* dest);
int read_region(memory_t* mem, uint8_t* array, uint32_t address, uint32_t offset);

int write_byte(memory_t* mem, uint32_t address, uint8_t byte);
int write_halfword(memory_t* mem, uint32_t address, uint16_t halfword);
int write_word(memory_t* mem, uint32_t address, uint32_t word);
int write_region(memory_t* mem, uint8_t* array, uint32_t address, uint32_t offset);

bool check_dirty(memory_t* mem, uint32_t address);

#endif