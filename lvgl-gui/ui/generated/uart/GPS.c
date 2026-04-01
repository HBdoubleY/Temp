
#include <stdio.h>
#include <stdbool.h>
#include "GPS.h"
#include "wchar.h"
#include "Rtc2C.h"



/* 验证NMEA语句的校验和 (规格书5.1.1节提到校验和) */
static int verify_nmea_checksum(const char *nmea_str) {
    int i = 1; /* 跳过开头的'$' */
    uint8_t checksum = 0;
    char checksum_str[3] = {0};
    int checksum_val = 0;
    
    if (nmea_str[0] != '$') {
        return 0; /* 无效的NMEA语句 */
    }
    
    /* 计算校验和 (从'$'后到'*'前的所有字符异或) */
    while (nmea_str[i] != '\0' && nmea_str[i] != '*') {
        checksum ^= nmea_str[i];
        i++;
    }
    
    if (nmea_str[i] != '*') {
        return 0; /* 没有找到校验和分隔符 */
    }
    
    /* 获取语句中的校验和值 (16进制) */
    if (nmea_str[i+1] != '\0' && nmea_str[i+2] != '\0') {
        checksum_str[0] = nmea_str[i+1];
        checksum_str[1] = nmea_str[i+2];
        sscanf(checksum_str, "%2x", &checksum_val);
    } else {
        return 0; /* 校验和字段不完整 */
    }
    
    return (checksum == checksum_val);
}

/* 解析GGA数据 (Global positioning system fixed data) - 规格书5.1.1节 */
static void parse_gga(const char *gga_str) {
    char nmea_copy[NMEA_MAX_LENGTH];
    char *saveptr = NULL;
    char *token;
    int field_count = 0;
    
    /* 复制字符串以免破坏原数据 */
    strncpy(nmea_copy, gga_str, NMEA_MAX_LENGTH-1);
    nmea_copy[NMEA_MAX_LENGTH-1] = '\0';
    
    DEBUG_PRINTF("\n=== GGA 定位数据 ===\n");
    
    /* 分割逗号分隔的字段 */
    token = strtok_r(nmea_copy, ",", &saveptr);
    
    while (token != NULL && field_count < 15) {
        field_count++;
        
        switch (field_count) {
            case 1: /* Message ID */
                DEBUG_PRINTF("消息类型: %s\n", token);
                break;
                
            case 2: /* UTC时间 (hhmmss.sss) */
                if (strlen(token) >= 6) {
                    char hour[3] = {0}, min[3] = {0}, sec[10] = {0};
                    strncpy(hour, token, 2);
                    strncpy(min, token+2, 2);
                    strncpy(sec, token+4, 6);
                    DEBUG_PRINTF("UTC时间: %s:%s:%s\n", hour, min, sec);
                }
                break;
                
            case 3: /* 纬度 (ddmm.mmmmm) */
                if (strlen(token) > 0 && token[0] != '\0') {
                    char deg[3] = {0};
                    char min[10] = {0};
                    strncpy(deg, token, 2);
                    strncpy(min, token+2, strlen(token)-2);
                    DEBUG_PRINTF("纬度: %s度 %s分", deg, min);
                }
                break;
                
            case 4: /* 北/南指示符 */
                if (token[0] == 'N') DEBUG_PRINTF(" 北纬\n");
                else if (token[0] == 'S') DEBUG_PRINTF(" 南纬\n");
                else DEBUG_PRINTF("\n");
                break;
                
            case 5: /* 经度 (dddmm.mmmmm) */
                if (strlen(token) > 0 && token[0] != '\0') {
                    char deg[4] = {0};
                    char min[10] = {0};
                    strncpy(deg, token, 3);
                    strncpy(min, token+3, strlen(token)-3);
                    DEBUG_PRINTF("经度: %s度 %s分", deg, min);
                }
                break;
                
            case 6: /* 东/西指示符 */
                if (token[0] == 'E') DEBUG_PRINTF(" 东经\n");
                else if (token[0] == 'W') DEBUG_PRINTF(" 西经\n");
                else DEBUG_PRINTF("\n");
                break;
                
            case 7: /* 定位状态 (规格书 Table 2) */
                {
                    int fix_quality = atoi(token);
                    if (fix_quality >= 0 && fix_quality <= 2) {
                        DEBUG_PRINTF("定位状态: %d (%s)\n", fix_quality, fix_status_desc[fix_quality]);
                    } else if (fix_quality == 4 || fix_quality == 5) {
                        DEBUG_PRINTF("定位状态: %d (RTK Fixed/Float)\n", fix_quality);
                    } else {
                        DEBUG_PRINTF("定位状态: %d\n", fix_quality);
                    }
                }
                break;
                
            case 8: /* 使用的卫星数量 */
                DEBUG_PRINTF("使用卫星数: %s\n", token);
                break;
                
            case 9: /* HDOP (水平精度因子) */
                DEBUG_PRINTF("HDOP: %s 米\n", token);
                break;
                
            case 10: /* 海拔高度 (天线海拔) */
                DEBUG_PRINTF("海拔高度: %s 米\n", token);
                break;
                
            case 12: /* 大地水准面高 */
                if (token[0] != '\0')
                    DEBUG_PRINTF("大地水准面高: %s 米\n", token);
                break;
                
            case 14: /* 差分龄期 */
                if (token[0] != '\0')
                    DEBUG_PRINTF("差分数据龄期: %s 秒\n", token);
                break;
                
            case 15: /* 差分基站ID */
                if (token[0] != '\0' && strstr(token, "*") == NULL)
                    DEBUG_PRINTF("差分基站ID: %s\n", token);
                break;
        }
        
        token = strtok_r(NULL, ",", &saveptr);
    }
    DEBUG_PRINTF("===================\n");
}

/* 解析RMC数据 (Recommended Minimum Specific GNSS Data) - 规格书5.1.4节 */
static void parse_rmc(const char *rmc_str) {
    char nmea_copy[NMEA_MAX_LENGTH];
    char *saveptr = NULL;
    char *token;
    int field_count = 0;
    TTime time = {0};
    strncpy(nmea_copy, rmc_str, NMEA_MAX_LENGTH-1);
    nmea_copy[NMEA_MAX_LENGTH-1] = '\0';
    
    DEBUG_PRINTF("\n=== RMC 推荐最小定位数据 ===\n");
    
    token = strtok_r(nmea_copy, ",", &saveptr);
    
    while (token != NULL && field_count < 12) {
        field_count++;
        
        switch (field_count) {
            case 1: /* Message ID */
                DEBUG_PRINTF("消息类型: %s\n", token);
                break;
                
            case 2: /* UTC时间 */
                if (strlen(token) >= 6) {
                    char hour[3] = {0}, min[3] = {0}, sec[3] = {0};
                    strncpy(hour, token, 2);
                    strncpy(min, token+2, 2);
                    strncpy(sec, token+4, 2);
                    time.nHour = atoi(hour)+8;
                    time.nMinute = atoi(min);
                    time.nSecond = atoi(sec);
                    DEBUG_PRINTF("UTC时间: %d:%d:%d\n", time.nHour, time.nMinute, time.nSecond);
                }
                break;
                
            case 3: /* 状态 A=有效, V=无效 */
                DEBUG_PRINTF("数据状态: %s\n", (token[0] == 'A') ? "有效" : "无效");
                break;
                
            case 4: /* 纬度 */
                if (strlen(token) > 0 && token[0] != '\0') {
                    DEBUG_PRINTF("纬度: %s", token);
                }
                break;
                
            case 5: /* 北/南指示符 */
                if (token[0] == 'N') DEBUG_PRINTF(" 北纬\n");
                else if (token[0] == 'S') DEBUG_PRINTF(" 南纬\n");
                else DEBUG_PRINTF("\n");
                break;
                
            case 6: /* 经度 */
                if (strlen(token) > 0 && token[0] != '\0') {
                    DEBUG_PRINTF("经度: %s", token);
                }
                break;
                
            case 7: /* 东/西指示符 */
                if (token[0] == 'E') DEBUG_PRINTF(" 东经\n");
                else if (token[0] == 'W') DEBUG_PRINTF(" 西经\n");
                else DEBUG_PRINTF("\n");
                break;
                
            case 8: /* 对地速度 (节) */
                if (token[0] != '\0') {
                    float speed_knots = atof(token);
                    float speed_kmh = speed_knots * 1.852;
                    DEBUG_PRINTF("对地速度: %.3f 节 (约 %.2f km/h)\n", speed_knots, speed_kmh);
                }
                break;
                
            case 9: /* 对地航向 (度) */
                if (token[0] != '\0')
                    DEBUG_PRINTF("对地航向: %s 度\n", token);
                break;
                
            case 10: /* UTC日期 (DDMMYY) */
                if (strlen(token) == 6) {
                    char day[3] = {0}, month[3] = {0}, year[5] = {0};
                    strncpy(day, token, 2);
                    strncpy(month, token+2, 2);
                    strncpy(year, token+4, 2);
                    time.nYear = atoi(year)+2000;
                    time.nMonth = atoi(month);
                    time.nDay = atoi(day);
                    DEBUG_PRINTF("UTC日期: %d年%d月%d日\n", time.nYear, time.nMonth, time.nDay);
                }
                break;
                
            case 12: /* 模式指示 */
                if (token[0] != '\0' && strstr(token, "*") == NULL) {
                    DEBUG_PRINTF("模式指示: %s\n", token);
                }
                break;
        }
        
        token = strtok_r(NULL, ",", &saveptr);
    }
    //避免频繁写rtc
    static unsigned int n = 0;
    if((n % 30) == 0) RtcSetTime2C(&time);
    n++;
    if(n == 901) n = 1;
    DEBUG_PRINTF("============================\n");
}

/* 解析GSA数据 (GNSS DOP and Active Satellites) - 规格书5.1.2节 */
static void parse_gsa(const char *gsa_str) {
    char nmea_copy[NMEA_MAX_LENGTH];
    char *saveptr = NULL;
    char *token;
    int field_count = 0;
    int satellite_count = 0;
    
    strncpy(nmea_copy, gsa_str, NMEA_MAX_LENGTH-1);
    nmea_copy[NMEA_MAX_LENGTH-1] = '\0';
    
    DEBUG_PRINTF("\n=== GSA 可用卫星与精度因子 ===\n");
    
    token = strtok_r(nmea_copy, ",", &saveptr);
    
    while (token != NULL && field_count < 18) {
        field_count++;
        
        switch (field_count) {
            case 1: /* Message ID */
                DEBUG_PRINTF("消息类型: %s", token);
                if (strstr(token, "GPGSA")) DEBUG_PRINTF(" (GPS)\n");
                else if (strstr(token, "BDGSA")) DEBUG_PRINTF(" (BeiDou)\n");
                else if (strstr(token, "GNGSA")) DEBUG_PRINTF(" (GNSS)\n");
                else DEBUG_PRINTF("\n");
                break;
                
            case 2: /* Mode 1 */
                if (token[0] == 'M') DEBUG_PRINTF("模式1: M (手动)\n");
                else if (token[0] == 'A') DEBUG_PRINTF("模式1: A (自动)\n");
                break;
                
            case 3: /* Mode 2 */
                {
                    int mode2 = atoi(token);
                    if (mode2 == 1) DEBUG_PRINTF("模式2: 1 (无定位)\n");
                    else if (mode2 == 2) DEBUG_PRINTF("模式2: 2 (2D定位)\n");
                    else if (mode2 == 3) DEBUG_PRINTF("模式2: 3 (3D定位)\n");
                    else DEBUG_PRINTF("模式2: %d\n", mode2);
                }
                break;
                
            // case 4: /* 卫星PRN号 (通道1) */
            //     if (token[0] != '\0') {
            //         DEBUG_PRINTF("使用卫星PRN: ");
            //     }
            //     /* 继续后续字段... */
            //     break;
                
            case 4: /* 卫星PRN号 (通道1-12) */
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
                if (token[0] != '\0' && strlen(token) > 0) {
                    satellite_count++;
                    DEBUG_PRINTF("%s ", token);
                    if (satellite_count == 12) DEBUG_PRINTF("\n");
                }
                break;
                
            case 16: /* PDOP (位置精度因子) */
                DEBUG_PRINTF("PDOP: %s\n", token);
                break;
                
            case 17: /* HDOP (水平精度因子) */
                DEBUG_PRINTF("HDOP: %s\n", token);
                break;
                
            case 18: /* VDOP (垂直精度因子) */
                if (token[0] != '\0') {
                    char *checksum_pos = strchr(token, '*');
                    if (checksum_pos) {
                        *checksum_pos = '\0';
                    }
                    DEBUG_PRINTF("VDOP: %s\n", token);
                }
                break;
        }
        
        token = strtok_r(NULL, ",", &saveptr);
    }
    DEBUG_PRINTF("使用卫星数: %d 颗\n", satellite_count);
    DEBUG_PRINTF("============================\n");
}

/* 解析GSV数据 (GNSS Satellites in View) - 规格书5.1.3节 */
static void parse_gsv(const char *gsv_str) {
    char nmea_copy[NMEA_MAX_LENGTH];
    char *saveptr = NULL;
    char *token;
    int field_count = 0;
    int total_msgs = 0, msg_num = 0, sats_in_view = 0;
    
    strncpy(nmea_copy, gsv_str, NMEA_MAX_LENGTH-1);
    nmea_copy[NMEA_MAX_LENGTH-1] = '\0';
    
    token = strtok_r(nmea_copy, ",", &saveptr);
    
    while (token != NULL && field_count < 20) {
        field_count++;
        
        switch (field_count) {
            case 1: /* Message ID */
                if (strstr(token, "GPGSV")) DEBUG_PRINTF("\n[GPS] ");
                else if (strstr(token, "BDGSV")) DEBUG_PRINTF("\n[BeiDou] ");
                else if (strstr(token, "GLGSV")) DEBUG_PRINTF("\n[GLONASS] ");
                else DEBUG_PRINTF("\n[GNSS] ");
                DEBUG_PRINTF("GSV 可见卫星信息\n");
                break;
                
            case 2: /* 总消息数 */
                total_msgs = atoi(token);
                DEBUG_PRINTF("总消息数: %d\n", total_msgs);
                break;
                
            case 3: /* 当前消息序号 */
                msg_num = atoi(token);
                DEBUG_PRINTF("当前消息: %d/%d\n", msg_num, total_msgs);
                break;
                
            case 4: /* 可见卫星总数 */
                sats_in_view = atoi(token);
                DEBUG_PRINTF("可见卫星总数: %d\n", sats_in_view);
                break;
                
            case 5: /* 卫星1 PRN号 */
            case 9: /* 卫星2 PRN号 */
            case 13: /* 卫星3 PRN号 */
            case 17: /* 卫星4 PRN号 */
                if (token[0] != '\0' && strlen(token) > 0) {
                    int sat_num = (field_count - 1) / 4;
                    DEBUG_PRINTF("卫星%d - PRN: %s, ", sat_num, token);
                }
                break;
                
            case 6: /* 卫星1 仰角 */
            case 10: /* 卫星2 仰角 */
            case 14: /* 卫星3 仰角 */
            case 18: /* 卫星4 仰角 */
                if (token[0] != '\0' && strlen(token) > 0) {
                    DEBUG_PRINTF("仰角: %s°, ", token);
                }
                break;
                
            case 7: /* 卫星1 方位角 */
            case 11: /* 卫星2 方位角 */
            case 15: /* 卫星3 方位角 */
            case 19: /* 卫星4 方位角 */
                if (token[0] != '\0' && strlen(token) > 0) {
                    DEBUG_PRINTF("方位角: %s°, ", token);
                }
                break;
                
            case 8: /* 卫星1 信噪比 */
            case 12: /* 卫星2 信噪比 */
            case 16: /* 卫星3 信噪比 */
            case 20: /* 卫星4 信噪比 */
                if (token[0] != '\0') {
                    char *checksum_pos = strchr(token, '*');
                    if (checksum_pos) {
                        *checksum_pos = '\0';
                    }
                    DEBUG_PRINTF("SNR: %s dB-Hz\n", token);
                }
                break;
        }
        
        token = strtok_r(NULL, ",", &saveptr);
    }
}

/* 解析其他NMEA语句 (通用解析) */
static void parse_general_nmea(const char *nmea_str) {
    char nmea_copy[NMEA_MAX_LENGTH];
    
    strncpy(nmea_copy, nmea_str, NMEA_MAX_LENGTH-1);
    nmea_copy[NMEA_MAX_LENGTH-1] = '\0';
    
    /* 移除可能的换行符 */
    char *newline = strchr(nmea_copy, '\n');
    if (newline) *newline = '\0';
    
    newline = strchr(nmea_copy, '\r');
    if (newline) *newline = '\0';
    
    DEBUG_PRINTF("收到NMEA: %s\n", nmea_copy);
}

/* 处理NMEA语句 (路由到相应的解析函数) */
static void process_nmea_sentence(const char *nmea_str) {
    /* 基本验证 */
    if (nmea_str == NULL || strlen(nmea_str) < 6) {
        return;
    }
    
    /* 验证校验和 */
    if (!verify_nmea_checksum(nmea_str)) {
        DEBUG_PRINTF("[警告] 校验和错误: %s\n", nmea_str);
        return;
    }
    
    /* 根据NMEA语句类型路由到相应的解析函数 */
    if (strstr(nmea_str, "GGA") != NULL) {
        // parse_gga(nmea_str);
    } else if (strstr(nmea_str, "RMC") != NULL) {
        parse_rmc(nmea_str);
    } else if (strstr(nmea_str, "GSA") != NULL) {
        parse_gsa(nmea_str);
    } else if (strstr(nmea_str, "GSV") != NULL) {
        parse_gsv(nmea_str);
    } else if (strstr(nmea_str, "ZDA") != NULL) {
        parse_general_nmea(nmea_str);
    } else {
        /* 其他NMEA语句 */
        parse_general_nmea(nmea_str);
    }
}

/* 处理串口数据流 */
static int process_serial_data(int fd) {
    DEBUG_PRINTF("%s:%d_____%d\n",__func__,__LINE__, fd);
    char buffer[BUFFER_SIZE];
    char nmea_buffer[NMEA_MAX_LENGTH];
    int nmea_index = 0;
    int total_bytes = 0;
    int sentence_count = 0;
    time_t last_output_time = 0;
    
    
    while (1) {
        if(fd == -1) break;
        int bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            total_bytes += bytes_read;
            
            /* 处理每个字节，寻找完整的NMEA语句 */
            for (int i = 0; i < bytes_read; i++) {
                char c = buffer[i];
                
                /* 开始新的NMEA语句 */
                if (c == '$') {
                    if (nmea_index > 0) {
                        nmea_buffer[nmea_index] = '\0';
                        process_nmea_sentence(nmea_buffer);
                        sentence_count++;
                    }
                    nmea_index = 0;
                    nmea_buffer[nmea_index++] = c;
                }
                /* 继续收集字符 */
                else if (nmea_index > 0) {
                    /* 遇到换行符表示语句结束 */
                    if (c == '\n' || c == '\r') {
                        if (nmea_index > 6) { /* 最小有效长度: $GPGGA */
                            nmea_buffer[nmea_index] = '\0';
                            process_nmea_sentence(nmea_buffer);
                            sentence_count++;
                        }
                        nmea_index = 0;
                    }
                    /* 缓冲区保护 */
                    else if (nmea_index < NMEA_MAX_LENGTH - 1) {
                        nmea_buffer[nmea_index++] = c;
                    } else {
                        /* 缓冲区溢出，重新开始 */
                        nmea_index = 0;
                    }
                }
            }
            
            /* 定期显示统计信息 */
            time_t now = time(NULL);
            if (now - last_output_time >= 10) { /* 每10秒显示一次统计 */
                DEBUG_PRINTF("\n[统计] 总接收: %d 字节, 解析语句: %d 条\n", 
                       total_bytes, sentence_count);
                last_output_time = now;
            }
        } else if (bytes_read < 0) {
            /* 读取错误 */
            perror("读取串口错误");
            sleep(1); /* 等待1s后重试 */
        } else {
            /* 无数据 */
            sleep(1); /* 等待1s避免CPU占用过高 */
        }
    }
    return 0;
}

static void GpsThread(void *arg){
    int fd = *((int *)arg);
    process_serial_data(fd);
}

void createGpsThread(void){
    gpsFd = OpenDev("/dev/ttyS3");
    if(gpsFd < 0) return;
    set_speed(gpsFd, 115200);
    set_Parity(gpsFd, 8, 1, 'S');
    int ret = pthread_create(&GpsPthreadId, NULL, GpsThread, &gpsFd);
    if(ret != 0){
        printf("fatal error! create createGpsThread Thread fail[%d]\n", ret);
        return ret;
    }else{
        printf("create createGpsThread Thread success\n");
    }
    return 0;
}

void destoryGpsThead(void){
    close(gpsFd);
    gpsFd = -1;
    pthread_join(GpsPthreadId, NULL);
}
/*
void gpstest(){
    gpsFd = OpenDev("/dev/ttyS3");
    if(gpsFd < 0) return;
    set_speed(gpsFd, 115200);
    set_Parity(gpsFd, 8, 1, 'S');  
    process_serial_data(gpsFd);  
}
*/