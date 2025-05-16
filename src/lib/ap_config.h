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


/* Tamano de los datos a guardar en cada  linea */
/* dd/mm/aa,hh:mm:ss */
#define DATE_TIME_BUFF 18
/* id,tt.t,hh,v.mv,v.mv  -> temperatura,  humedad, volt sc, volt pv */
#define RECV_LENGTH 21
/* v.mv */
#define VOLT_AP_VATT 4
/* id -> node id */
#define NODE_ID 2
/* dd/mm/aa,hh:mm:ss,v.mv,id,tt.t,hh,v.mv,v.mv - (43) */
#define FULL_LINE_BUFF DATE_TIME_BUFF + VOLT_AP_VATT + NODE_ID + RECV_LENGTH

/* Archivo donde se guardan los datos */
#define SD_DATALOG "datalog.csv"

/** Batt volt */
#define BATT_PIN A0
//#define BATT_PIN A1
#define VOLT_MAX_REF 5.635   		/* Para un divisor 1M/240K */

/* Mensajes */
#define ACK_OK_RESP         "ACK=200 pl="   //Recurso encontrado, a continuacion viene la respuesta
#define ACK_OK_UPDATED      "ACK=201"       //Recurso actualizado, nada nuevo que enviar
#define ACK_SET_DATE_TIME   "ACK=202"       //Actualizacion de la fecha y hora del RTC
#define ACK_ERR_SERVER      "ACK=500"       //Erro en el servidor. El archivo no abre o esta completamente en blanco
#define ACK_ERR_OFFSET      "ACK=402"       //El offset es mayor que el tamannno del archivo 
#define ACK_ERR_NO_OFFSET   "ACK=403"       //No hay offset en la peticion
#define ACK_ERR_NO_SET      "ACK=404"       //Recurso para SET no encontrado o SET no permitido
#define ACK_ERR_NO_GET      "ACK=405"       //Recurso para GET no encontrado o GET no permitido
#define ACK_ERR_NO_RECORD   "ACK=406"       //Error al grabar en la SD
#define ACK_BAD_DATETIME    "ACK=407"       //Error al grabar en la SD

/* Global buffer, sera tan largo como lo que se va a transimitir por eth */
#define BUFFER_SIZE 128
/* largo del header OK_RESP */
#define HEADER_OK_RESP_LENGTH (sizeof(ACK_OK_RESP) - 1) //Aqui el lee tambien el '\0' entonces -1
/* Pedazo maximo de memoria que se puede leer desde la SD para no devordar el buffer */
#define MAX_SD_READ_CHUNK BUFFER_SIZE - HEADER_OK_RESP_LENGTH - 1 //hay que dejar un caracter para el cierre de cadena

#endif /* _AP_CONFIG_H_ */