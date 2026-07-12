#define UART_TX   (*(volatile unsigned int*)0x10000000)
#define UART_RX   (*(volatile unsigned int*)0x10000004)
#define UART_STATUS (*(volatile unsigned char*)0x10000008)
#define UART_CONTROL (*(volatile unsigned char*)0x10000010)

void UART_write(unsigned char data) {
    if (!(UART_STATUS & 1)) {
        UART_TX = data;
    }
}

void write_char(char c) {
    UART_write(c);
}

void write_string(const char* string) {
    while (*string) {
        write_char(*string++);
    }
}

int main(void) {
    UART_CONTROL = 1;
    write_string("Why does this actually work?\n");
    return 0;
}
