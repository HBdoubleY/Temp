#include "events_init.h"
#include <stdio.h>
#include <string.h>
#include "lvgl.h"
#include "pthread.h"
#include <stdint.h>
#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif
#include "hwdisplay.h"
#include "lv_obj_draw.h"
#include "Rtc2C.h"
#include "media_main_interface.h"
#include "ota_main_interface.h"
#include "storageDataApi.h"
#include "tire_manager.h"
#include "lvgl_main.h"
static void tire_pair_popup_close(void);

static void screen_Tire_btn_return_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del, &guider_ui.screen_Tire_del, setup_scr_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true, true);
		break;
	}
    default:
        break;
    }
}

static void screen_Tire_btn_lPressMax_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		tire_alarm_adjust_pressure_max(-1);
		break;
	}
    default:
        break;
    }
}

static void screen_Tire_btn_rPressMin_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		tire_alarm_adjust_pressure_min(1);
		break;
	}
    default:
        break;
    }
}

static void screen_Tire_btn_lPressMin_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		tire_alarm_adjust_pressure_min(-1);
		break;
	}
    default:
        break;
    }
}

static void screen_Tire_btn_rPressMax_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		tire_alarm_adjust_pressure_max(1);
		break;
	}
    default:
        break;
    }
}

static void screen_Tire_btn_lTemp_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		tire_alarm_adjust_temp_max(-1);
		break;
	}
    default:
        break;
    }
}

static void screen_Tire_btn_rTemp_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		tire_alarm_adjust_temp_max(1);
		break;
	}
    default:
        break;
    }
}

static void screen_Tire_btn_pressBar_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(!g_sys_Data.pressureUnit)
			return;
		tire_alarm_set_pressure_unit(false);
		g_sys_Data.bPressure = PressureUnitConversion(g_sys_Data.bPressure);
		g_sys_Data.fPressure = PressureUnitConversion(g_sys_Data.fPressure);		
		if(g_sys_Data.themeMode == THEME_DAY){
			lv_img_set_src(guider_ui.screen_Tire_img_pressBar,&_selected_40x40);
			lv_img_set_src(guider_ui.screen_Tire_img_pressPsj,&_not_select_40x40);
		}else if(g_sys_Data.themeMode == THEME_DARK){
			lv_img_set_src(guider_ui.screen_Tire_img_pressBar,&_selected_dark_40x40);
			lv_img_set_src(guider_ui.screen_Tire_img_pressPsj,&_not_select_dark_40x40);
		}

		lv_obj_invalidate(guider_ui.screen_Tire_img_pressBar);
		lv_obj_invalidate(guider_ui.screen_Tire_img_pressPsj);	
		char str[20];
		sprintf(str,"%.1f Bar",g_sys_Data.fPressure);
		lv_label_set_text(guider_ui.screen_Tire_label_fPressure,str);
		sprintf(str,"%.1f Bar",g_sys_Data.bPressure);
		lv_label_set_text(guider_ui.screen_Tire_label_bPressure,str);
		lv_obj_invalidate(guider_ui.screen_Tire_label_bPressure);
		lv_obj_invalidate(guider_ui.screen_Tire_label_fPressure);
		tire_alarm_refresh_threshold_ui();
		// 单位切换后重新按“是否有最新 BLE 数据”刷新显示（避免显示旧值）
		tire_ui_refresh_now();
		break;
	}
    default:
        break;
    }
}

static void screen_Tire_btn_pressPsj_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(g_sys_Data.pressureUnit)
			return;
		tire_alarm_set_pressure_unit(true);
		g_sys_Data.bPressure = PressureUnitConversion(g_sys_Data.bPressure);
		g_sys_Data.fPressure = PressureUnitConversion(g_sys_Data.fPressure);
		if(g_sys_Data.themeMode == THEME_DAY){
			lv_img_set_src(guider_ui.screen_Tire_img_pressPsj,&_selected_40x40);
			lv_img_set_src(guider_ui.screen_Tire_img_pressBar,&_not_select_40x40);
		}else if(g_sys_Data.themeMode == THEME_DARK){
			lv_img_set_src(guider_ui.screen_Tire_img_pressPsj,&_selected_dark_40x40);
			lv_img_set_src(guider_ui.screen_Tire_img_pressBar,&_not_select_dark_40x40);
		}

		lv_obj_invalidate(guider_ui.screen_Tire_img_pressBar);
		lv_obj_invalidate(guider_ui.screen_Tire_img_pressPsj);
		char str[20];
		sprintf(str,"%.0f Psi",g_sys_Data.fPressure);
		lv_label_set_text(guider_ui.screen_Tire_label_fPressure,str);
		sprintf(str,"%.0f Psi",g_sys_Data.bPressure);
		lv_label_set_text(guider_ui.screen_Tire_label_bPressure,str);
		lv_obj_invalidate(guider_ui.screen_Tire_label_bPressure);
		lv_obj_invalidate(guider_ui.screen_Tire_label_fPressure);
		tire_alarm_refresh_threshold_ui();
		// 单位切换后重新按“是否有最新 BLE 数据”刷新显示（避免显示旧值）
		tire_ui_refresh_now();
		break;
	}
    default:
        break;
    }
}

static void screen_Tire_btn_tempC_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(!g_sys_Data.tempUnit)
			return;
		tire_alarm_set_temp_unit(false);
		g_sys_Data.bTemp = TempUnitConversion(g_sys_Data.bTemp);
		g_sys_Data.fTemp = TempUnitConversion(g_sys_Data.fTemp);
		if(g_sys_Data.themeMode == THEME_DAY){
			lv_img_set_src(guider_ui.screen_Tire_img_tempC,&_selected_40x40);
			lv_img_set_src(guider_ui.screen_Tire_img_tempF,&_not_select_40x40);
		}else if(g_sys_Data.themeMode == THEME_DARK){
			lv_img_set_src(guider_ui.screen_Tire_img_tempC,&_selected_dark_40x40);
			lv_img_set_src(guider_ui.screen_Tire_img_tempF,&_not_select_dark_40x40);
		}

		lv_obj_invalidate(guider_ui.screen_Tire_img_tempC);
		lv_obj_invalidate(guider_ui.screen_Tire_img_tempF);
		char str[20];
		sprintf(str,"%d ℃",g_sys_Data.fTemp);
		lv_label_set_text(guider_ui.screen_Tire_label_fTemp,str);
		sprintf(str,"%d ℃",g_sys_Data.bTemp);
		lv_label_set_text(guider_ui.screen_Tire_label_bTemp,str);
		lv_obj_invalidate(guider_ui.screen_Tire_label_fTemp);
		lv_obj_invalidate(guider_ui.screen_Tire_label_bTemp);
		tire_alarm_refresh_threshold_ui();
		// 单位切换后重新按“是否有最新 BLE 数据”刷新显示（避免显示旧值）
		tire_ui_refresh_now();
		break;
	}
    default:
        break;
    }
}

static void screen_Tire_btn_tempF_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(g_sys_Data.tempUnit)
			return;
		tire_alarm_set_temp_unit(true);
		g_sys_Data.bTemp = TempUnitConversion(g_sys_Data.bTemp);
		g_sys_Data.fTemp = TempUnitConversion(g_sys_Data.fTemp);	
		if(g_sys_Data.themeMode == THEME_DAY){
			lv_img_set_src(guider_ui.screen_Tire_img_tempF,&_selected_40x40);
			lv_img_set_src(guider_ui.screen_Tire_img_tempC,&_not_select_40x40);
		}else if(g_sys_Data.themeMode == THEME_DARK){
			lv_img_set_src(guider_ui.screen_Tire_img_tempF,&_selected_dark_40x40);
			lv_img_set_src(guider_ui.screen_Tire_img_tempC,&_not_select_dark_40x40);
		}
		lv_obj_invalidate(guider_ui.screen_Tire_img_tempC);
		lv_obj_invalidate(guider_ui.screen_Tire_img_tempF);
		char str[20];
		sprintf(str,"%d ℉",g_sys_Data.fTemp);
		lv_label_set_text(guider_ui.screen_Tire_label_fTemp,str);
		sprintf(str,"%d ℉",g_sys_Data.bTemp);
		lv_label_set_text(guider_ui.screen_Tire_label_bTemp,str);
		lv_obj_invalidate(guider_ui.screen_Tire_label_fTemp);
		lv_obj_invalidate(guider_ui.screen_Tire_label_bTemp);	
		// 单位切换后重新按“是否有最新 BLE 数据”刷新显示（避免显示旧值）
		tire_ui_refresh_now();
		break;
	}
    default:
        break;
    }
}

static void screen_Tire_btn_paraReset_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		tire_alarm_reset_defaults();
		break;
	}
    default:
        break;
    }
}

static void screen_Tire_btn_clearPair_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
	case LV_EVENT_CLICKED:
	{
		//by jy:clear the bind devices
		tire_pair_clear_all();
		tire_pair_popup_close();
		tire_ui_refresh_now();
		break;
	}
    default:
        break;
    }
}

/*============================
 * 胎压配对弹窗（自定义 4x9 键盘）
 *============================*/
static lv_obj_t *s_pair_mask = NULL;
static lv_obj_t *s_pair_ta = NULL;
static lv_obj_t *s_pair_hint = NULL;
static int s_pair_target_front = 1;

static void tire_pair_popup_close(void) {
    if (s_pair_mask) {
        lv_obj_del(s_pair_mask);
        s_pair_mask = NULL;
    }
    s_pair_ta = NULL;
    s_pair_hint = NULL;
}

static void tire_pair_mask_event_cb(lv_event_t *e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_event_get_target(e) != s_pair_mask) return;
    tire_pair_popup_close();
}

static void tire_pair_hint_show(const char *text) {
    if (!s_pair_hint || !lv_obj_is_valid(s_pair_hint)) return;
    lv_label_set_text(s_pair_hint, text ? text : "");
    lv_obj_invalidate(s_pair_hint);
}

static void tire_pair_append_char(char ch) {
    if (!s_pair_ta) return;
    if (strlen(lv_textarea_get_text(s_pair_ta)) >= 6) return;
    lv_textarea_set_cursor_pos(s_pair_ta, LV_TEXTAREA_CURSOR_LAST);
    lv_textarea_add_char(s_pair_ta, (uint32_t)ch);
    tire_pair_hint_show("");
}

static void tire_pair_key_event_cb(lv_event_t *e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (!s_pair_ta) return;

    lv_obj_t *btn = lv_event_get_target(e);
    const char *key = (const char *)lv_event_get_user_data(e);
    if (!key) key = (const char *)lv_obj_get_user_data(btn);
    if (!key) return;

    if (strcmp(key, "DEL") == 0) {
        lv_textarea_del_char(s_pair_ta);
        tire_pair_hint_show("");
        return;
    }
    if (strcmp(key, "CLOSE") == 0) {
        tire_pair_popup_close();
        return;
    }
    if (strcmp(key, "OK") == 0) {
        const char *txt = lv_textarea_get_text(s_pair_ta);
        if (!txt || strlen(txt) != 6) {
            tire_pair_hint_show("请输入6位配对码");
            return;
        }
        if (s_pair_target_front) {
            tire_pair_set_front_suffix(txt);
        } else {
            tire_pair_set_rear_suffix(txt);
        }
        tire_pair_popup_close();
        tire_ui_refresh_now();
        return;
    }

    tire_pair_append_char(key[0]);
}

static lv_obj_t *tire_pair_create_key(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
                                      lv_coord_t w, lv_coord_t h, const char *text,
                                      const char *key, lv_color_t bg_color) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_style_radius(btn, 18, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_bg_color(btn, bg_color, 0);
    lv_obj_set_style_bg_grad_color(btn, lv_color_hex(0x7388b8), 0);
    lv_obj_set_style_bg_grad_dir(btn, LV_GRAD_DIR_VER, 0);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_user_data(btn, (void *)key);
    lv_obj_add_event_cb(btn, tire_pair_key_event_cb, LV_EVENT_CLICKED, (void *)key);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(lbl, &lv_font_harmonyOS_42, 0);
    lv_obj_center(lbl);
    return btn;
}

static void tire_pair_popup_open(int is_front) {
    tire_pair_popup_close();
    s_pair_target_front = is_front ? 1 : 0;

    s_pair_mask = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_pair_mask, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(s_pair_mask, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(s_pair_mask, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(s_pair_mask, LV_OPA_30, 0);
    lv_obj_move_foreground(s_pair_mask);
    lv_obj_add_event_cb(s_pair_mask, tire_pair_mask_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *popup = lv_obj_create(s_pair_mask);
    lv_obj_set_size(popup, 1024, 560);
    lv_obj_center(popup);
    lv_obj_set_style_bg_color(popup, lv_color_hex(0x4c6295), 0);
    lv_obj_set_style_bg_grad_color(popup, lv_color_hex(0x324777), 0);
    lv_obj_set_style_bg_grad_dir(popup, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(popup, 255, 0);
    lv_obj_set_style_radius(popup, 24, 0);
    lv_obj_set_style_pad_all(popup, 24, 0);
    lv_obj_set_style_border_width(popup, 0, 0);
	lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(popup);
	lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), 0);
    lv_label_set_text(title, is_front ? "请输入前轮传感器的配对码" : "请输入后轮传感器的配对码");
    lv_obj_set_style_text_font(title, &lv_font_harmonyOS_42, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 32, 10);

    s_pair_ta = lv_textarea_create(popup);
    lv_obj_set_size(s_pair_ta, 860, 78);
    lv_obj_align(s_pair_ta, LV_ALIGN_TOP_MID, 0, 88);
    lv_textarea_set_one_line(s_pair_ta, true);
    lv_textarea_set_max_length(s_pair_ta, 6);
    lv_textarea_set_accepted_chars(s_pair_ta, "0123456789ABCDEF");
    lv_textarea_set_text(s_pair_ta, "");
    lv_textarea_set_cursor_pos(s_pair_ta, LV_TEXTAREA_CURSOR_LAST);
	lv_obj_set_style_bg_color(s_pair_ta, lv_color_hex(0x7d8eb8), 0);
    lv_obj_set_style_text_color(s_pair_ta, lv_color_hex(0xffffff), 0);
	lv_obj_set_style_text_font(s_pair_ta, &lv_font_harmonyOS_42, 0);
    lv_obj_set_style_radius(s_pair_ta, 14, 0);
    lv_obj_set_style_border_width(s_pair_ta, 0, 0);
    lv_obj_set_style_text_align(s_pair_ta, LV_TEXT_ALIGN_CENTER, 0);

    s_pair_hint = lv_label_create(popup);
    lv_label_set_text(s_pair_hint, "");
    lv_obj_set_width(s_pair_hint, 860);
    lv_obj_set_style_text_color(s_pair_hint, lv_color_hex(0xffd36b), 0);
    lv_obj_set_style_text_font(s_pair_hint, &lv_font_harmonyOS_30, 0);
    lv_obj_set_style_text_align(s_pair_hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(s_pair_hint, s_pair_ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lv_obj_t *kb_cont = lv_obj_create(popup);
    lv_obj_set_size(kb_cont, 860, 250);
    lv_obj_align(kb_cont, LV_ALIGN_BOTTOM_MID, 0, -26);
	lv_obj_clear_flag(kb_cont, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_pad_all(kb_cont, 0, 0);
	lv_obj_set_style_bg_opa(kb_cont, 0, 0);

    {
        const lv_coord_t key_w = 118;
        const lv_coord_t key_h = 68;
        const lv_coord_t gap = 8;
        const lv_color_t normal_bg = lv_color_hex(0x6d80b0);
        const lv_color_t action_bg = lv_color_hex(0x7888b3);
        const char *row1[] = {"1", "2", "3", "4", "5", "6"};
        const char *row2[] = {"7", "8", "9", "0", "A"};
        const char *row3[] = {"B", "C", "D", "E", "F"};
        int i = 0;

        for (i = 0; i < 6; ++i) {
            tire_pair_create_key(kb_cont, i * (key_w + gap), 0, key_w, key_h,
                                 row1[i], row1[i], normal_bg);
        }
        tire_pair_create_key(kb_cont, 6 * (key_w + gap), 0, 144, key_h,
                             "退格", "DEL", action_bg);

        for (i = 0; i < 5; ++i) {
            tire_pair_create_key(kb_cont, i * (key_w + gap), key_h + gap, key_w, key_h,
                                 row2[i], row2[i], normal_bg);
        }
        tire_pair_create_key(kb_cont, 5 * (key_w + gap), key_h + gap,
                             262, key_h, "确定", "OK", action_bg);

        for (i = 0; i < 5; ++i) {
            tire_pair_create_key(kb_cont, i * (key_w + gap), (key_h + gap) * 2, key_w, key_h,
                                 row3[i], row3[i], normal_bg);
        }
        tire_pair_create_key(kb_cont, 5 * (key_w + gap), (key_h + gap) * 2,
                             262, key_h, "关闭", "CLOSE", action_bg);
    }
}

static void screen_Tire_btn_fPair_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
        case LV_EVENT_CLICKED: {
            tire_pair_popup_open(1);
            break;
        }
        default:
            break;
    }
}

static void screen_Tire_btn_bPair_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
        case LV_EVENT_CLICKED: {
            tire_pair_popup_open(0);
            break;
        }
        default:
            break;
    }
}

void events_init_screen_Tire (lv_ui *ui)
{
	lv_obj_add_event_cb(ui->screen_Tire_btn_return, screen_Tire_btn_return_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_Tire_btn_lPressMax, screen_Tire_btn_lPressMax_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_Tire_btn_rPressMin, screen_Tire_btn_rPressMin_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_Tire_btn_lPressMin, screen_Tire_btn_lPressMin_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_Tire_btn_rPressMax, screen_Tire_btn_rPressMax_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_Tire_btn_lTemp, screen_Tire_btn_lTemp_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_Tire_btn_rTemp, screen_Tire_btn_rTemp_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_Tire_btn_pressBar, screen_Tire_btn_pressBar_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_Tire_btn_pressPsj, screen_Tire_btn_pressPsj_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_Tire_btn_tempC, screen_Tire_btn_tempC_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_Tire_btn_tempF, screen_Tire_btn_tempF_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_Tire_btn_paraReset, screen_Tire_btn_paraReset_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_Tire_btn_clearPair, screen_Tire_btn_clearPair_event_handler, LV_EVENT_ALL, ui);

	// 配对按钮事件
	lv_obj_add_event_cb(ui->screen_Tire_btn_fPair, screen_Tire_btn_fPair_event_handler, LV_EVENT_ALL, ui);
	lv_obj_add_event_cb(ui->screen_Tire_btn_bPair, screen_Tire_btn_bPair_event_handler, LV_EVENT_ALL, ui);

	// 进入胎压界面时立即刷新一次，避免显示旧值或 0 值
	tire_alarm_init_if_needed();
	tire_alarm_refresh_threshold_ui();
	tire_ui_refresh_now();
}