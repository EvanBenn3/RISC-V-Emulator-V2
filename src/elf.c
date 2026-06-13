#include "..\include\elf.h"

ElfProgram *parse_elf(const char *filename) {
    uint8_t header[52];

    FILE* f = fopen(filename, "rb");
    if (f == NULL) {
        return NULL;
    }
    fread(header, 1, 52, f);

    uint32_t e_entry = header[24] | (header[25]<<8) | (header[26]<<16) | (header[27]<<24);
    uint32_t e_phoff = header[28] | (header[29]<<8) | (header[30]<<16) | (header[31]<<24);
    uint16_t e_phnum = header[44] | (header[45]<<8);

    fseek(f, e_phoff, SEEK_SET);
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    LoadSegment *segments = NULL;
    uint32_t size = 0;
    for (int i = 0; i < e_phnum; i++) {
        uint8_t phdr[32];
        fread(phdr, 1, 32, f);
        
        uint32_t p_type = phdr[0] | (phdr[1]<<8) | (phdr[2]<<16) | (phdr[3]<<24);
        
        if (p_type == 1) {  // LOAD
            p_offset = phdr[4] | (phdr[5]<<8) | (phdr[6]<<16) | (phdr[7]<<24);
            p_vaddr  = phdr[8] | (phdr[9]<<8) | (phdr[10]<<16) | (phdr[11]<<24);
            p_filesz = phdr[16] | (phdr[17]<<8) | (phdr[18]<<16) | (phdr[19]<<24);
            p_memsz  = phdr[20] | (phdr[21]<<8) | (phdr[22]<<16) | (phdr[23]<<24);

            segments = realloc(segments, sizeof(LoadSegment) * (size+1));
            LoadSegment segment;
            segment.p_type = 1;
            segment.p_vaddr = p_vaddr;
            segment.p_filesz = p_filesz;
            segment.p_memsz = p_memsz;

            fseek(f, p_offset, SEEK_SET);
            uint8_t* data = malloc(p_memsz);
            fread(data, 1, p_filesz, f);
            memset(data + p_filesz, 0, p_memsz - p_filesz);
            segment.p_data = data;
            
            segments[size++] = segment;
        }
    }
    fclose(f);

    ElfProgram *elf = malloc(sizeof(ElfProgram));
    elf->entry_point = e_entry;
    elf->segment_count = size;
    elf->segments = segments;
    return elf;
}

void free_elf(ElfProgram *elf) {
    for (int i = 0; i < elf->segment_count; i++){
        free(elf->segments[i].p_data);
    }
    free(elf->segments);
    free(elf);
}