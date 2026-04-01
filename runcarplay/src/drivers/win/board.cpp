#include "stdafx.h"
#include "board.h"
#include "sysapi.h"

void InitGPIO(void) {

}

void ClearWDG() {
	if (IsAppExit()) {
		return;
	}
	/*
	 g_dwRunCnt++;
	 int fd = open(GPIODEV, O_RDWR);
	 if (fd < 0)
	 {
	 printf("fail to open gpio device for clearwdg!\n");
	 return;
	 }

	 ioctl(fd, (g_dwRunCnt & 1) | GPIO_L, PIN_WGT_CLR);
	 close(fd);
	 */
}

bool InitDrivers(WORD wHwType) {
	InitGPIO();

	return true;
}
