/** 
 * 
 * 
 */

#ifndef _RTC_H_
#define _RTC_H_

 /* Include la biblioteca oficial */
#include <Arduino.h>
#include <uRTCLib.h>

#define RTC_DEV DS3231

#if RTC_DEV == DS3231

#define RTC_ADDR 0x68

#endif /* RTC_DEV == RTC_DEV_DEFAULT */


/** Esta funcion escribe la cadena dd/mm/aa,hh:mm:ss
*  a partir time. Esta funcion NO chequea el largo 
*  del buffer, por lo que al ser llamada hay que asegurarse
*  que el buffer apuntado por time sea no menos que 
*  DATE_TIME_BUFF 18
*/
void get_time(char * time);


/**  
 * Esta funcion valida la cadena de fecha y hora
 * en el formato dd/mm/aa,hh:mm:ss
 * 
 * Devuelve true si es correcta, false si no lo es
 */
bool validate_date_time(const char* date_time);

/** Esta funcion setea la fecha y hora */
bool set_date_time(char * date_time);




#endif /* _RTC_H_ */