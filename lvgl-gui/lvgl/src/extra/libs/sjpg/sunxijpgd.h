/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : jpegdecode.h
 * Description : jpegdecode
 * History :
 *
 */


#ifndef JPEG_DECODE
#define JPEG_DECODE

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../lv_conf_internal.h"
#if LV_USE_SJPG && defined(USE_HARDWARE_JPEGDECODER)

#include "vdecoder.h"           //* video decode library in "libcedarc/include/"
#include "memoryAdapter.h"

typedef enum JpegDecodeScaleDownRatio{
    JPEG_DECODE_SCALE_DOWN_1      = 0, /*no scale down*/
    JPEG_DECODE_SCALE_DOWN_2      = 1, /*scale down 1/2*/
    JPEG_DECODE_SCALE_DOWN_4      = 2, /*scale down 1/4*/
    JPEG_DECODE_SCALE_DOWN_8      = 3, /*scale down 1/8*/
}JpegDecodeScaleDownRatio;

typedef enum JpegDecodeOutputDataType
{
    JpegDecodeOutputDataNV21 = 1,
    JpegDecodeOutputDataNV12 = 2,
    JpegDecodeOutputDataYU12 = 3,
    JpegDecodeOutputDataYV12 = 4,
    JpegDecodeOutputDataRGB565 = 5,
    JpegDecodeOutputDataARGB8888 = 6,
}JpegDecodeOutputDataType;

typedef struct ImgFrame
{
    uint32_t mWidth;                // the width which contains aligned buffer
    uint32_t mHeight;               // the height which contains aligned buffer
    uint32_t mDisplayWidth;         // the actural frame width
    uint32_t mDisplayHeight;        // the actural frame height
    uint32_t mYuvSize;              // Number of bytes in mYuvData
    uint8_t* mYuvData;              // Actual YUV data
    uint32_t mRGBSize;              // Number of bytes in mRGBData
    uint8_t* mRGBData;              // Actual RGB565 or ARGB8888 data
    int32_t  mRotationAngle;	    // rotation angle, clockwise
}ImgFrame;

typedef struct JpegDecoderContext
{
    VideoDecoder*      mVideoDecoder;
    ImgFrame           mImgFrame;
    char*              mSrcBuf;
    int                mSrcBufLen;
    int                mScaleDownEn;
    int                mHorizonScaleDownRatio;
    int                mVerticalScaleDownRatio;
    int                mDecodedPixFormat;
    int                mOutputDataType;
    struct ScMemOpsS  *memops;
}JpegDecoderContext;

typedef void* JpegDecoder;

int sunxijpgd_load_file(char** out, int* outsize, const char* filename);
JpegDecoder* JpegDecoderCreate();
void JpegDecoderDestory(JpegDecoder* v);
void JpegDecoderSetDataSourceBuf(JpegDecoder* v, char* buffer, int bufLen,
		JpegDecodeScaleDownRatio scaleRatio, JpegDecodeOutputDataType outputType);
ImgFrame *JpegDecoderGetFrame(JpegDecoder* v);

#endif /*LV_USE_SJPG*/

#ifdef __cplusplus
}
#endif

#endif
