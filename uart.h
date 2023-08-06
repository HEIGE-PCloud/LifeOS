#ifndef UART_H
#define UART_H
void init_uart();
void putc(unsigned int c);
char getc();
void puts(char *s);
void print_hex(unsigned int d);
#endif
