/** 
 * 
 * 
 * 
 */

#include "rtc.h"

uRTCLib rtc(RTC_ADDR);


void get_time(char * time) {

	rtc.refresh();

    /* dd/mm/aa,hh:mm:ss */
	sprintf(time, "%02d/%02d/%02d,%02d:%02d:%02d", rtc.day(), rtc.month(), rtc.year(), rtc.hour(), rtc.minute(), rtc.second());
	

}

void set_date_time(char * date_time) {

	/** RTCLib::set(byte second, 
	 * byte minute, 
	 * byte hour (0-23:24-hr mode only),
	 * byte dayOfWeek (Sun = 1, Sat = 7), 
	 * byte dayOfMonth (1-12), 
	 * byte month, 
	 * byte year) */

	//Serial.println(date_time);
	//delay(20);

 	uint8_t second, minute, hour, dayOfWeek, dayOfMonth, month, year;

	// Parsear la cadena
	sscanf(date_time, "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu", 
			&second, 
			&minute, 
			&hour, 
			&dayOfWeek, 
			&dayOfMonth, 
			&month, 
			&year);
   
	// Configurar RTC
	rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year);

}