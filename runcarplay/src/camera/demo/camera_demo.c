#include "camera_demo.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../camera_capture.h"
#include "../camera_overlay.h"
#include "../camera_preview.h"
#include "../camera_record.h"
#include "../camera_vi.h"

/*
 * 说明:
 * 该 demo 仅演示调用顺序与模块组合方式，便于后续移植项目直接照搬。
 * 这里不接入当前项目的业务调用链，不修改 mpp_camera。
 */

static camera_vi_attr_t make_default_vi_cfg(void)
{
    camera_vi_attr_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.width = 1280;
    cfg.height = 720;
    cfg.pixel_format = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    cfg.frame_rate = 25;
    cfg.vi_buf_num = 5;
    cfg.enable_wdr = 0;
    cfg.vi_drop_frame_num = 0;
    cfg.color_space = V4L2_COLORSPACE_JPEG;
    cfg.mirror = 0;
    cfg.flip = 0;
    return cfg;
}

static camera_preview_cfg_t make_default_preview_cfg(void)
{
    camera_preview_cfg_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.vo_dev = 0;
    cfg.disp_type = VO_INTF_LCD;
    cfg.disp_sync = VO_OUTPUT_NTSC;
    cfg.front_rect.x = 0;
    cfg.front_rect.y = 0;
    cfg.front_rect.width = 640;
    cfg.front_rect.height = 360;
    cfg.front_rect.zorder = 0;
    cfg.rear_rect.x = 640;
    cfg.rear_rect.y = 0;
    cfg.rear_rect.width = 640;
    cfg.rear_rect.height = 360;
    cfg.rear_rect.zorder = 1;
    return cfg;
}

static camera_record_cfg_t make_default_record_cfg(int dual)
{
    camera_record_cfg_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.front.width = 1280;
    cfg.front.height = 720;
    cfg.front.frame_rate = 25;
    cfg.front.bit_rate = 4 * 1024 * 1024;
    cfg.front.payload_type = PT_H264;
    cfg.front.segment_duration_s = 60;
#if CAMERA_ENABLE_SECOND_SENSOR
    cfg.rear = cfg.front;
#endif
    cfg.enable_dual = dual;
    cfg.output_dir_front = "/mnt/extsd/recorder/frontCamera";
    cfg.output_dir_rear = "/mnt/extsd/recorder/rearCamera";
    return cfg;
}

static camera_capture_cfg_t make_default_capture_cfg(void)
{
    camera_capture_cfg_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.width = 1280;
    cfg.height = 720;
    cfg.frame_rate = 25;
    cfg.jpeg_qfactor = 90;
    return cfg;
}

static int apply_overlay_text(VENC_CHN venc_chn, RGN_HANDLE handle, const char *text)
{
    camera_overlay_ctx_t overlay;
    camera_overlay_cfg_t ocfg;
    camera_bitmap_t bmp;
    int ret = -1;

    memset(&overlay, 0, sizeof(overlay));
    memset(&ocfg, 0, sizeof(ocfg));
    memset(&bmp, 0, sizeof(bmp));

    ocfg.x = 16;
    ocfg.y = 16;
    ocfg.width = 320;
    ocfg.height = 48;
    ocfg.layer = 0;
    ocfg.fg_alpha = 200;
    ocfg.bg_alpha = 80;

    if (camera_overlay_init(&overlay, venc_chn, handle, &ocfg) != 0) {
        return -1;
    }
    if (camera_overlay_start(&overlay) != 0) {
        camera_overlay_deinit(&overlay);
        return -1;
    }
    if (camera_overlay_text_to_bitmap(text, 16, 24, 0xFFFFFFFF, 0x80000000, &bmp) != 0) {
        camera_overlay_deinit(&overlay);
        return -1;
    }
    ret = camera_overlay_set_bitmap(&overlay, &bmp);
    camera_overlay_free_bitmap(&bmp);
    camera_overlay_stop(&overlay);
    camera_overlay_deinit(&overlay);
    return ret;
}

int camera_demo_run_single(void)
{
    camera_vi_ctx_t vi_ctx;
    camera_preview_ctx_t preview_ctx;
    camera_record_ctx_t record_ctx;
    camera_capture_ctx_t capture_ctx;
    camera_vi_attr_t front_vi = make_default_vi_cfg();
    camera_preview_cfg_t preview_cfg = make_default_preview_cfg();
    camera_record_cfg_t record_cfg = make_default_record_cfg(0);
    camera_capture_cfg_t cap_cfg = make_default_capture_cfg();

    memset(&vi_ctx, 0, sizeof(vi_ctx));
    memset(&preview_ctx, 0, sizeof(preview_ctx));
    memset(&record_ctx, 0, sizeof(record_ctx));
    memset(&capture_ctx, 0, sizeof(capture_ctx));

    if (camera_vi_init(&vi_ctx, &front_vi
#if CAMERA_ENABLE_SECOND_SENSOR
        , NULL
#endif
    ) != 0) {
        printf("demo(single): camera_vi_init failed\n");
        return -1;
    }
    if (camera_vi_start(&vi_ctx) != 0) {
        printf("demo(single): camera_vi_start failed\n");
        camera_vi_deinit(&vi_ctx);
        return -1;
    }

    if (camera_preview_init(&preview_ctx, &vi_ctx, &preview_cfg) == 0) {
        camera_preview_start(&preview_ctx, CAMERA_PREVIEW_FRONT);
    }

    if (camera_record_init(&record_ctx, &vi_ctx, &record_cfg) == 0) {
        camera_record_set_segment_params(&record_ctx, 1, 60, 0);
        camera_record_start(&record_ctx);
        apply_overlay_text(record_ctx.front.venc_chn, 100, "FRONT-REC");
    }

    if (camera_capture_init(&capture_ctx, &vi_ctx, CAMERA_SENSOR_FRONT, &cap_cfg) == 0) {
        camera_capture_start(&capture_ctx);
        camera_capture_take_with_filename(&capture_ctx, "/mnt/extsd/DVRpic/front_demo.jpg");
        camera_capture_stop(&capture_ctx);
        camera_capture_deinit(&capture_ctx);
    }

    sleep(3);

    camera_record_stop(&record_ctx);
    camera_record_deinit(&record_ctx);
    camera_preview_stop(&preview_ctx);
    camera_preview_deinit(&preview_ctx);
    camera_vi_stop(&vi_ctx);
    camera_vi_deinit(&vi_ctx);
    return 0;
}

int camera_demo_run_dual(void)
{
#if !CAMERA_ENABLE_SECOND_SENSOR
    printf("demo(dual): CAMERA_ENABLE_SECOND_SENSOR=0, dual mode disabled\n");
    return -1;
#else
    camera_vi_ctx_t vi_ctx;
    camera_preview_ctx_t preview_ctx;
    camera_record_ctx_t record_ctx;
    camera_capture_ctx_t cap_front;
    camera_capture_ctx_t cap_rear;
    camera_vi_attr_t front_vi = make_default_vi_cfg();
    camera_vi_attr_t rear_vi = make_default_vi_cfg();
    camera_preview_cfg_t preview_cfg = make_default_preview_cfg();
    camera_record_cfg_t record_cfg = make_default_record_cfg(1);
    camera_capture_cfg_t cap_cfg = make_default_capture_cfg();

    memset(&vi_ctx, 0, sizeof(vi_ctx));
    memset(&preview_ctx, 0, sizeof(preview_ctx));
    memset(&record_ctx, 0, sizeof(record_ctx));
    memset(&cap_front, 0, sizeof(cap_front));
    memset(&cap_rear, 0, sizeof(cap_rear));

    if (camera_vi_init(&vi_ctx, &front_vi, &rear_vi) != 0) {
        printf("demo(dual): camera_vi_init failed\n");
        return -1;
    }
    if (camera_vi_start(&vi_ctx) != 0) {
        printf("demo(dual): camera_vi_start failed\n");
        camera_vi_deinit(&vi_ctx);
        return -1;
    }

    if (camera_preview_init(&preview_ctx, &vi_ctx, &preview_cfg) == 0) {
        camera_preview_start(&preview_ctx, CAMERA_PREVIEW_DUAL);
    }

    if (camera_record_init(&record_ctx, &vi_ctx, &record_cfg) == 0) {
        camera_record_set_segment_params(&record_ctx, 1, 60, 60);
        camera_record_start(&record_ctx);
        apply_overlay_text(record_ctx.front.venc_chn, 101, "FRONT");
        apply_overlay_text(record_ctx.rear.venc_chn, 102, "REAR");
    }

    if (camera_capture_init(&cap_front, &vi_ctx, CAMERA_SENSOR_FRONT, &cap_cfg) == 0) {
        camera_capture_start(&cap_front);
        camera_capture_take_with_filename(&cap_front, "/mnt/extsd/DVRpic/front_demo_dual.jpg");
        camera_capture_stop(&cap_front);
        camera_capture_deinit(&cap_front);
    }

    if (camera_capture_init(&cap_rear, &vi_ctx, CAMERA_SENSOR_REAR, &cap_cfg) == 0) {
        camera_capture_start(&cap_rear);
        camera_capture_take_with_filename(&cap_rear, "/mnt/extsd/DVRpic/rear_demo_dual.jpg");
        camera_capture_stop(&cap_rear);
        camera_capture_deinit(&cap_rear);
    }

    sleep(3);

    camera_record_stop(&record_ctx);
    camera_record_deinit(&record_ctx);
    camera_preview_stop(&preview_ctx);
    camera_preview_deinit(&preview_ctx);
    camera_vi_stop(&vi_ctx);
    camera_vi_deinit(&vi_ctx);
    return 0;
#endif
}
