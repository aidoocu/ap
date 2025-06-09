/** 
 * 
 * 
 * 
 */

#include "rtc.h"

uRTCLib rtc(RTC_ADDR);


void get_time(char * time) {

	rtc.refresh();

    /* yyyy-mm-ddThh:mm:ssZ (formato ISO 8601 UTC) */
	sprintf(time, "20%02d-%02d-%02dT%02d:%02d:%02dZ", rtc.year(), rtc.month(), rtc.day(), rtc.hour(), rtc.minute(), rtc.second());
	

}

bool set_date_time(char * date_time) {
    // Solo acepta formato ISO 8601 (yyyy-mm-ddThh:mm:ssZ)
    if(!validate_date_time(date_time)) {
        Serial.println("Invalid date_time");
        return false;
    }
    uint8_t second, minute, hour, dayOfWeek, dayOfMonth, month, year;
    int fullYear;
    sscanf(date_time, "%4d-%hhu-%hhuT%hhu:%hhu:%hhuZ", 
            &fullYear, 
            &month, 
            &dayOfMonth, 
            &hour, 
            &minute, 
            &second);
    year = fullYear % 100;
    // Calcular el día de la semana (algoritmo de Zeller)
    int m = (month == 1 || month == 2) ? month + 12 : month;
    int y = (month == 1 || month == 2) ? fullYear - 1 : fullYear;
    dayOfWeek = (dayOfMonth + (13*(m+1))/5 + y + y/4 - y/100 + y/400) % 7;
    dayOfWeek = (dayOfWeek == 0) ? 7 : dayOfWeek; // 1=Domingo, 7=Sábado
    rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
    return true;
}

bool validate_date_time(const char* date_time) {
    // Solo acepta formato ISO 8601 UTC (yyyy-mm-ddThh:mm:ssZ)
    if (strlen(date_time) != 20) 
        return false;
    if (date_time[4] != '-' || date_time[7] != '-' || 
        date_time[10] != 'T' || date_time[13] != ':' || 
        date_time[16] != ':' || date_time[19] != 'Z') 
        return false;
    int year, month, day, hour, minute, second;
    if (sscanf(date_time, "%4d-%2d-%2dT%2d:%2d:%2dZ", 
              &year, &month, &day, &hour, &minute, &second) != 6) {
        return false;
    }
    if (year < 2000 || year > 2099 || 
        month < 1 || month > 12 ||
        day < 1 || day > 31 ||
        hour < 0 || hour > 23 ||
        minute < 0 || minute > 59 ||
        second < 0 || second > 59) {
        return false;
    }
    static const int daysInMonth[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
    int maxDays = daysInMonth[month];
    if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)))
        maxDays = 29;
    if (day > maxDays)
        return false;
    return true;
}
