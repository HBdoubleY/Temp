#include <stdafx.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <sys/errno.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sysarch.h>
#include <linux/rtc.h>
#include "board.h"
#include <sysapi.h>


//kwx_bsp_gpio Kwx_io_ctr;

void InitGPIO(void) {

}

void ClearWDG() {
	if (IsAppExit()) {
		return;
	}

//	system("echo 0 > /dev/watchdog");

/*		
	char tmp[40];
	if (Kwx_io_ctr.bit.wdg_ctr == 0) {
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"echo %d > /sys/class/gpio_sw/P%c%d/data", 1,'I',13); 
		system(tmp); 
		Kwx_io_ctr.bit.wdg_ctr = 1;
	} else {
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"echo %d > /sys/class/gpio_sw/P%c%d/data", 0,'I',13); 
		system(tmp); 
		Kwx_io_ctr.bit.wdg_ctr = 0;
	}
*/
}

bool InitDrivers(WORD wHwType) {
	InitGPIO();

	return true;
}

