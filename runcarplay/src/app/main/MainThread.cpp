#include "stdafx.h"
#include "sysfs.h"
#include "sysarch.h"
#include "sysapi.h"
#include "board.h"
#include "sysdebug.h"
#include "Monitor.h"
#include "sysapi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef SYS_LINUX
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <comm_uart.h>
#include <DHCryptlib.h>
#endif
#include "MainThread.hpp"
#include "Timer.h"
#include "lvgl_main.h"

#include "golbal_sys_para.hpp"

#include "bt_serial.h"
#include "tire/tire_manager.h"
#include <thread>

#ifdef ENABLE_CARPLAY
#include "zlink_client.h"
#include "link_touch_evdev.h"
#endif

void InitTarget(void) {
	InitDir();
	SysInit();
}



TThreadRet WdgThread(void* pvPara) {
	int iMonitorID;
	int i = 0;
	
	ClearWDG();
//	SyncTimer();

	if (i++ >= 10) {
		i = 0;

		iMonitorID = RunMonitor(); 
		if (iMonitorID < 0) {
			iMonitorID = -iMonitorID - 1;

			char name[128];
			GetMonitorName(iMonitorID, name);
			printf("monitor reset cpu: %s", name);
			Sleep(10000);
			ResetApp();
			while (1) {
				Sleep(1000);
			}
		}
	}
	Sleep(200);
	return THREAD_RET_OK;
}

int WdgThreadService(void) {

	NewThread(WdgThread, NULL, 2048*4, THREAD_PRIORITY_ABOVE_NORMAL);
	return 0;
}


#ifdef SYS_LINUX

int lil_is_file_exist(char *pth){
	if (!pth) {
		return -1;
	}
	if (access(pth, F_OK) == 0){
		return 0;
	}
	return -1;
}


void user_config_pwd_dir(void){
   	
	if (access("/opt/work/config", F_OK) != 0){
		system("mkdir /opt/work/config");  
	}
/*
	if (system("mount /dev/mmcblk0p1 /opt/work/config") != 0){
		system("mount /dev/mmcblk0p1 /opt/work/config"); 
	}
*/
	if (access("/opt/work/config/license", F_OK) != 0){
		system("mkdir /opt/work/config/license"); 
	}

}


#endif


extern int GuiAppService(int arg);
extern int MenuIfService(void);



#if defined(__cplusplus)||defined(c_plusplus)
extern "C"
{
#endif

extern int carplayMain();


#if defined(__cplusplus)||defined(c_plusplus)
}
#endif


void SysGpioInit(void){


}

void SysSetTp(int x, int y, int e, int ms) {

}

void MenuIfRefresh(void){

}



int g_ScreenWidth = 0;
int g_ScreenHeight = 0;
TThreadRet MainThread(void* pvPara) {

// 	SysGpioInit();
// 	SysInitDebug();
// 	InitTarget();
		
// 	InitMonitor(); 
// 	int iMonitorID = ReqMonitor((char *) "main-thrd", 20);
// 	gsp.init();
	
// 	WdgThreadService();

// #ifdef SYS_LINUX
	// mpi_sys_init();
// 	user_config_pwd_dir();
// #endif

	// TimerService();
// #ifdef SYS_LINUX
	
// 	g_ScreenWidth = gsp.fb_width;
// 	g_ScreenHeight = gsp.fb_height;
// #else
// 	g_ScreenWidth = gsp.fb_width;
// 	g_ScreenHeight = gsp.fb_height;
// #endif	

// 	Sleep(3*1000);

#ifdef ENABLE_CARPLAY
	carplay_link_touch_init();
#endif
	LvglService(gsp.fb_width, gsp.fb_height);

	sleep(3);

	tire_manager_init();
	if (bt_serial_init("/dev/goc_serial", 460800, on_bt_data, NULL) != 0) {
		fprintf(stderr, "Failed to init serial\n");
		// return 1;
	}

	// std::thread t([](){
	// 	const char* cmd = 
	// 		"wpa_cli -i wlan1 scan";
	// 		// "udhcpc -b -i wlan1 -q >/tmp/udhcpc_wlan1.log 2>&1 || true; ";
	// 	std::system(cmd);
	// });
	// t.detach();

#ifdef ENABLE_CARPLAY
	{
		NewThread([](void*) -> TThreadRet {
			zlink_client_run();
			return (TThreadRet)0;
		}, NULL, 4096, THREAD_PRIORITY_NORMAL);
	}
#endif
	// carplayMain();  //while

		while(1){
Sleep(5000);
		}
	Sleep(5000);
#ifdef SYS_LINUX
	system("umount -f /opt/usb/usb1");
	system("umount -l /opt/usb/usb1");
#endif


	// FreeMonitor(iMonitorID);
	return THREAD_RET_OK;
}

