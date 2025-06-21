/** 
 * 
 * 
 * 
 */

#ifndef _AP_CONFIG_H_
#define _AP_CONFIG_H_

#include "nrf.h"
#include "rtc.h"
#include "sd_card.h"
#include "eth.h"
#include "wifi.h"

#include "web_request.h"

/* Tamano de los datos a guardar en cada  linea */
/* yyyy-mm-ddThh:mm:ssZ */
#define DATE_TIME_BUFF 21
/* id,tt.t,hh,v.mv,v.mv  -> temperatura,  humedad, volt sc, volt pv */
#define RECV_LENGTH 21
/* v.mv */
#define VOLT_AP_VATT 4
/* id -> node id */
#define NODE_ID 2
/* yyyy-mm-ddThh:mm:ssZ,v.mv,id,tt.t,hh,v.mv,v.mv - (46) */
#define FULL_LINE_BUFF DATE_TIME_BUFF + VOLT_AP_VATT + NODE_ID + RECV_LENGTH  // = 21 + 4 + 2 + 21 = 48

/* Archivo donde se guardan los datos */
#define SD_DATALOG "datalog.csv"

/** Batt volt */
#define BATT_PIN A0
//#define BATT_PIN A1
#define VOLT_MAX_REF 24.28   		/* Para un divisor 1M/240K, teniendo en cuenta que ya el Wemos tiene un divisor 220 kΩ/100 kΩ */


/* Global buffer, sera tan largo como lo que se va a transimitir por eth */
#define BUFFER_SIZE 2048  // 2KB


/* largo del header OK_RESP */
#define HEADER_OK_RESP_LENGTH (sizeof(ACK_OK_RESP) - 1) //Aqui el lee tambien el '\0' entonces -1
/* Pedazo maximo de memoria que se puede leer desde la SD para no devordar el buffer */
#define MAX_SD_READ_CHUNK BUFFER_SIZE - HEADER_OK_RESP_LENGTH - 1 //hay que dejar un caracter para el cierre de cadena

/* Timeout para esperar la respuesta del servidor */
#define REQUEST_TIMEOUT 15000 //5 segundos

// Direcciones EEPROM para configuraciones persistentes
// La dirección 0 ya no se usa para timezone offset (eliminado)
// Aquí puedes agregar más defines para otras configuraciones persistentes
// #define EEPROM_ADDR_OTRA_CONFIG 4


/* Largo del valor del campo error en la respuesta del servidor. Incluye el '\0' */
#define FIELD_ERROR_VALUE_LENGTH 4

#endif /* _AP_CONFIG_H_ */