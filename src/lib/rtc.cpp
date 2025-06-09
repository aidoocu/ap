/** 
 * 
 * 
 * 
 */

#include "rtc.h"
#include "ap_config.h"
#include <EEPROM.h>

uRTCLib rtc(RTC_ADDR);

void get_time(char * time, int tz_offset_minutes) {

    rtc.refresh();
    
	// Obtener hora UTC del RTC
    int year = 2000 + rtc.year();
    int month = rtc.month();
    int day = rtc.day();
    int hour = rtc.hour();
    int minute = rtc.minute();
    int second = rtc.second();
    
	// Aplicar offset si es necesario
    if (tz_offset_minutes != 0) {
        // Convertir a timestamp
        struct tm t;
        t.tm_year = year - 1900;
        t.tm_mon = month - 1;
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min = minute + tz_offset_minutes;
        t.tm_sec = second;
        t.tm_isdst = 0;
        time_t ts = mktime(&t);
        // Convertir de nuevo a struct tm
        struct tm *lt = gmtime(&ts);
        year = lt->tm_year + 1900;
        month = lt->tm_mon + 1;
        day = lt->tm_mday;
        hour = lt->tm_hour;
        minute = lt->tm_min;
        second = lt->tm_sec;
        // Formato con offset
        int offset_h = tz_offset_minutes / 60;
        int offset_m = abs(tz_offset_minutes % 60);
        char sign = (tz_offset_minutes >= 0) ? '+' : '-';
        sprintf(time, "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d", year, month, day, hour, minute, second, sign, abs(offset_h), offset_m);
    } else {
        // Formato UTC
        sprintf(time, "%04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour, minute, second);
    }
}

bool set_date_time(char * date_time) {
    // Acepta formato ISO 8601 UTC (Z) o con offset (±hh:mm)
    if(!validate_date_time(date_time)) {
        Serial.println("Invalid date_time");
        return false;
    }
    int year, month, day, hour, minute, second, offset_h = 0, offset_m = 0;
    char tz[7] = "Z";
    // Detectar si es con offset o Z
    if (date_time[19] == 'Z') {
        // UTC puro
        sscanf(date_time, "%4d-%2d-%2dT%2d:%2d:%2dZ", &year, &month, &day, &hour, &minute, &second);
    } else {
        // Con offset
        sscanf(date_time, "%4d-%2d-%2dT%2d:%2d:%2d%6s", &year, &month, &day, &hour, &minute, &second, tz);
        // tz será como +02:00 o -03:30
        if ((tz[0] == '+' || tz[0] == '-') && tz[3] == ':') {
            offset_h = atoi(&tz[1]);
            offset_m = atoi(&tz[4]);
            if (tz[0] == '-') offset_h = -offset_h, offset_m = -offset_m;
            // Convertir hora local a UTC
            minute -= offset_m;
            hour -= offset_h;
            // Normalizar usando mktime
            struct tm t;
            t.tm_year = year - 1900;
            t.tm_mon = month - 1;
            t.tm_mday = day;
            t.tm_hour = hour;
            t.tm_min = minute;
            t.tm_sec = second;
            t.tm_isdst = 0;
            time_t ts = mktime(&t);
            struct tm *ut = gmtime(&ts);
            year = ut->tm_year + 1900;
            month = ut->tm_mon + 1;
            day = ut->tm_mday;
            hour = ut->tm_hour;
            minute = ut->tm_min;
            second = ut->tm_sec;
        } else {
            Serial.println("Invalid timezone format");
            return false;
        }
    }
    uint8_t rtc_year = year % 100;
    uint8_t rtc_month = month;
    uint8_t rtc_day = day;
    uint8_t rtc_hour = hour;
    uint8_t rtc_minute = minute;
    uint8_t rtc_second = second;

    // Calcular el día de la semana (algoritmo de Zeller)
    int m = (rtc_month == 1 || rtc_month == 2) ? rtc_month + 12 : rtc_month;
    int y = (rtc_month == 1 || rtc_month == 2) ? year - 1 : year;
    uint8_t dayOfWeek = (rtc_day + (13*(m+1))/5 + y + y/4 - y/100 + y/400) % 7;
    dayOfWeek = (dayOfWeek == 0) ? 7 : dayOfWeek; // 1=Domingo, 7=Sábado
	
    rtc.set(rtc_second, rtc_minute, rtc_hour, dayOfWeek, rtc_day, rtc_month, rtc_year);
    return true;
}

bool validate_date_time(const char* date_time) {
    // Acepta yyyy-mm-ddThh:mm:ssZ o yyyy-mm-ddThh:mm:ss±hh:mm
    int len = strlen(date_time);
    if (len != 20 && len != 25) return false;
    if (date_time[4] != '-' || date_time[7] != '-' || date_time[10] != 'T' || date_time[13] != ':' || date_time[16] != ':') return false;
    if (len == 20 && date_time[19] != 'Z') return false;
    if (len == 25 && !( (date_time[19] == '+' || date_time[19] == '-') && date_time[22] == ':')) return false;
    int year, month, day, hour, minute, second;
    if (len == 20) {
        if (sscanf(date_time, "%4d-%2d-%2dT%2d:%2d:%2dZ", &year, &month, &day, &hour, &minute, &second) != 6) return false;
    } else {
        int offset_h, offset_m;
        char sign;
        if (sscanf(date_time, "%4d-%2d-%2dT%2d:%2d:%2d%c%2d:%2d", &year, &month, &day, &hour, &minute, &second, &sign, &offset_h, &offset_m) != 9) return false;
        if (!(sign == '+' || sign == '-')) return false;
        if (offset_h < 0 || offset_h > 14 || offset_m < 0 || offset_m > 59) return false;
    }
    if (year < 2000 || year > 2099 || month < 1 || month > 12 || day < 1 || day > 31 || hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) return false;
    static const int daysInMonth[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
    int maxDays = daysInMonth[month];
    if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) maxDays = 29;
    if (day > maxDays) return false;
    return true;
}

int get_timezone_offset() {
    int offset = 0;
    EEPROM.get(EEPROM_ADDR_TZ_OFFSET, offset);
    return offset;
}

void set_timezone_offset(int offset_minutes) {
    EEPROM.put(EEPROM_ADDR_TZ_OFFSET, offset_minutes);
}

void init_timezone_offset(int default_offset_minutes) {
    int test;
    EEPROM.get(EEPROM_ADDR_TZ_OFFSET, test);
    // Si la EEPROM está vacía (0xFFFF o 0x0000), inicializa
    if (test == 0xFFFF || test == 0x0000) {
        set_timezone_offset(default_offset_minutes);
    }
}
