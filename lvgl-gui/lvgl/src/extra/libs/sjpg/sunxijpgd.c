/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : sunxijpgd.c
 * Description : jpegdecode for Linux,
 *               decode jpeg image(base line format)
 * History :
 *
 */

#include "sunxijpgd.h"
#if LV_USE_SJPG && defined(USE_HARDWARE_JPEGDECODER)

#include <errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../../lvgl.h"
#ifdef USE_SUNXIFB_G2D
#include <sys/ioctl.h>
#include "../../../../../lv_drivers/display/sunximem.h"
#include "../../../../../lv_drivers/display/sunxig2d.h"
extern int g_g2dfd;
#endif

#ifdef USE_SUNXIFB_G2D
static int g2d_transform(unsigned char* srcY, unsigned char* srcU, unsigned char* srcV,
		unsigned char* dst, unsigned int dst_size, unsigned int width, unsigned int height,
		JpegDecodeOutputDataType outputType)
{
	g2d_blt_h blit_para;
	memset(&blit_para, 0, sizeof(g2d_blt_h));
    sunxifb_mem_flush_cache(dst, dst_size);

	blit_para.src_image_h.laddr[0] = (uintptr_t)srcY;
	blit_para.src_image_h.laddr[1] = (uintptr_t)srcU;
	blit_para.src_image_h.laddr[2] = (uintptr_t)srcV;
	blit_para.src_image_h.use_phy_addr = 1;

	blit_para.dst_image_h.laddr[0] = (uintptr_t)sunxifb_mem_get_phyaddr(dst);
	blit_para.dst_image_h.use_phy_addr = 1;

	blit_para.src_image_h.clip_rect.x = 0;
	blit_para.src_image_h.clip_rect.y = 0;
	blit_para.src_image_h.clip_rect.w = width;
	blit_para.src_image_h.clip_rect.h = height;

	blit_para.dst_image_h.clip_rect.x = 0;
	blit_para.dst_image_h.clip_rect.y = 0;
	blit_para.dst_image_h.clip_rect.w = width;
	blit_para.dst_image_h.clip_rect.h = height;

	blit_para.src_image_h.format = G2D_FORMAT_YUV420_PLANAR;
	blit_para.src_image_h.width = width;
	blit_para.src_image_h.height = height;

	if (outputType == JpegDecodeOutputDataRGB565)
		blit_para.dst_image_h.format = G2D_FORMAT_RGB565;
	else
		blit_para.dst_image_h.format = G2D_FORMAT_ARGB8888;
	blit_para.dst_image_h.width = width;
	blit_para.dst_image_h.height = height;

	blit_para.flag_h = G2D_BLT_NONE_H;

	blit_para.dst_image_h.color = 0xee8899;
	blit_para.dst_image_h.mode = G2D_PIXEL_ALPHA;
	blit_para.dst_image_h.alpha = 255;
	blit_para.src_image_h.color = 0xee8899;
	blit_para.src_image_h.mode = G2D_PIXEL_ALPHA;
	blit_para.src_image_h.alpha = 255;

	if (ioctl(g_g2dfd, G2D_CMD_BITBLT_H, (uintptr_t)(&blit_para)) < 0) {
		printf("[%s,%d]: g2d G2D_CMD_BITBLT_H fail\n", __func__, __LINE__);
		return -1;
	}

	return 0;
}

static int G2DtransformYU12toRGB(struct ScMemOpsS *memOps, VideoPicture *pPicture,
		ImgFrame* pImgFrame, JpegDecodeOutputDataType outputType)
{
    unsigned char*   pDst;
    unsigned char*   pSrcY;
    unsigned char*   pSrcU;
    unsigned char*   pSrcV;

    //* flush cache.
    CdcMemFlushCache(memOps, pPicture->pData0, pPicture->nWidth*pPicture->nHeight);
    CdcMemFlushCache(memOps, pPicture->pData1, pPicture->nWidth*pPicture->nHeight/4);
    CdcMemFlushCache(memOps, pPicture->pData2, pPicture->nWidth*pPicture->nHeight/4);

    //* set pointers.
    pDst  = (unsigned char*)pImgFrame->mRGBData;
    pSrcY = (unsigned char*)pPicture->pData0 + pPicture->nTopOffset * \
                pPicture->nLineStride + pPicture->nLeftOffset;

    pSrcU = (unsigned char*)pPicture->pData1 + (pPicture->nTopOffset/2) * \
                        (pPicture->nLineStride/2) + pPicture->nLeftOffset/2;

    pSrcV = (unsigned char*)pPicture->pData2 + (pPicture->nTopOffset/2) * \
                        (pPicture->nLineStride/2) + pPicture->nLeftOffset/2;

	pSrcY = CdcMemGetPhysicAddress(memOps, pSrcY);
	pSrcU = CdcMemGetPhysicAddress(memOps, pSrcU);
	pSrcV = CdcMemGetPhysicAddress(memOps, pSrcV);

    if (g2d_transform(pSrcY, pSrcU, pSrcV, pDst, pImgFrame->mRGBSize, pImgFrame->mDisplayWidth,
			pImgFrame->mDisplayHeight, outputType) < 0) {
		return -1;
	}

    return 0;
}
#endif

static int transformYU12toRGB565(struct ScMemOpsS *memOps,
                                 VideoPicture  * pPicture,
                                 ImgFrame* pImgFrame)
{
    unsigned char*   pDst;
    unsigned char*   pSrcY;
    unsigned char*   pSrcU;
    unsigned char*   pSrcV;
    int              y;
    int              x;
    unsigned char*   pClipTable;
    unsigned char*   pClip;

    static const int nClipMin = -278;
    static const int nClipMax = 535;

    //* initialize the clip table.
    pClipTable = (unsigned char*)lv_mem_alloc(nClipMax - nClipMin + 1);
    if(pClipTable == NULL)
    {
        LV_LOG_WARN("can not allocate memory for the clip table, quit.\n");
        return -1;
    }
    for(x=nClipMin; x<=nClipMax; x++)
    {
        pClipTable[x-nClipMin] = (x<0) ? 0 : (x>255) ? 255 : x;
    }
    pClip = &pClipTable[-nClipMin];

    //* flush cache.
    CdcMemFlushCache(memOps, pPicture->pData0, pPicture->nWidth*pPicture->nHeight);
    CdcMemFlushCache(memOps, pPicture->pData1, pPicture->nWidth*pPicture->nHeight/4);
    CdcMemFlushCache(memOps, pPicture->pData2, pPicture->nWidth*pPicture->nHeight/4);

    //* set pointers.
    pDst  = (unsigned char*)pImgFrame->mRGBData;
    pSrcY = (unsigned char*)pPicture->pData0 + pPicture->nTopOffset * \
                pPicture->nLineStride + pPicture->nLeftOffset;

    pSrcU = (unsigned char*)pPicture->pData1 + (pPicture->nTopOffset/2) * \
                        (pPicture->nLineStride/2) + pPicture->nLeftOffset/2;

    pSrcV = (unsigned char*)pPicture->pData2 + (pPicture->nTopOffset/2) * \
                        (pPicture->nLineStride/2) + pPicture->nLeftOffset/2;

    for(y = 0; y < (int)pImgFrame->mDisplayHeight; ++y)
    {
        for(x = 0; x < (int)pImgFrame->mDisplayWidth; x += 2)
        {
            // B = 1.164 * (Y - 16) + 2.018 * (U - 128)
            // G = 1.164 * (Y - 16) - 0.813 * (V - 128) - 0.391 * (U - 128)
            // R = 1.164 * (Y - 16) + 1.596 * (V - 128)

            // B = 298/256 * (Y - 16) + 517/256 * (U - 128)
            // G = .................. - 208/256 * (V - 128) - 100/256 * (U - 128)
            // R = .................. + 409/256 * (V - 128)

            // min_B = (298 * (- 16) + 517 * (- 128)) / 256 = -277
            // min_G = (298 * (- 16) - 208 * (255 - 128) - 100 * (255 - 128)) / 256 = -172
            // min_R = (298 * (- 16) + 409 * (- 128)) / 256 = -223

            // max_B = (298 * (255 - 16) + 517 * (255 - 128)) / 256 = 534
            // max_G = (298 * (255 - 16) - 208 * (- 128) - 100 * (- 128)) / 256 = 432
            // max_R = (298 * (255 - 16) + 409 * (255 - 128)) / 256 = 481

            // clip range -278 .. 535

            signed y1 = (signed)pSrcY[x] - 16;
            signed y2 = (signed)pSrcY[x + 1] - 16;

            signed u = (signed)pSrcU[x / 2] - 128;
            signed v = (signed)pSrcV[x / 2] - 128;

            signed u_b = u * 517;
            signed u_g = -u * 100;
            signed v_g = -v * 208;
            signed v_r = v * 409;

            signed tmp1 = y1 * 298;
            signed b1 = (tmp1 + u_b) / 256;
            signed g1 = (tmp1 + v_g + u_g) / 256;
            signed r1 = (tmp1 + v_r) / 256;

            signed tmp2 = y2 * 298;
            signed b2 = (tmp2 + u_b) / 256;
            signed g2 = (tmp2 + v_g + u_g) / 256;
            signed r2 = (tmp2 + v_r) / 256;

            unsigned int rgb1 = ((pClip[r1] >> 3) << 11) |
                                ((pClip[g1] >> 2) << 5)  |
                                (pClip[b1] >> 3);

            unsigned int rgb2 = ((pClip[r2] >> 3) << 11) |
                                ((pClip[g2] >> 2) << 5)  |
                                (pClip[b2] >> 3);

            *(unsigned int *)(&pDst[x]) = (rgb2 << 16) | rgb1;
        }

        pSrcY += pImgFrame->mDisplayWidth;

        if(y & 1)
        {
            pSrcU += pImgFrame->mDisplayWidth / 2;
            pSrcV += pImgFrame->mDisplayWidth / 2;
        }

        pDst += pImgFrame->mDisplayWidth;
    }

    if (pClipTable)
    {
        lv_mem_free(pClipTable);
        pClipTable = NULL;
    }

    return 0;
}


static int Table_fv1[256] = {
	-180, -179, -177, -176, -174, -173, -172, -170, -169, -167, -166, -165,
	-163, -162, -160, -159, -158, -156, -155, -153, -152, -151, -149, -148,
	-146, -145, -144, -142, -141, -139, -138, -137, -135, -134, -132, -131,
	-130, -128, -127, -125, -124, -123, -121, -120, -118, -117, -115, -114,
	-113, -111, -110, -108, -107, -106, -104, -103, -101, -100, -99, -97,
	-96, -94, -93, -92, -90, -89, -87, -86, -85, -83, -82, -80, -79, -78,
	-76, -75, -73, -72, -71, -69, -68, -66, -65, -64, -62, -61, -59, -58,
	-57, -55, -54, -52, -51, -50, -48, -47, -45, -44, -43, -41, -40, -38,
	-37, -36, -34, -33, -31, -30, -29, -27, -26, -24, -23, -22, -20, -19,
	-17, -16, -15, -13, -12, -10, -9, -8, -6, -5, -3, -2, 0, 1, 2, 4, 5, 7,
	8, 9, 11, 12, 14, 15, 16, 18, 19, 21, 22, 23, 25, 26, 28, 29, 30, 32,
	33, 35, 36, 37, 39, 40, 42, 43, 44, 46, 47, 49, 50, 51, 53, 54, 56, 57,
	58, 60, 61, 63, 64, 65, 67, 68, 70, 71, 72, 74, 75, 77, 78, 79, 81, 82,
	84, 85, 86, 88, 89, 91, 92, 93, 95, 96, 98, 99, 100, 102, 103, 105,
	106, 107, 109, 110, 112, 113, 114, 116, 117, 119, 120, 122, 123, 124,
	126, 127, 129, 130, 131, 133, 134, 136, 137, 138, 140, 141, 143, 144,
	145, 147, 148, 150, 151, 152, 154, 155, 157, 158, 159, 161, 162, 164,
	165, 166, 168, 169, 171, 172, 173, 175, 176, 178
};
static int Table_fv2[256] = {
	-92, -91, -91, -90, -89, -88, -88, -87, -86, -86, -85, -84, -83, -83,
	-82, -81, -81, -80, -79, -78, -78, -77, -76, -76, -75, -74, -73, -73,
	-72, -71, -71, -70, -69, -68, -68, -67, -66, -66, -65, -64, -63, -63,
	-62, -61, -61, -60, -59, -58, -58, -57, -56, -56, -55, -54, -53, -53,
	-52, -51, -51, -50, -49, -48, -48, -47, -46, -46, -45, -44, -43, -43,
	-42, -41, -41, -40, -39, -38, -38, -37, -36, -36, -35, -34, -33, -33,
	-32, -31, -31, -30, -29, -28, -28, -27, -26, -26, -25, -24, -23, -23,
	-22, -21, -21, -20, -19, -18, -18, -17, -16, -16, -15, -14, -13, -13,
	-12, -11, -11, -10, -9, -8, -8, -7, -6, -6, -5, -4, -3, -3, -2, -1, 0,
	0, 1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 8, 9, 10, 10, 11, 12, 12, 13, 14, 15,
	15, 16, 17, 17, 18, 19, 20, 20, 21, 22, 22, 23, 24, 25, 25, 26, 27, 27,
	28, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 37, 38, 39, 40, 40,
	41, 42, 42, 43, 44, 45, 45, 46, 47, 47, 48, 49, 50, 50, 51, 52, 52, 53,
	54, 55, 55, 56, 57, 57, 58, 59, 60, 60, 61, 62, 62, 63, 64, 65, 65, 66,
	67, 67, 68, 69, 70, 70, 71, 72, 72, 73, 74, 75, 75, 76, 77, 77, 78, 79,
	80, 80, 81, 82, 82, 83, 84, 85, 85, 86, 87, 87, 88, 89, 90, 90
};
static int Table_fu1[256] = {
	-44, -44, -44, -43, -43, -43, -42, -42, -42, -41, -41, -41, -40, -40,
	-40, -39, -39, -39, -38, -38, -38, -37, -37, -37, -36, -36, -36, -35,
	-35, -35, -34, -34, -33, -33, -33, -32, -32, -32, -31, -31, -31, -30,
	-30, -30, -29, -29, -29, -28, -28, -28, -27, -27, -27, -26, -26, -26,
	-25, -25, -25, -24, -24, -24, -23, -23, -22, -22, -22, -21, -21, -21,
	-20, -20, -20, -19, -19, -19, -18, -18, -18, -17, -17, -17, -16, -16,
	-16, -15, -15, -15, -14, -14, -14, -13, -13, -13, -12, -12, -11, -11,
	-11, -10, -10, -10, -9, -9, -9, -8, -8, -8, -7, -7, -7, -6, -6, -6, -5,
	-5, -5, -4, -4, -4, -3, -3, -3, -2, -2, -2, -1, -1, 0, 0, 0, 1, 1, 1,
	2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9,
	10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16,
	16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 22, 22,
	22, 23, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28,
	28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 32, 32, 33, 33, 33, 34, 34, 34,
	35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 40, 40, 40,
	41, 41, 41, 42, 42, 42, 43, 43
};
static int Table_fu2[256] = {
	-227, -226, -224, -222, -220, -219, -217, -215, -213, -212, -210, -208,
	-206, -204, -203, -201, -199, -197, -196, -194, -192, -190, -188, -187,
	-185, -183, -181, -180, -178, -176, -174, -173, -171, -169, -167, -165,
	-164, -162, -160, -158, -157, -155, -153, -151, -149, -148, -146, -144,
	-142, -141, -139, -137, -135, -134, -132, -130, -128, -126, -125, -123,
	-121, -119, -118, -116, -114, -112, -110, -109, -107, -105, -103, -102,
	-100, -98, -96, -94, -93, -91, -89, -87, -86, -84, -82, -80, -79, -77,
	-75, -73, -71, -70, -68, -66, -64, -63, -61, -59, -57, -55, -54, -52,
	-50, -48, -47, -45, -43, -41, -40, -38, -36, -34, -32, -31, -29, -27,
	-25, -24, -22, -20, -18, -16, -15, -13, -11, -9, -8, -6, -4, -2, 0, 1,
	3, 5, 7, 8, 10, 12, 14, 15, 17, 19, 21, 23, 24, 26, 28, 30, 31, 33, 35,
	37, 39, 40, 42, 44, 46, 47, 49, 51, 53, 54, 56, 58, 60, 62, 63, 65, 67,
	69, 70, 72, 74, 76, 78, 79, 81, 83, 85, 86, 88, 90, 92, 93, 95, 97, 99,
	101, 102, 104, 106, 108, 109, 111, 113, 115, 117, 118, 120, 122, 124,
	125, 127, 129, 131, 133, 134, 136, 138, 140, 141, 143, 145, 147, 148,
	150, 152, 154, 156, 157, 159, 161, 163, 164, 166, 168, 170, 172, 173,
	175, 177, 179, 180, 182, 184, 186, 187, 189, 191, 193, 195, 196, 198,
	200, 202, 203, 205, 207, 209, 211, 212, 214, 216, 218, 219, 221, 223,
	225
};

static int transformYU12toARGB8888(struct ScMemOpsS *memOps,
                                 VideoPicture  * pPicture,
                                 ImgFrame* pImgFrame)
{
    unsigned char*   pDst;
    unsigned char*   pSrcY;
    unsigned char*   pSrcU;
    unsigned char*   pSrcV;
    int              y;
    int              x;

    //* flush cache.
    CdcMemFlushCache(memOps, pPicture->pData0, pPicture->nWidth*pPicture->nHeight);
    CdcMemFlushCache(memOps, pPicture->pData1, pPicture->nWidth*pPicture->nHeight/4);
    CdcMemFlushCache(memOps, pPicture->pData2, pPicture->nWidth*pPicture->nHeight/4);

    //* set pointers.
    pDst  = (unsigned char*)pImgFrame->mRGBData;
    pSrcY = (unsigned char*)pPicture->pData0 + pPicture->nTopOffset * \
                pPicture->nLineStride + pPicture->nLeftOffset;

    pSrcU = (unsigned char*)pPicture->pData1 + (pPicture->nTopOffset/2) * \
                        (pPicture->nLineStride/2) + pPicture->nLeftOffset/2;

    pSrcV = (unsigned char*)pPicture->pData2 + (pPicture->nTopOffset/2) * \
                        (pPicture->nLineStride/2) + pPicture->nLeftOffset/2;

    int argb[4];
    int yIdx, uIdx, vIdx, idx;
    int rdif, invgdif, bdif;
    for (int i = 0;i < (int)pImgFrame->mDisplayHeight;i++){
        for (int j = 0;j < (int)pImgFrame->mDisplayWidth;j++){
            yIdx = i * pImgFrame->mDisplayWidth + j;
            vIdx = (i/2) * (pImgFrame->mDisplayWidth/2) + (j/2);
            uIdx = vIdx;

            rdif = Table_fv1[pSrcU[vIdx]];
            invgdif = Table_fu1[pSrcV[uIdx]] + Table_fv2[pSrcU[vIdx]];
            bdif = Table_fu2[pSrcV[uIdx]];

            argb[0] = pSrcY[yIdx] + rdif;
            argb[1] = pSrcY[yIdx] - invgdif;
            argb[2] = pSrcY[yIdx] + bdif;
            argb[3] = 255;

            for (int k = 0;k < 4;k++){
                idx = (i * pImgFrame->mDisplayWidth + j) * 4 + k;
                if(argb[k] >= 0 && argb[k] <= 255)
                    pDst[idx] = argb[k];
                else
                    pDst[idx] = (argb[k] < 0)?0:255;
            }
        }
    }

    return 0;
}

int sunxijpgd_load_file(char** out, int* outsize, const char* filename)
{
    lv_fs_file_t lv_file;
    int jpg_size;

    lv_fs_res_t res = lv_fs_open(&lv_file, filename, LV_FS_MODE_RD);
    if(res != LV_FS_RES_OK) {
        return -1;
    }
    if(lv_fs_seek(&lv_file, 0, LV_FS_SEEK_END) != 0) {
        lv_fs_close(&lv_file);
        return -1;
    }
    lv_fs_tell(&lv_file, &jpg_size);
    lv_fs_close(&lv_file);
    *outsize = jpg_size;

    *out = lv_mem_alloc(jpg_size);
    res = lv_fs_open(&lv_file, filename, LV_FS_MODE_RD);
    if(res != LV_FS_RES_OK) {
        lv_mem_free(*out);
        return -1;
    }
	if(lv_fs_read(&lv_file, *out, jpg_size, NULL) != 0) {
        lv_fs_close(&lv_file);
        lv_mem_free(*out);
        return -1;
    }
    lv_fs_close(&lv_file);

    return 0;
}

static void setDecoderPara(JpegDecoderContext* p, VideoStreamInfo* vStreamInfo, VConfig* vConfig)
{
    vStreamInfo->eCodecFormat = VIDEO_CODEC_FORMAT_MJPEG;
    vConfig->bDisable3D = 0;
    vConfig->bDispErrorFrame = 0;
    vConfig->bNoBFrames = 0;
    vConfig->bRotationEn = 0;
    vConfig->bScaleDownEn = p->mScaleDownEn;
    vConfig->nHorizonScaleDownRatio = p->mHorizonScaleDownRatio;
    vConfig->nVerticalScaleDownRatio = p->mVerticalScaleDownRatio;
    vConfig->eOutputPixelFormat = p->mDecodedPixFormat;
    vConfig->nDeInterlaceHoldingFrameBufferNum = 0;
    vConfig->nDisplayHoldingFrameBufferNum = 0;
    vConfig->nRotateHoldingFrameBufferNum = 0;
    vConfig->nDecodeSmoothFrameBufferNum = 0;
    vConfig->nVbvBufferSize = p->mSrcBufLen + 2*1024*1024;
    vConfig->bThumbnailMode = 1;
    vConfig->memops = p->memops;
}

static long long GetNowMs()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static long long GetNowUs()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)tv.tv_sec * 1000000 + tv.tv_usec;
}

JpegDecoder* JpegDecoderCreate()
{
    JpegDecoderContext* p;
    p = (JpegDecoderContext*)malloc(sizeof(JpegDecoderContext));
    if(!p)
    {
        LV_LOG_WARN("JpegDecoderContext malloc fail\n");
        return NULL;
    }
    memset(p, 0x00, sizeof(JpegDecoderContext));

    p->memops = MemAdapterGetOpsS();
    if(p->memops == NULL)
    {
        free(p);
        LV_LOG_WARN("MemAdapterGetOpsS fail\n");
        return NULL;
    }
    CdcMemOpen(p->memops);
    AddVDPlugin();

    return (JpegDecoder*)p;
}

void JpegDecoderDestory(JpegDecoder* v)
{
    JpegDecoderContext* p;
    p = (JpegDecoderContext*)v;
    if(p->mVideoDecoder != NULL)
    {
        DestroyVideoDecoder(p->mVideoDecoder);
        p->mVideoDecoder = NULL;
    }
    if(p->mImgFrame.mRGBData)
    {
#ifdef USE_SUNXIFB_G2D
        sunxifb_mem_free((void **) &(p->mImgFrame.mRGBData), "sunxijpgd_close");
#else
        lv_mem_free(p->mImgFrame.mRGBData);
        p->mImgFrame.mRGBData = NULL;
#endif
    }
    memset(&p->mImgFrame, 0x00, sizeof(ImgFrame));
#if 0
    if(p->memops){
        CdcMemClose(p->memops);
        p->memops = NULL;
    }
    free(p);
#endif
}

void JpegDecoderSetDataSourceBuf(JpegDecoder* v, char* buffer, int bufLen,
		JpegDecodeScaleDownRatio scaleRatio, JpegDecodeOutputDataType outputType)
{
    JpegDecoderContext* p;
    p = (JpegDecoderContext*)v;
    p->mSrcBuf = buffer;
    p->mSrcBufLen = bufLen;
    p->mOutputDataType = outputType;
    p->mDecodedPixFormat = PIXEL_FORMAT_YUV_PLANER_420;
    switch (scaleRatio)
    {
        case JPEG_DECODE_SCALE_DOWN_1:
            p->mScaleDownEn = 0;
            p->mHorizonScaleDownRatio = 0;
            p->mVerticalScaleDownRatio = 0;
            break;
        case JPEG_DECODE_SCALE_DOWN_2:
            p->mScaleDownEn = 1;
            p->mHorizonScaleDownRatio = 1;
            p->mVerticalScaleDownRatio = 1;
            break;
        case JPEG_DECODE_SCALE_DOWN_4:
            p->mScaleDownEn = 1;
            p->mHorizonScaleDownRatio = 2;
            p->mVerticalScaleDownRatio = 2;
            break;
        case JPEG_DECODE_SCALE_DOWN_8:
            p->mScaleDownEn = 1;
            p->mHorizonScaleDownRatio = 3;
            p->mVerticalScaleDownRatio = 3;
            break;
        default:
            LV_LOG_WARN("the scalRatio = %d,which is not support,use the default\n", scaleRatio);
            p->mScaleDownEn = 0;
            p->mHorizonScaleDownRatio = 0;
            p->mVerticalScaleDownRatio = 0;
            break;
    }
}

ImgFrame *JpegDecoderGetFrame(JpegDecoder* v)
{
    JpegDecoderContext* p;
    p = (JpegDecoderContext*)v;
    VideoStreamInfo vStreamInfo;
    VConfig vConfig;
    char *jpegData = NULL;
    int dataLen =0;
    char *buf, *ringBuf;
    int buflen, ringBufLen;
    VideoPicture* videoPicture;

    /**read jpeg data**/
    if(p->mSrcBuf && p->mSrcBufLen) {
        jpegData = p->mSrcBuf;
        dataLen = p->mSrcBufLen;
    } else {
        LV_LOG_WARN("the input source has error\n");
        return NULL;
    }

    /**create the video decoder**/
    p->mVideoDecoder = CreateVideoDecoder();
    if(!p->mVideoDecoder) {
        LV_LOG_WARN("create video decoder failed\n");
        return NULL;
    }

    /**init the video decoder parameter**/
    memset(&vStreamInfo,0,sizeof(VideoStreamInfo));
    memset(&vConfig,0,sizeof(vConfig));
    setDecoderPara(p,&vStreamInfo,&vConfig);
    if ((InitializeVideoDecoder(p->mVideoDecoder, &vStreamInfo, &vConfig)) != 0)
    {
        LV_LOG_WARN("InitializeVideoDecoder failed\n");
        DestroyVideoDecoder(p->mVideoDecoder);
        p->mVideoDecoder = NULL;
        return NULL;
    }

    /**request vbm buffer from video decoder**/
     if(RequestVideoStreamBuffer(p->mVideoDecoder,
                                dataLen,
                                (char**)&buf,
                                &buflen,
                                (char**)&ringBuf,
                                &ringBufLen,
                                0))
    {
        LV_LOG_WARN("Request Video Stream Buffer failed\n");
        DestroyVideoDecoder(p->mVideoDecoder);
        p->mVideoDecoder = NULL;
        return NULL;
    }

    if(buflen + ringBufLen < dataLen)
    {
        LV_LOG_WARN("#####Error: request buffer failed, buffer is not enough\n");
        DestroyVideoDecoder(p->mVideoDecoder);
        p->mVideoDecoder = NULL;
        return NULL;
    }

    /**copy and submit stream to video decoder SBM**/
    if(buflen >= dataLen)
    {
        memcpy(buf,jpegData,dataLen);
    }
    else
    {
        memcpy(buf,jpegData,buflen);
        memcpy(ringBuf,jpegData+buflen,dataLen-buflen);
    }

    VideoStreamDataInfo DataInfo;
    memset(&DataInfo, 0, sizeof(DataInfo));
    DataInfo.pData = buf;
    DataInfo.nLength = dataLen;
    DataInfo.bIsFirstPart = 1;
    DataInfo.bIsLastPart = 1;
    DataInfo.bValid = 1;

    if (SubmitVideoStreamData(p->mVideoDecoder, &DataInfo, 0))
    {
        LV_LOG_WARN("#####Error: Submit Video Stream Data failed!\n");
        DestroyVideoDecoder(p->mVideoDecoder);
        p->mVideoDecoder = NULL;
        return NULL;
    }

    /**decode stream**/
    int endofstream = 0;
    int dropBFrameifdelay = 0;
    int64_t currenttimeus = 0;
    int decodekeyframeonly = 0;
    int ret = DecodeVideoStream(p->mVideoDecoder, endofstream, decodekeyframeonly,
                        dropBFrameifdelay, currenttimeus);
    switch (ret)
    {
        case VDECODE_RESULT_KEYFRAME_DECODED:
        case VDECODE_RESULT_FRAME_DECODED:
        case VDECODE_RESULT_NO_FRAME_BUFFER:
        {
            ret = ValidPictureNum(p->mVideoDecoder, 0);
            if (ret>= 0)
            {
                videoPicture = RequestPicture(p->mVideoDecoder, 0);
                if (videoPicture == NULL) {
                    LV_LOG_WARN("RequestPicture fail\n");
                    DestroyVideoDecoder(p->mVideoDecoder);
                    p->mVideoDecoder = NULL;
                    return NULL;
                }

                //if the mRGBData is not NULL,we should free the buffer which is malloc before
                if (p->mImgFrame.mRGBData != NULL) {
#ifdef USE_SUNXIFB_G2D
                    sunxifb_mem_free((void **) &(p->mImgFrame.mRGBData), "sunxijpgd");
#else
                    lv_mem_free(p->mImgFrame.mRGBData);
#endif
                    memset(&p->mImgFrame, 0x00, sizeof(ImgFrame));
                }

                p->mImgFrame.mWidth = videoPicture->nWidth;
                p->mImgFrame.mHeight = videoPicture->nHeight;
                p->mImgFrame.mDisplayWidth = videoPicture->nRightOffset - videoPicture->nLeftOffset;
                p->mImgFrame.mDisplayHeight = videoPicture->nBottomOffset - videoPicture->nTopOffset;
                p->mImgFrame.mRGBSize = p->mImgFrame.mDisplayWidth * p->mImgFrame.mDisplayHeight
					* LV_COLOR_DEPTH / 8;
#ifdef USE_SUNXIFB_G2D
                p->mImgFrame.mRGBData = (uint8_t*)sunxifb_mem_alloc(p->mImgFrame.mRGBSize, "sunxijpgd_load");
#else
                p->mImgFrame.mRGBData = (uint8_t*)lv_mem_alloc(p->mImgFrame.mRGBSize);
#endif
                if (p->mImgFrame.mRGBData == NULL)
                {
                    LV_LOG_WARN("imgFrame.mRGBData malloc fail\n");
                    DestroyVideoDecoder(p->mVideoDecoder);
                    p->mVideoDecoder = NULL;
                    return NULL;
                }

#ifdef USE_SUNXIFB_G2D
				G2DtransformYU12toRGB(p->memops, videoPicture, &p->mImgFrame, p->mOutputDataType);
#else
                if (p->mOutputDataType == JpegDecodeOutputDataRGB565) {
                    transformYU12toRGB565(p->memops, videoPicture, &p->mImgFrame);
				} else {
					transformYU12toARGB8888(p->memops, videoPicture, &p->mImgFrame);
				}
#endif
                /**return the picture buf to video decoder**/
                ReturnPicture(p->mVideoDecoder, videoPicture);

            } else {
               LV_LOG_WARN("no ValidPictureNum! ret is %d\n", ret);
            }
            break;
        }

        case VDECODE_RESULT_OK:
        case VDECODE_RESULT_CONTINUE:
        case VDECODE_RESULT_NO_BITSTREAM:
        case VDECODE_RESULT_RESOLUTION_CHANGE:
        case VDECODE_RESULT_UNSUPPORTED:
        default:
            LV_LOG_WARN("video decode Error: %d\n", ret);
            DestroyVideoDecoder(p->mVideoDecoder);
            p->mVideoDecoder = NULL;
            return NULL;
    }

    return &p->mImgFrame;
}

#endif /*LV_USE_SJPG*/
