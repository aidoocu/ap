/** 
 * 
 * 
 * 
 */

#include "rtc.h"

uRTCLib rtc(RTC_ADDR);

void time(char * time) {

	rtc.refresh();

    /* dd/mm/aa,hh:mm:ss */
	sprintf(time, "%02d/%02d/%02d,%02d:%02d:%02d", rtc.day(), rtc.month(), rtc.year(), rtc.hour(), rtc.minute(), rtc.second());
	

}