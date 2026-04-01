#ifndef __PARA_SYSPARA_HPP__
#define __PARA_SYSPARA_HPP__

//#include <opencv2/opencv.hpp>
#include <string>
#include "apptypedef.h"
#include "comm_sig.hpp"
#include "syscfg.h"
#include "comm_sync.hpp"
//#include "FileCheck.hpp"

//using namespace cv;




class GolbalSysPara {
private:
	

public:
	static int first;

	static float hv;
	static float sv;

	static int fb_width;					//Cvbs
	static int fb_height;					//Cvbs

	static int carplayType;
	static int ConnType;
public:
	GolbalSysPara();
	int savePara(void);
	void loadPara(void);

	void init(void);

};

extern GolbalSysPara gsp;

#endif /* PARA_SYSPARA_HPP_ */
