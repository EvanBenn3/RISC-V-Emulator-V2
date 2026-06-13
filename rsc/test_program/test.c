volatile int* UART = (int*)0x10000000;

void _start(void) {
    *UART = 0x48;  // 'H'
    *UART = 0x69;  // 'i'
    *UART = 0x21;  // '!'
    while(1);
}