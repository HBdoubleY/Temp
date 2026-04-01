#include "events_init.h"
#include <stdio.h>
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

extern void tire_ui_refresh_now(void);
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
		//by jy:pressure min regulate down
		if(g_sys_Data.pressureMax <= 0)
			return;
		if(!g_sys_Data.pressureUnit){
			g_sys_Data.pressureMax -= 0.1;
			char str[20];
			sprintf(str,"%.1f",g_sys_Data.pressureMax);
			lv_label_set_text(guider_ui.screen_Tire_label_pressMax1,str);
			lv_obj_invalidate(guider_ui.screen_Tire_label_pressMax1); 			
		}else{
			g_sys_Data.pressureMax -= 1;
			char str[20];
			sprintf(str,"%.0f",g_sys_Data.pressureMax);
			lv_label_set_text(guider_ui.screen_Tire_label_pressMax1,str);
			lv_obj_invalidate(guider_ui.screen_Tire_label_pressMax1); 		
		}		
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
		//by jy:pressure min regulate up
		if(!g_sys_Data.pressureUnit){
			g_sys_Data.pressureMin += 0.1;
			char str[20];
			sprintf(str,"%.1f",g_sys_Data.pressureMin);
			lv_label_set_text(guider_ui.screen_Tire_label_pressMin1,str);
			lv_obj_invalidate(guider_ui.screen_Tire_label_pressMin1); 			
		}else{
			g_sys_Data.pressureMin += 1;
			char str[20];
			sprintf(str,"%.0f",g_sys_Data.pressureMin);
			lv_label_set_text(guider_ui.screen_Tire_label_pressMin1,str);
			lv_obj_invalidate(guider_ui.screen_Tire_label_pressMin1); 			
		}		
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
		//by jy:pressure max regulat down
		if(g_sys_Data.pressureMin<=0)
			return;
		if(!g_sys_Data.pressureUnit){
			g_sys_Data.pressureMin -= 0.1;
			char str[20];
			sprintf(str,"%.1f",g_sys_Data.pressureMin);
			lv_label_set_text(guider_ui.screen_Tire_label_pressMin1,str);
			lv_obj_invalidate(guider_ui.screen_Tire_label_pressMin1); 
		}else{
			g_sys_Data.pressureMin -= 1;
			char str[20];
			sprintf(str,"%.0f",g_sys_Data.pressureMin);
			lv_label_set_text(guider_ui.screen_Tire_label_pressMin1,str);
			lv_obj_invalidate(guider_ui.screen_Tire_label_pressMin1); 			
		}
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
		//by jy:pressure max regulate up
		if(!g_sys_Data.pressureUnit){
			g_sys_Data.pressureMax += 0.1;
			char str[20];
			sprintf(str,"%.1f",g_sys_Data.pressureMax);
			lv_label_set_text(guider_ui.screen_Tire_label_pressMax1,str);
			lv_obj_invalidate(guider_ui.screen_Tire_label_pressMax1); 			
		}else{
			g_sys_Data.pressureMax += 1;
			char str[20];
			sprintf(str,"%.0f",g_sys_Data.pressureMax);
			lv_label_set_text(guider_ui.screen_Tire_label_pressMax1,str);
			lv_obj_invalidate(guider_ui.screen_Tire_label_pressMax1); 			
		}

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
		//by jy:temp max regulate down
		if(g_sys_Data.tempMax == 0)
			return;
		g_sys_Data.tempMax -= 1;
		char str[20];
		sprintf(str,"%d",g_sys_Data.tempMax);
		lv_label_set_text(guider_ui.screen_Tire_label_tempMax1,str);
		lv_obj_invalidate(guider_ui.screen_Tire_label_tempMax1); 
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
		//by jy:temp max regulate up
		g_sys_Data.tempMax += 1;
		char str[20];
		sprintf(str,"%d",g_sys_Data.tempMax);
		lv_label_set_text(guider_ui.screen_Tire_label_tempMax1,str);
		lv_obj_invalidate(guider_ui.screen_Tire_label_tempMax1); 
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
		//by jy:change the pressure unit
		if(!g_sys_Data.pressureUnit)
			return;
		g_sys_Data.bPressure = PressureUnitConversion(g_sys_Data.bPressure);
		g_sys_Data.fPressure = PressureUnitConversion(g_sys_Data.fPressure);		
		g_sys_Data.pressureMax = PressureUnitConversion(g_sys_Data.pressureMax);
		g_sys_Data.pressureMin = PressureUnitConversion(g_sys_Data.pressureMin);
		g_sys_Data.pressureUnit =  false;
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
		sprintf(str,"%.1f",g_sys_Data.pressureMin);
		lv_label_set_text(guider_ui.screen_Tire_label_pressMin1,str);
		sprintf(str,"%.1f",g_sys_Data.pressureMax);
		lv_label_set_text(guider_ui.screen_Tire_label_pressMax1,str);
		lv_obj_invalidate(guider_ui.screen_Tire_label_pressMin1); 
		lv_obj_invalidate(guider_ui.screen_Tire_label_pressMax1); 		
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
		//by jy:change the pressure unit
		if(g_sys_Data.pressureUnit)
			return;
		g_sys_Data.bPressure = PressureUnitConversion(g_sys_Data.bPressure);
		g_sys_Data.fPressure = PressureUnitConversion(g_sys_Data.fPressure);
		g_sys_Data.pressureMax = PressureUnitConversion(g_sys_Data.pressureMax);
		g_sys_Data.pressureMin = PressureUnitConversion(g_sys_Data.pressureMin);
		g_sys_Data.pressureUnit =  true;
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
		sprintf(str,"%.0f",g_sys_Data.pressureMin);
		lv_label_set_text(guider_ui.screen_Tire_label_pressMin1,str);
		sprintf(str,"%.0f",g_sys_Data.pressureMax);
		lv_label_set_text(guider_ui.screen_Tire_label_pressMax1,str);
		lv_obj_invalidate(guider_ui.screen_Tire_label_pressMin1); 
		lv_obj_invalidate(guider_ui.screen_Tire_label_pressMax1); 						
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
		//by jy:change the temp unit
		if(!g_sys_Data.tempUnit)
			return;
		g_sys_Data.bTemp = TempUnitConversion(g_sys_Data.bTemp);
		g_sys_Data.fTemp = TempUnitConversion(g_sys_Data.fTemp);
		g_sys_Data.tempMax = TempUnitConversion(g_sys_Data.tempMax);
		g_sys_Data.tempUnit = false;
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
		sprintf(str,"%d",g_sys_Data.tempMax);
		lv_label_set_text(guider_ui.screen_Tire_label_tempMax1,str);			
		lv_obj_invalidate(guider_ui.screen_Tire_label_fTemp);
		lv_obj_invalidate(guider_ui.screen_Tire_label_bTemp);
		lv_obj_invalidate(guider_ui.screen_Tire_label_tempMax1);
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
		//by jy:change the temp unit
		if(g_sys_Data.tempUnit)
			return;
		g_sys_Data.bTemp = TempUnitConversion(g_sys_Data.bTemp);
		g_sys_Data.fTemp = TempUnitConversion(g_sys_Data.fTemp);	
		g_sys_Data.tempMax = TempUnitConversion(g_sys_Data.tempMax);		
		g_sys_Data.tempUnit = true;
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
		//by jy:reset the parameter 
		if(!g_sys_Data.pressureUnit){
			g_sys_Data.pressureMax = 4.0;
			g_sys_Data.pressureMin = 1.6;
		}else{
			g_sys_Data.pressureMax = 58;
			g_sys_Data.pressureMin = 23;
		}
		if(g_sys_Data.tempUnit){
			g_sys_Data.tempMax = 154;
		}else{
			g_sys_Data.tempMax = 68;
		}
		char str[20];
		sprintf(str,"%.1f",g_sys_Data.pressureMin);
		lv_label_set_text(guider_ui.screen_Tire_label_pressMin1,str);
		sprintf(str,"%d",g_sys_Data.tempMax);
		lv_label_set_text(guider_ui.screen_Tire_label_tempMax1,str);
		sprintf(str,"%.1f",g_sys_Data.pressureMax);
		lv_label_set_text(guider_ui.screen_Tire_label_pressMax1,str);
		lv_obj_invalidate(guider_ui.screen_Tire_label_pressMin1); 
		lv_obj_invalidate(guider_ui.screen_Tire_label_tempMax1); 
		lv_obj_invalidate(guider_ui.screen_Tire_label_pressMax1); 
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
static int s_pair_target_front = 1;

static void tire_pair_popup_close(void) {
    if (s_pair_mask) {
        lv_obj_del(s_pair_mask);
        s_pair_mask = NULL;
    }
    s_pair_ta = NULL;
}

static void tire_pair_mask_event_cb(lv_event_t *e) {
    // 仅当点击到 mask 自身背景才关闭（避免点击键盘按钮时误关）
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (lv_event_get_target(e) != s_pair_mask) return;
    tire_pair_popup_close();
}

static void tire_pair_key_event_cb(lv_event_t *e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (!s_pair_ta) return;

    lv_obj_t *btn = lv_event_get_target(e);
    uintptr_t idx = (uintptr_t)lv_obj_get_user_data(btn);

    // // idx==35: DEL（键盘最后一格 Z 用作退格）
    // if (idx == 35) {
    //     lv_textarea_del_char(s_pair_ta);
    //     return;
    // }

    // 正常数字/字母按键：按键仅会插入被 accepted_chars 允许的字符
    static const char kb_labels[36] = {
        '1','2','3','C','D','E','F','G','H',
        '4','5','6','I','J','K','L','M','N',
        '7','8','9','O','P','Q','R','S','T',
        '0','A','B','U','V','W','X','Y','Z'
    };

    char ch = kb_labels[idx % 36];
    // 光标强制到末尾，避免插入到中间导致长度校验失败
    lv_textarea_set_cursor_pos(s_pair_ta, LV_TEXTAREA_CURSOR_LAST);
    lv_textarea_add_char(s_pair_ta, (uint32_t)ch);

    // 自动确认：长度到 6 后直接保存配对
    const char *txt = lv_textarea_get_text(s_pair_ta);
    if (txt && strlen(txt) == 6) {
        if (s_pair_target_front) {
            tire_pair_set_front_suffix(txt);
        } else {
            tire_pair_set_rear_suffix(txt);
        }
        tire_pair_popup_close();
        tire_ui_refresh_now();
    }
}

static void tire_pair_popup_open(int is_front) {
    tire_pair_popup_close();
    s_pair_target_front = is_front ? 1 : 0;

    s_pair_mask = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_pair_mask, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(s_pair_mask, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(s_pair_mask, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(s_pair_mask, 120, 0);
    lv_obj_move_foreground(s_pair_mask);
    lv_obj_add_event_cb(s_pair_mask, tire_pair_mask_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *popup = lv_obj_create(s_pair_mask);
    lv_obj_set_size(popup, 1000, 630);
    lv_obj_center(popup);
    lv_obj_set_style_bg_color(popup, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(popup, 255, 0);
    lv_obj_set_style_radius(popup, 12, 0);
    lv_obj_set_style_pad_all(popup, 20, 0);
	lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(popup);
	lv_obj_set_style_text_color(title, lv_color_hex(0x000000), 0);
    lv_label_set_text(title, is_front ? "前轮匹配后三位MAC" : "后轮匹配后三位MAC");
    lv_obj_set_style_text_font(title, &lv_font_harmonyOS_42, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    s_pair_ta = lv_textarea_create(popup);
    lv_obj_set_size(s_pair_ta, 740, 100);
    lv_obj_align(s_pair_ta, LV_ALIGN_TOP_MID, 0, 80);
    lv_textarea_set_one_line(s_pair_ta, true);
    lv_textarea_set_max_length(s_pair_ta, 6);
    // lv_textarea_set_accepted_chars(s_pair_ta, "0123456789ABCDEF");
    lv_textarea_set_text(s_pair_ta, "");
    lv_textarea_set_cursor_pos(s_pair_ta, LV_TEXTAREA_CURSOR_LAST);
	lv_obj_set_style_bg_color(s_pair_ta, lv_color_hex(0xf2f2f2), 0);
    lv_obj_set_style_text_color(s_pair_ta, lv_color_hex(0x000000), 0);
	lv_obj_set_style_text_font(s_pair_ta, &lv_font_montserrat_30, 0);  // 24px大小
    lv_obj_set_style_radius(s_pair_ta, 8, 0);

    // 4行9列键盘区域
    lv_obj_t *kb_cont = lv_obj_create(popup);
    lv_obj_set_size(kb_cont, 810, 400);
    lv_obj_align(kb_cont, LV_ALIGN_BOTTOM_MID, 0, -10);
	lv_obj_clear_flag(kb_cont, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_pad_all(kb_cont, 0, 0);
	lv_obj_set_style_pad_gap(kb_cont, 0, 0);
	// lv_obj_set_style_layout(kb_cont, LV_LAYOUT_NONE, 0);  // 设置为无布局，使用绝对定位
	lv_obj_set_style_bg_opa(kb_cont, 0, 0);

    lv_coord_t kb_w = 810;
    lv_coord_t kb_h = 400;
    lv_coord_t cell_w = kb_w / 9;
    lv_coord_t cell_h = kb_h / 4;
    lv_coord_t gap_x = 2;
    lv_coord_t gap_y = 2;

    for (uintptr_t idx = 0; idx < 36; idx++) {
        lv_coord_t row = idx / 9;
        lv_coord_t col = idx % 9;

        lv_obj_t *btn = lv_btn_create(kb_cont);
        lv_obj_set_size(btn, cell_w - gap_x, cell_h - gap_y);
        lv_obj_set_pos(btn, col * cell_w, row * cell_h);

        lv_obj_set_style_radius(btn, 6, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xe6f3ff), 0);

        lv_obj_set_user_data(btn, (void *)(uintptr_t)idx);
        lv_obj_add_event_cb(btn, tire_pair_key_event_cb, LV_EVENT_ALL, NULL);

        lv_obj_t *lbl = lv_label_create(btn);

        static const char kb_labels[36] = {
            '1','2','3','C','D','E','F','G','H',
            '4','5','6','I','J','K','L','M','N',
            '7','8','9','O','P','Q','R','S','T',
            '0','A','B','U','V','W','X','Y','Z'
        };
        char t[2] = { kb_labels[idx], '\0' };
        lv_label_set_text(lbl, t);
		lv_obj_set_style_text_color(lbl, lv_color_hex(0x000000), 0);
		lv_obj_set_style_text_font(lbl, &lv_font_montserrat_30, 0);

        lv_obj_center(lbl);
        // lv_obj_set_style_text_font(lbl, &lv_font_harmonyOS_42, 0);
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
	tire_ui_refresh_now();
}