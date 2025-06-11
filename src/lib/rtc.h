/** 
 * 
 * 
 */

#ifndef _RTC_H_
#define _RTC_H_

 /* Include la biblioteca oficial */
#include <Arduino.h>
#include <uRTCLib.h>
#include <EEPROM.h>

#define RTC_DEV DS3231

#if RTC_DEV == DS3231

#define RTC_ADDR 0x68

#endif /* RTC_DEV == RTC_DEV_DEFAULT */


/** Esta funcion escribe la cadena en formato ISO 8601: yyyy-mm-ddThh:mm:ssZ
*  a partir de la hora UTC almacenada en el RTC.
*  El buffer debe tener al menos DATE_TIME_BUFF caracteres.
*/
void get_time(char * time);

/**
 * Esta funcion valida la cadena de fecha y hora
 * en el formato ISO 8601: yyyy-mm-ddThh:mm:ssZ
 * Devuelve true si es correcta, false si no lo es
 */
bool validate_date_time(const char* date_time);

/**
 * Esta funcion setea la fecha y hora en el RTC a partir de una cadena ISO 8601 UTC.
 */
bool set_date_time(char * date_time);

// No timezone handling needed, all timestamps are in simple UTC format


#endif /* _RTC_H_ */