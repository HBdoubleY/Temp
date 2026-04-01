#ifndef UART_H_
#define UART_H_

int OpenDev(char *name);
void set_speed(int fd, int speed);
int set_Parity(int fd, int databits,int stopbits,int parity);
void str_print(char *buf, int len);
int uart_test();

#endif