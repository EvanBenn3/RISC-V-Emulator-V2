#define UART_TX   (*(volatile unsigned short*)0x10000000)
#define UART_RX   (*(volatile unsigned short*)0x10000002)
#define UART_STATUS (*(volatile unsigned char*)0x10000003)

void UART16_write(unsigned char data) {
    UART_TX = data | (1 << 8);
    UART_STATUS |= 1;
}

void write_char(char c) {
    UART16_write(c);
}

void write_string(const char* string) {
    while (*string) {
        write_char(*string++);
    }
}

int main(void) {
    write_string("Hello World!\n");
    return 0;
}
