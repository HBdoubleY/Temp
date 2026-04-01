#include "stdafx.h"
#include "stdio.h"
#include "sysarch.h"
#include "sysapi.h"
#include <golbal_sys_para.hpp>
#include <comm_sync.hpp>
#include "ComAPI.h"
//#include "FileCheck.hpp"
#include "board.h"
#ifdef SYS_LINUX
#endif

#ifdef WIN32
#pragma init_seg(".CRT$XCC")
#endif

using namespace std;

#ifdef SYS_LINUX
#define GOLBAL_FILE_BASEPARA		    ((char*)"/opt/work/data/basepara")

#else
#define GOLBAL_FILE_BASEPARA		    ((char*)"./cfg/data/basepara")

#endif



int GolbalSysPara::first = 0;



//版本信息
float GolbalSysPara::hv = 1.0f;
float GolbalSysPara::sv = 2.00f;



int GolbalSysPara::fb_width = 720;
int GolbalSysPara::fb_height = 1440;


int GolbalSysPara::carplayType = 0x2; //LINK_TYPE_CARPLAY
int GolbalSysPara::ConnType = 0x02;//carplay

GolbalSysPara::GolbalSysPara() {

}

void GolbalSysPara::init(void) {
	if (first == 0) {
		first++;
		loadPara();
	}
}

int GolbalSysPara::savePara(void) {
/*
	if (fs.saveOpen(GOLBAL_FILE_BASEPARA) == 0){
		fs << "fb_width" << fb.width;
		fs << "fb_height" << fb.height;
					
		fs.saveClose();
	}
*/
	int buff[32];
	buff[0] = fb_width;
	buff[1] = fb_height;
	buff[2] = carplayType;
	buff[3] = ConnType;
	WriteFile(GOLBAL_FILE_BASEPARA, (BYTE *)&buff, sizeof(buff));
#ifdef SYS_LINUX
	sync();
	system("sync");
#endif
	return 0;
}

void GolbalSysPara::loadPara(void) {	
	/*
	if (fs.loadOpen(GOLBAL_FILE_BASEPARA) == 0){
				
		if (fs["fb_width"].isNone() == false) {
			fs["fb_width"] >> fb.width;
		}
		if (fs["fb_height"].isNone() == false) {
			fs["fb_height"] >> fb.height;
		}
		
		if (fb.width < 320 || fb.height < 240){
			fb.width = 1280;
			fb.height = 720;
		}
		
		fs.loadClose();
	}*/
	
	int buff[32];
	int len = readfile(GOLBAL_FILE_BASEPARA, (BYTE *)&buff, sizeof(buff));
	if (len == sizeof(buff)){
		fb_width = buff[0];
		fb_height = buff[1];
		carplayType = buff[2];
		ConnType = buff[3];
		printf("####--%s---%d---%d----%d----%d---\n",__FUNCTION__,fb_width,fb_height,carplayType,ConnType);
	}
}


GolbalSysPara gsp;
