#ifndef ELF_H
#define ELF_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*
#pragma pack(1)
typedef struct {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} Elf32_Phdr;
#pragma pack()
*/

typedef struct {
    uint32_t p_type;
    uint32_t p_vaddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint8_t *p_data;  // allocated buffer containing p_filesz bytes
} LoadSegment;

typedef struct {
    uint32_t entry_point;
    LoadSegment *segments;
    int segment_count;
} ElfProgram;

ElfProgram *parse_elf(const char *filename);
void free_elf(ElfProgram *elf);

#endif