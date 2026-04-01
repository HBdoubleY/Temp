#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#ifndef LIBZLINK_H
#define LIBZLINK_H

typedef void* LIBZLINK_HANDLE;

enum ENABLE_BT_TYPE {
	ENABLE_TYPE_NONE		=		0b0000,	// 0x00
	ENABLE_TYPE_CARPLAY		=		0b0001,	//0x01
	ENABLE_TYPE_AA			=		0b0010,	//0x02
	ENABLE_TYPE_ANDROID_CARLIFE	=	0b0100	,//0x04
	ENABLE_TYPE_ANDROID_HICAR	=	0b1000, //0x08
	ENABLE_TYPE_ANDROID_CARLINK	=	0b10000, //0x10

	ENABLE_TYPE_CP_AA			=	0b0011,//(ENABLE_TYPE_CARPLAY | ENABLE_TYPE_AA),	0x03
	ENABLE_TYPE_CP_ANDROIDCARLIFE=	0b0101,//(ENABLE_TYPE_CARPLAY | ENABLE_TYPE_ANDROID_CARLIFE),	//0x05
	ENABLE_TYPE_CP_HICAR		=	0b1001,//(ENABLE_TYPE_CARPLAY | ENABLE_TYPE_ANDROID_HICAR),	//0x09

	ENABLE_TYPE_RECONN  		=	0x0020,	//32
	ENABLE_TYPE_CLEAN_LIST  	=	0x0040, // 64
};

//usb mode-Apple
enum APPLE_WRIED_LINK_MODE {
    APPLE_WIRED_LINK_NONE = (1 << 0),	// 1
    CARPLAY_WIRED_MODE = (1 << 1),		// 2
    QUICK_TIME_MODE = (1 << 2),			// 4
};

//wireless mode-Apple
enum APPLE_WIRELESS_LINK_MODE {
    APPLE_WIRELESS_NONE = (1 << 3),		// 8
    CARPLAY_WIRELESS_MODE = (1 << 4),	// 16
    AIRPLAY_WIRELESS_MODE = (1 << 5),	//32
    iOS_IP_LINK_MODE = (1 << 16),		//65536,
};

//usb mode-Android
enum ANDROID_WRIED_LINK_MODE {
    ANDROID_WIRED_LINK_NONE = (1 << 6),	// 64
    AA_WIRED_MODE = (1 << 7),			//128
    AOA_WIRED_MODE = (1 << 8),			//256
    CARLIFE_WIRED_MODE = (1 << 9),		//512,
    HICAR_WIRED_MODE = (1 << 10),		//1024,
};

//wireless mode-Android
enum ANDROID_WIRELESS_LINK_MODE {
    ANDROID_WIRELESS_NONE = (1 << 11),		//2048,
    AA_WIRELESS_MODE = (1 << 12),		//4096,
    IP_WIRELESS_MODE = (1 << 13),		//8192,
    HICAR_WIRELESS_MODE = (1 << 14),		//16384,
    CARLIFE_WIRELESS_MODE = (1 << 15),		//32768,
	CARLINK_WIRELESS_MODE = (1 << 16),
	MIRACAST_WIRELESS_MODE = (1 << 17),
};

enum LIBZLINK_SESSION_STATE{
	SESSION_WAIT_INIT = 0,
	SESSION_WAITING_AUTH = 1,
	SESSION_WAITING_DEVICE = 2,
	SESSION_STARTING = 3,
	SESSION_STARTED = 4,
	SESSION_FAILED = 5,
	SESSION_END = 6,
	AIRPLAY_SESSION_STARTED=7,
	AIRPLAY_SESSION_END =8,
	AIRPLAY_SESSION_PLAY=9,
	AIRPLAY_SESSION_PAUSE=10,
	AIRPLAY_SESSION_PHONE_END=11,
	AIRPLAY_SESSION_STARTING=12,
};
enum AIRPLAY_STATE{
	AIRPLAY_STATE_STOP = 0,
	AIRPLAY_STATE_START = 1,
	AIRPLAY_STATE_PLAY = 2,
	AIRPLAY_STATE_PAUSE = 3,
};
enum PHONE_TYPE{
	NO_DEVICE_LINK = 0,
	CARPLAY_WIRED = 1,
	QUICK_TIME = 2,
	AA_WIRED = 3,
	AOA_WIRED = 4,
	CARPLAY_WIRELESS = 5,
	AIRPLAY_WIRELESS = 6,
	AA_WIRELESS = 7,
	IP_WIRELESS = 8,	
	CARLIFE_WIRED = 9,
    	ZJ_AUTH_WIRED = 10,
   	HICAR_WIRED = 11,
	CARLIFE_WIRELESS = 12,
	ZJ_AUTH_WIRELESS = 13,
	CARLINK_WIRELESS = 14,
	HICAR_WIRELESS = 15,
	ZJ_AUTH_ONLINE = 16,
};

enum ZLINK_MEDIA_TYPE{
	AUDIO_TYPE_NONE = 0,
	AUDIO_TYPE_MEDIA = 1,
	AUDIO_TYPE_SPEECH = 2,
	AUDIO_TYPE_ALERT = 3,
	AUDIO_TYPE_TELEPHONY = 4,
	AUDIO_TYPE_OTHRE = 5,
};

enum ERROR_CODE{
	NO_ERROR = 0,
	MFI_ERROR = 1,
	iAP_ERROR = 2,
	WIFI_ERROR = 3,
};

enum ACTIVATE_STATE{
    ACTIVATION_SUCCESS = 0,
    ACTIVATION_FAILED,
    ACTIVATION_READ_FAILED,
    ACTIVATION_WRITE_FAILED,
    ACTIVATION_INFO_INVALID,
    ACTIVATION_ABNORMAL,
    ACTIVATION_STARTING,
};

enum CP_SPEECH_MODE{
	CP_SPEECH_NONE = -1,
	CP_SPEECH_NO_APP = 0,	// No speech-related states are active.
	CP_SPEECH_SPEAKING = 1,	// phone speech is speaking to the user.
	CP_SPEECH_RECOGNIZING = 2,  // phone speech is recording audio to recognize speech from the user.
};

enum KEYCODE_TYPE {
    KEY_MEDIA_PLAY = 1,
    KEY_MEDIA_PAUSE = 2,
    KEY_MEDIA_PLAY_PAUSE = 3,
    KEY_MEDIA_NEXT = 4,
    KEY_MEDIA_PREVIOUS = 5,
    KEY_SOFT_LEFT = 6,
    KEY_DPAD_LEFT = 7,
    KEY_SOFT_RIGHT = 8,
    KEY_DPAD_RIGHT = 9,
    KEY_DPAD_UP = 10,
    KEY_DPAD_DOWN = 11,
    KEY_ENTER = 12,
    KEY_DPAD_CENTER = 13,
    KEY_HOME = 14,
    KEY_BACK = 15,
    KEY_ENDCALL = 16,
    KEY_CALL = 17,
    KEY_ASR = 18,
    KEY_LEFT_TRUN = 19,
    KEY_RIGNT_TRUN = 20,
//    KEY_CLOCKWISE_ROTATE = 21,			//--- deprecated and replaced by libzlink_android_mirror_rotation()
//    KEY_ANTI_CLOCKWISE_ROTATE = 22,        //--- deprecated and replaced by libzlink_android_mirror_rotation()
	KEY_CALL_AND_ENDCALL = 23,
	KEY_WIN_ENTER = 24, //
	KEY_WIN_UP = 25,
	KEY_WIN_DOWN = 26,
	KEY_WIN_LEFT =27,
	KEY_WIN_RIGHT =28,
	KEY_ALT_F4 = 29,//:
	KEY_WIN_DELETE = 30,
	KEY_CALL_MUTE = 31,
};

enum CP_KEYCODE_REQUESTUI_TYPE {
	CP_KEY_REQUESTUI_CURRENT = 2000,
	CP_KEY_REQUESTUI_MAP = 2001,
	CP_KEY_REQUESTUI_MUSIC = 2002,
	CP_KEY_REQUESTUI_NOWPLAYING = 2003,
	CP_KEY_REQUESTUI_PHONECALL = 2004,
};

enum AA_KEYCODE_REQUESTUI_TYPE {
	AA_KEY_REQUESTUI_MAP = 2005,
	AA_KEY_REQUESTUI_MUSIC = 2006,
	AA_KEY_REQUESTUI_PHONECALL = 2007,
};

enum ROTATION_ANGLE {
	CLOCKWISR_0,
	CLOCKWISR_90,
	CLOCKWISR_180,
	CLOCKWISR_270,
};

//only for Android AOA Mirror and IP wireless Mirror
enum MIRROR_ANDROID_VIDEO_SETTING {
    MIRROR_ANDROID_STANDARD = 0, //960*640
    MIRROR_ANDROID_HD = 1, 		//1280*720
};

//only for Android AOA Mirror and IP wireless Mirror
enum MIRROR_ANDROID_LUNCH {
    MIRROR_ANDROID_LUNCH_CARLIFE = 1,
	MIRROR_ANDROID_LUNCH_QQ_MUSIC = 2,
	MIRROR_ANDROID_LUNCH_WECHAT = 3,
};

//only for Android AOA Mirror and IP wireless Mirror
enum MIRROR_ANDROID_ROTATION {
    MIRROR_ANDROID_ROTATE_LEFT = 0, 		// screen turn left
    MIRROR_ANDROID_ROTATE_RIGHT = 1, 		// screen turn right
};

struct VIDEO_SCREEN_INFO {
  int width;
  int height;
  int available_width;
  int available_height;
  int available_pixel_x;
  int available_pixel_y;
  enum ROTATION_ANGLE rotation_angle;
  uint64_t inDisplayTicks;
};

enum MIRROR_PHONE_APP_STATE{
	APP_FORE_GROUND = 0,
	APP_BACK_GROUND = 1,
	PHONE_SCREEN_LOCK = 2,
};

enum MIRROR_STATE{
	MIRROR_STOPED = 0,
	MIRROR_RUNNING = 1,
};

enum HICAR_CALL_STATE{
	HICAR_CALL_STATE_IDLE = 0,
	HICAR_CALL_STATE_RINGING = 1,
	HICAR_CALL_STATE_OFFHOOK = 2,
};

enum CARLIFE_CALL_STATE{
	TELEPHONE_IDEL=0,
	TELEPHONE_IN=1,
	TELEPHONE_OUT=2,
};

struct SESSION_DATA {
	int width;				//width request to phone, it can be send after HU got phone link
	int height;				//height request to phone, it can be send after HU got phone link
	int fps;					//frame rate request to phone, it can be send after HU got phone link
	int density;
	int width_margin;
	int height_margin;
	char is_right_hand;			//if HU use right hand mode, must set, only work on carplay and Android Auto
	char is_night_mode;			//if use the dark mode, must set, only word on carplay and Android Auto and IP link
	enum APPLE_WRIED_LINK_MODE apple_wired;			//wired link for iPhone, must set
	enum APPLE_WIRELESS_LINK_MODE apple_wireless;	//wireless link for iPhone, must set 
	enum ANDROID_WRIED_LINK_MODE android_wired;		//wired link for Android Phone, must set
	enum ANDROID_WIRELESS_LINK_MODE android_wireless;	//wireless link for Android Phone, must set
	char *cp_icon_path;			//carplay back_to_HU icon, optional, only for carplay
	char *cp_icon_label;			//carplay back_to_HU label, optional, only for carplay
	char *cp_show_name;			//carplay name of show, optional, only for carplay
	char is_use_phone_audio;	//disable the carplay audio output of HU, use the phone audio
	char *platform_id;			//cpu platform id, must set
	char *vendor_name;			//vendor name of your company, must set
	char *hu_bt_addr;			//optional
	char is_force_usb_host;		//for carplay force switch otg host
	char *aoa_app_url;
	int mfi_bus_num;		//mfi i2c bus number, '-1' for auto 
	int otg_bus_num;		//usb otg bus number, '-1' for auto 
	char *otg_switch_host;	//cmd of usb otg switch to host
	char *otg_switch_device;  //cmd of usb otg switch to device
	char *print_save_path;  //save log of path(fliename-1~9)
	int control_airplay_start;
	int is_hud_info_on;
	char *auth_online_account;
	char *auth_online_passwd;
	char is_aa_developer_options;
	int is_aa_bypath;
};

struct MOTORCYCLE_DATA{
	int speed;
	int rotational_speed;
	int oil;
	int water_temperature;
	int average_speed;
	int average_oil;
	int tyre_pressure;
	int remaining_mileage;
	int ODO;				//total_mileage
	int TRIP;				//Current mileage
	int battery_voltage;
	int light_signal;
	int error_code;
};

struct P2P_INFO_DATA
{
   char *p2p_name;
   char *p2p_mac_address;
   char *netcard_name;
};

struct BY_LINK_INFO_DATA
{
  int is_connect;
  int is_pair;
  char local_mac_address[32];
  char local_bt_name[32];
  char phone_bt_mac_addr[32];
  char phone_bt_name[32];
};

typedef int ( *zlink_ready_f )(int is_zlink_ready, void *user_data);
typedef int ( *session_state_f )(enum LIBZLINK_SESSION_STATE session_state, enum PHONE_TYPE phone_type, void *user_data);
typedef int  ( *mfi_state_f)(int mfi_state, void *user_data);
typedef void ( *log_f )(enum ERROR_CODE error_num, char *log, void *user_data);
typedef int ( *phone_time_f )(uint64_t now_time_second, int time_zone_minute, void *user_data);
typedef int ( *ota_data_f )(char *file_name, char *file_md5, int file_total_lenght, 
							char is_last_packet, char *ota_data, int data_len, void *user_data);
typedef int ( *phone_gps_info_f )(double longitude, double latitude, double altitude, double direction, double speed, double time_stamp, void *user_data);
typedef int ( *phone_mold_data_f )(char *mold_data, void *user_data);
typedef void ( *pincode_data_f )(char *pincode_data, void *user_data);
typedef int ( *QRcode_data_f )(char *QRcode_data, void *user_data);
typedef int ( *video_data_f )(char *data, int len, struct VIDEO_SCREEN_INFO *screen_info, void *user_data);
typedef int ( *video_data2_f )(char *data, int len, struct VIDEO_SCREEN_INFO *screen_info, uint64_t inDisplayTicks, void *user_data);	//add inDisplayTicks
typedef int ( *vs_video_data_f )(char *data, int len, struct VIDEO_SCREEN_INFO *screen_info, uint64_t inDisplayTicks, void *user_data);
typedef int ( *main_audio_start_f )(enum ZLINK_MEDIA_TYPE media_type, void *user_data);
typedef int ( *main_audio_stop_f )(enum ZLINK_MEDIA_TYPE media_type, void *user_data);
typedef int ( *alt_audio_start_f )(int is_navi_audio, void *user_data);
typedef int ( *alt_audio_stop_f )(void *user_data);
typedef int ( *main_audio_data_f )(char *data, int len, enum ZLINK_MEDIA_TYPE media_type, int sample, int channels, int bits, void *user_data);
typedef int ( *alt_audio_data_f )(char *data, int len, int sample, int channels, int bits, void *user_data);
typedef int ( *mic_start_f )(enum ZLINK_MEDIA_TYPE media_type, int sample, int channels, int bits, void *user_data);
typedef int ( *mic_stop_f )(enum ZLINK_MEDIA_TYPE media_type, void *user_data);
typedef int ( *video_focus_request_f )(int is_hu_focus_on, void *user_data);
typedef int ( *audio_focus_request_f )(int is_hu_focus_on, void *user_data);
typedef int ( *audio_focus_request_with_type_f )(int is_hu_focus_on, int audio_focus_type, void *user_data);
typedef int ( *request_disableBT_f )(void *user_data);

typedef int ( *bt_hfp_state_f )(int is_bt_hfp_connect, void *user_data);


typedef int ( *request_p2p_start_f )(void *user_data);
typedef int ( *request_wifi_info_f )(void *user_data);
typedef int ( *cp_mode_change_f )(int is_cp_video_on, int is_cp_audio_on, int is_cp_speech_on, enum CP_SPEECH_MODE speech_mode, int is_cp_phonecall_on, int is_cp_turnbyturn_on, void *user_data);
typedef int ( *AA_navi_focus_request_f )(int is_hu_navi_start, void *user_data);
typedef int ( *activate_state_f)(enum ACTIVATE_STATE activate_state, void *user_data);
typedef int ( *mfi_uuid_f )(char *mfi_uuid, void *user_data);
typedef int ( *zlink_version_f )(char *zlink_ver, void *user_data);
typedef int ( *ios_version_f )(int ios_version, void *user_data);
typedef int ( *lock_screen_state_f )(int lock_screen_state, void *user_data);
typedef int ( *vs_play_state_f)(int vs_state, void *user_data);
typedef int ( *video_screen_forground_f)(int state, void *user_data);
typedef int ( *dialog_raquest_f)(int error_tips_code,char *error_tips_msg,void *user_data);
typedef int ( *vs_play_pause_f)(int state,void *user_data);
typedef int ( *mirror_phone_app_state_f)(enum MIRROR_PHONE_APP_STATE app_state, void *user_data);
typedef int ( *mirror_state_f)(enum MIRROR_STATE mirror_state, void *user_data);
typedef int ( *hicar_call_mode_state_f)(enum HICAR_CALL_STATE call_state, void *user_data);
typedef int ( *bt_link_info_f)(struct BY_LINK_INFO_DATA *bt_link_info, void *user_data);
typedef int ( *dlna_url_f )(int type,char *url, char* title,char* artist,char* album,char* albumurl, void *user_data);
typedef int ( *carlife_call_state_f)(enum CARLIFE_CALL_STATE call_state, void *user_data);
//call back
void libzlink_ready_cb_init(zlink_ready_f fun);
void libzlink_session_state_cb_init(session_state_f fun);
void libzlink_log_cb_init(log_f fun);
void libzlink_mfi_state_cb_init(mfi_state_f fun);
void libzlink_phone_time_init(phone_time_f fun);
void libzlink_ota_data_init(ota_data_f fun);
void libzlink_video_data_cb_init(video_data_f fun);
void libzlink_video_data_cb_init2(video_data2_f fun);		// add inDisplayTicks
void libzlink_vs_video_data_cb_init(vs_video_data_f fun); // add for vs video 
void libzlink_main_audio_start_cb_init(main_audio_start_f fun);
void libzlink_main_data_cb_init(main_audio_data_f fun);
void libzlink_main_audio_stop_cb_init(main_audio_stop_f fun);
void libzlink_alt_audio_start_cb_init(alt_audio_start_f fun);
void libzlink_alt_data_cb_init(alt_audio_data_f fun);
void libzlink_alt_audio_stop_cb_init(alt_audio_stop_f fun);
void libzlink_mic_start_cb_init(mic_start_f fun);
void libzlink_mic_stop_cb_init(mic_stop_f fun);
void libzlink_video_focus_request_cb_init(video_focus_request_f fun);
void libzlink_audio_focus_request_cb_init(audio_focus_request_f fun);
void libzlink_audio_focus_request_with_type_cb_init(audio_focus_request_with_type_f fun);
void libzlink_request_disableBT_cb_init(request_disableBT_f fun);
void libzlink_request_p2p_start_cb_init(request_p2p_start_f fun);
void libzlink_request_wifi_info_cb_init(request_wifi_info_f fun);
void libzlink_AA_navi_focus_request_cb_init(AA_navi_focus_request_f fun);
void libzlink_cp_mode_change_cb_init(cp_mode_change_f fun);
void libzlink_activate_state_cb_init(activate_state_f fun);
void libzlink_mirror_phone_app_state_cb_init(mirror_phone_app_state_f fun);
void libzlink_mirror_state_cb_init(mirror_state_f fun);
void libzlink_phone_gps_info_init(phone_gps_info_f fun);
void libzlink_bt_hfp_state_cb_init(bt_hfp_state_f fun);
void libzlink_pincode_data_cb_init(pincode_data_f fun);
void libzlink_QRcode_data_cb_init(QRcode_data_f fun);
void libzlink_hicar_call_mode_cb_init(hicar_call_mode_state_f fun);
void libzlink_bt_link_info_cb_init(bt_link_info_f fun);
void libzlink_ios_version_init(ios_version_f fun);
void libzlink_vs_play_state_init(vs_play_state_f fun);
void libzlink_video_screen_forground_init(vs_play_state_f fun);
void libzlink_screen_state_init(lock_screen_state_f fun);
void libzlink_CARLIFE_call_status_cb_init(carlife_call_state_f fun);
void libzlink_dialog_raquest_init(dialog_raquest_f fun);
void libzlink_vs_play_pause_init(vs_play_pause_f fun);

//DLNA callback
void libzlink_dlna_url_init(dlna_url_f fun);

//main functions
LIBZLINK_HANDLE libzlink_init(void *user_data);
void libzlink_deinit(LIBZLINK_HANDLE handle);
int libzlink_check_ready(LIBZLINK_HANDLE handle);
int libzlink_request_state();
int libzlink_get_activated_function(int *activate_type);
int libzlink_init_session_2(struct SESSION_DATA *session_data); //replace the libzlink_init_session()
int libzlink_init_session(int width, int height, int fps, char is_right_hand, char is_night_mode,
				enum APPLE_WRIED_LINK_MODE apple_wired, enum APPLE_WIRELESS_LINK_MODE apple_wireless,
				enum ANDROID_WRIED_LINK_MODE android_wired, enum ANDROID_WIRELESS_LINK_MODE android_wireless,
				char *cp_icon_path, char *platform_id, char *vendor_name, char *hu_bt_addr);	 //deprecated and replaced by libzlink_init_session_2()
int libzlink_activate();
int libzlink_restart_session();
int libzlink_touch_event(int x, int y, int is_touch_down);
int libzlink_multi_touch_event(const int *x, const int *y, int is_touch_down, int n_pointer_data,int action_index,int action);
int libzlink_key_event(int key, int is_key_down);
int libzlink_mic_data(char *data, int len);
int libzlink_mic_data2(char *data, int len, int sample, int channels, int bits);
int libzlink_wifi_info(char *wifi_name, char *wifi_passwd, char *ip_addr, int wifi_channel);
int libzlink_wifi_info2(char *wifi_name, char *wifi_passwd, char *ip_addr, int wifi_channel, char *netcard_name);
int libzlink_p2p_info_send(char *p2p_name, char *p2p_mac_address);
int libzlink_p2p_info2_send(struct P2P_INFO_DATA *p2p_info_data);
int libzlink_station_info_send(char *phone_ip);
int libzlink_video_focus(int is_hu_video_focus_on);
int libzlink_main_audio_focus(char is_hu_audio_focus_on);		//long audio focus
int libzlink_main_audio_short_focus(char is_hu_audio_focus_on);	//short audio focus
int libzlink_audio_focus_notification(int unsolicited, int focus_state );
int libzlink_AA_navi_focus_set(char is_hu_navi_focus_on);		//only for Android Auto
int libzlink_night_mode_set(int is_night_mode_on);
int libzlink_control_bt_request(int is_bt_enable, int enable_type);
int libzlink_control_hicar_carlink_setup(int is_start);
int libzlink_carplay_convert_to_airplay(enum AIRPLAY_STATE arplay_state);                    //for carplay convent to airplay
int libzlink_remove_hicar_list();

int libzlink_control_cp_audio_source(int is_use_phone_audio);

//DLNA API
int libzlink_dlna_playback_status(long duration, long postion, int is_playing, int is_paused);

//int libzlink_enter_app_event(enum MIRROR_ANDROID_LUNCH app);	//deprecated and replaced by libzlink_android_mirror_app_lunch()
int libzlink_android_mirror_app_lunch(enum MIRROR_ANDROID_LUNCH app); //only for Android AOA Mirror and IP wireless Mirror

//int libzlink_hd_state_event(enum MIRROR_ANDROID_VIDEO_SETTING setting); 	//deprecated and replaced by libzlink_android_mirror_viedo_setting()
int libzlink_android_mirror_viedo_setting(enum MIRROR_ANDROID_VIDEO_SETTING setting); //only for Android AOA Mirror and IP wireless Mirror

//only for Android AOA Mirror and IP wireless Mirror
int libzlink_android_mirror_rotation(enum MIRROR_ANDROID_ROTATION is_rotate_right); //only for Android AOA Mirror and IP wireless Mirror

//only for motor cycle
int libzlink_motorcycle_info(struct MOTORCYCLE_DATA *motorcycle_data);

int libzlink_bt_disconnect_request(int is_disconnect_bt);
int libzlink_close_bt_active_connect(int is_close_bt_active_connect);
#endif

