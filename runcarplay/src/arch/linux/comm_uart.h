#ifndef _COMM_UART_H_
#define _COMM_UART_H_

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>

class comm_uart {
public:
	int open_dev(const char *dev, int *fd);
	int close_dev(int fd);

	int set_speed(int fd, speed_t speed);
	int set_parity(int fd, int databit, int stopbit, char parity);

	int write(int fd, const char *data, int size);
	int read(int fd, char *data, int size);
	struct timeval tv;
};

#endif /* COMM_UART_H_ */

