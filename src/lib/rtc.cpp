/** 
 * 
 * 
 * 
 */

#include "rtc.h"
#include "ap_config.h"
#include <EEPROM.h>

uRTCLib rtc(RTC_ADDR);

void get_time(char * time) {

    rtc.refresh();
    
	// Obtener hora UTC del RTC
    int year = 2000 + rtc.year();
    int month = rtc.month();
    int day = rtc.day();
    int hour = rtc.hour();
    int minute = rtc.minute();
    int second = rtc.second();
    
    // Formato UTC simple
    sprintf(time, "%04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour, minute, second);
}

bool set_date_time(char * date_time) {
    // Acepta formato ISO 8601 UTC (Z)
    if(!validate_date_time(date_time)) {
        Serial.println("Invalid date_time");
        return false;
    }
    int year, month, day, hour, minute, second;
    
    // Solo formato UTC
    sscanf(date_time, "%4d-%2d-%2dT%2d:%2d:%2dZ", &year, &month, &day, &hour, &minute, &second);
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
    
    // Solo acepta el formato yyyy-mm-ddThh:mm:ssZ

    if (strcmp(date_time, "0000-00-00T00:00:00Z") == 0) {
        return true; // Permitir el valor por defecto para evitar errores
    } 

    int len = strlen(date_time);
    if (len != 20) return false;
    if (date_time[4] != '-' || date_time[7] != '-' || date_time[10] != 'T' || 
        date_time[13] != ':' || date_time[16] != ':' || date_time[19] != 'Z') return false;
    
    int year, month, day, hour, minute, second;
    if (sscanf(date_time, "%4d-%2d-%2dT%2d:%2d:%2dZ", &year, &month, &day, &hour, &minute, &second) != 6) return false;
    
    if (year < 2000 || year > 2099 || month < 1 || month > 12 || day < 1 || day > 31 || 
        hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) return false;
    
    static const int daysInMonth[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
    int maxDays = daysInMonth[month];
    if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) maxDays = 29;
    if (day > maxDays) return false;
    
    return true;
}

// Timezone functions removed - all timestamps are now in simple UTC format
