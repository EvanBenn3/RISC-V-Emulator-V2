#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#include "../include/cpu.h"
#include "../include/elf.h"
#include "../include/peripheral.h"

uint32_t parse_memory_size(const char *arg) {
    char suffix = arg[strlen(arg) - 1];
    uint32_t val = atoi(arg);

    switch (suffix) {
        case 'K': return val * 1024;
        case 'M': return val * 1024 * 1024;
        case 'G': return val * 1024 * 1024 * 1024;
        default:  return val; // bytes
    }
}

memory_t mem;
int main(int argc, char *argv[]) {
    char *elf_path = NULL;
    uint32_t memory_size = 256 * 4096; // default: 256 pages (1M)

    int opt;
    while ((opt = getopt(argc, argv, "k:m:")) != -1) {
        switch (opt) {
            case 'k':
                elf_path = optarg;
                break;
            case 'm':
                // parse memory size: "1M", "256K", "1G"
                memory_size = parse_memory_size(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -k <elf_file> [-m <memory_size>]\n", argv[0]);
                return 1;
        }
    }

    if (elf_path == NULL) {
        fprintf(stderr, "Error: -k flag required (ELF file path)\n");
        return 1;
    }

    init_memory(&mem, memory_size);

    hart core;
    init_hart(&core, &mem, 0x80000000, 0);

    ElfProgram *elf = parse_elf(elf_path);
    if (!elf) {
        printf("Elf Null\n");
        return -1;
    }
    for (int i = 0; i < elf->segment_count; i++) {
        write_region(&mem, elf->segments[i].p_data, elf->segments[i].p_vaddr, elf->segments[i].p_memsz);
    }
    core.pc = elf->entry_point;
    free_elf(elf);
    printf("Elf Loaded\n");

    timer_t timer;
    UART_t uart;
    PLIC_t plic;
    init_timer(&timer);
    init_PLIC(&plic, 0x0c000000);
    init_UART(&uart, &plic, 0x10000000);

    int inst_count = 0;
    bool running = true;
    bool exit_dirty = false;
    uint8_t exit_byte;
    clock_t start = clock();
    while (running) {
        step(&core);
        if (check_dirty(&mem, 0x52) && mem.last_address_type == 'w') {
            read_byte(&mem, 0x52, &exit_byte);
            printf("Returned: %d\n", exit_byte);
            running = false;
        }

        step_timer(&timer, &mem, &core);

        step_UART(&uart, &mem, &plic);
        if (uart.txp > 0) {
            uint8_t data;
            read_UART(&uart, &data);
            printf("%c", data);
        }

        step_PLIC(&plic, &mem, &core);

        inst_count++;
    }

    clock_t end = clock();

    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Time: %f\n", cpu_time);
    
    printf("Count: %d\n", inst_count);
    return 0;
}

/*
Missing batch file to compile automatically test programs
Missing floating point
Missing the logic for the disk peripheral
Missing MMU and virtual memory
Missing Test Program that tests all of this
Want network Peripheral
Want OS
Want Graphics
Want Audio
Want Graphical Console
*/

/*
    printf("Entry: %08x\n", elf->entry_point);
    printf("Seg Count: %d\n", elf->segment_count);
    for (int i = 0; i < elf->segment_count; i++) {
        printf("Size: %d\n", elf->segments[i].p_memsz);
        printf("Address: %08x\n", elf->segments[i].p_vaddr);
        for (int j = 0; j < elf->segments[i].p_memsz; j++){
            printf("Data %d: %02x\n", i, elf->segments[i].p_data[j]);
        }
    }
    */
