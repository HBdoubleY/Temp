#ifndef  _BOARD_H_
#define  _BOARD_H_

#include <apptypedef.h>
#include "Rtc.h"

//#define BOARD_T707_CORE

typedef struct {
	unsigned int rear_ctr:1; // 转向灯检查 B 0有效
	unsigned int left_ctr:1; //L　0有效
	unsigned int rigth_ctr:1;//R　0有效
	unsigned int video_led:1;// 视频指示灯
	unsigned int disp_pwr:1; //disp onoff　
	unsigned int ahd_enable:1; //ahd PT1000K
	unsigned int camera_pwr:1; // camera power
	unsigned int acc_pwr:1; // acc check
	unsigned int sys_pwr:1; // 系统电源
	unsigned int zd_sendor_pwr:1; // //zd senser input   bg
	unsigned int usb_pwr:1; // //usb pwr
	unsigned int wdg_ctr:1; // //看门狗控制 pwr
	unsigned int reverse:4;
}kwx_bsp_gpio_bit;

typedef union
{
	unsigned int all;
	kwx_bsp_gpio_bit bit;
}kwx_bsp_gpio;


void ClearWDG();
void InitGPIO();

bool InitDrivers(WORD wHwType);
extern kwx_bsp_gpio Kwx_io_ctr;


#endif
