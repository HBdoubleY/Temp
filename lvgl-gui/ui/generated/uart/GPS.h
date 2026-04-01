#ifndef GPS_H_
#define GPS_H_

#include "uart.h"
#include "pthread.h"

#define DEBUG_PRINT_ENABLED 0

#if DEBUG_PRINT_ENABLED
    #define DEBUG_PRINTF(fmt, ...) printf("[DEBUG] " fmt, ##__VA_ARGS__)
#else
    #define DEBUG_PRINTF(fmt, ...) do {} while(0)
#endif

#define ID_GGA  "GGA"   //全球定位系统固定数据
//$GPGGA,012743.000,2320.287514,N,11214.799042,E,1,20,0.66,30.9,M,0.0,M,,*6F
#define ID_GSA  "GSA"   //GNSS 总体卫星数据
//$GPGSA,A,3,10,12,18,25,192,193,197,,,,,,1.26,0.66,1.07,1*2E $BDGSA,A,3,201,203,206,208,209,216,219,221,222,226,236,238,1.26,0.66,1.07,4*0B
#define ID_GSV  "GSV"   //GNSS 详细的卫星数据
//$GPGSV,3,1,11,10,55,338,45,12,35,087,44,15,00,000,31,18,28,191,36*70 $BDGSV,4,1,14,201,47,122,37,202,00,000,36,203,65,189,41,206,72,240,40*6C
#define ID_RMC  "RMC"   //推荐的 GNSS 最小数据
 //$GNRMC,012743.000,A,2320.287514,N,11214.799042,E,0.039,0.00,141125,,,A*43
#define ID_ZDA  "ZDA"   //Date and time
//$GNZDA,012743.000,14,11,2025,00,00*4B

#define NMEA_MAX_LENGTH 128
#define BUFFER_SIZE 128

typedef long time_t;
typedef unsigned char   uint8_t;

static pthread_t GpsPthreadId;
static int gpsFd = -1;
/* 定位状态定义 (根据规格书 Table 2 & Table 5) */
const char* fix_status_desc[] = {
    "Fix not available",  // 0
    "GNSS fix",           // 1 (2D/3D)
    "Differential GNSS fix" // 2
};

/* 模式定义 (根据规格书 Table 4) */
const char* mode1_desc[] = {
    "Manual",   // M
    "Automatic" // A
};

/* 系统ID定义 (根据规格书 5.1.2 GSA部分) */
const char* system_id_desc[] = {
    "",
    "GPS",  // 1
    "",
    "",
    "BeiDou" // 4
};


static int verify_nmea_checksum(const char *nmea_str);
static void parse_gga(const char *gga_str);
static void parse_rmc(const char *rmc_str);
static void parse_gsa(const char *gsa_str);
static void parse_gsv(const char *gsv_str);
static void parse_general_nmea(const char *nmea_str);
static void process_nmea_sentence(const char *nmea_str);
static int process_serial_data(int fd);
void createGpsThread(void);
void destoryGpsThead(void);
void gpstest();
#endif