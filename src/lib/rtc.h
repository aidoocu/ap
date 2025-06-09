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


/** Esta funcion escribe la cadena en formato ISO 8601: yyyy-mm-ddThh:mm:ssZ o yyyy-mm-ddThh:mm:ss±hh:mm
*  a partir de la hora UTC almacenada en el RTC y un offset opcional.
*  El buffer debe tener al menos DATE_TIME_BUFF caracteres.
*  Si no se especifica offset, devuelve en UTC (Z).
*/
void get_time(char * time, int tz_offset_minutes = 0);

/**
 * Esta funcion valida la cadena de fecha y hora
 * en el formato ISO 8601: yyyy-mm-ddThh:mm:ssZ o yyyy-mm-ddThh:mm:ss±hh:mm
 * Devuelve true si es correcta, false si no lo es
 */
bool validate_date_time(const char* date_time);

/**
 * Esta funcion setea la fecha y hora en el RTC a partir de una cadena ISO 8601 UTC o con offset.
 * Convierte la hora local+offset a UTC antes de guardar en el RTC.
 */
bool set_date_time(char * date_time);

/**
 * Configuración persistente de zona horaria (offset en minutos respecto a UTC)
 */
#define TZ_OFFSET_EEPROM_ADDR 0  // Dirección EEPROM donde se guarda el offset

// Devuelve el offset de zona horaria persistente (en minutos)
int get_timezone_offset();
// Guarda el offset de zona horaria persistente (en minutos)
void set_timezone_offset(int offset_minutes);
// Inicializa la zona horaria persistente (si no está configurada)
void init_timezone_offset(int default_offset_minutes);


#endif /* _RTC_H_ */