
/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : typedef.h
* Description :
* History :
*   Author  : xyliu <xyliu@allwinnertech.com>
*   Date    : 2016/04/13
*   Comment :
*
*
*/


#ifndef TYPEDEF_H
#define TYPEDEF_H
#ifndef CONFIG_VIDEO_RT_MEDIA
#include <stdint.h>
#endif
    typedef unsigned char u8;
    typedef unsigned short u16;
    typedef unsigned int u32;
#ifdef COMPILER_ARMCC
    typedef unsigned __int64 u64;
#else
    typedef unsigned long long u64;
#endif
    typedef signed char s8;
    typedef signed short s16;
    typedef signed int s32;
    typedef float f32;
    typedef double f64;
#ifdef COMPILER_ARMCC
    typedef signed __int64 s64;
#else
    typedef signed long long s64;
#endif
    typedef uintptr_t size_addr;

	typedef long long               S64;
	typedef int                     S32;
	typedef short                   S16;
	typedef char                    S8;
	typedef unsigned long long      U64;
	typedef unsigned int            U32;
	typedef unsigned short          U16;
	typedef unsigned char           U8;
#endif

