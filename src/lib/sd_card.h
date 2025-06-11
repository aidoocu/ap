/** 
 * 
 * 
 * 
 */

#ifndef _SD_CARD_H_
#define _SD_CARD_H_

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include "ap_config.h"

#define SD_CSN D8

void sd_init(void);

uint32_t sd_file_size(void);

uint32_t sd_record(char * sd_buffer);

/** Leer un bloque de datos de la SD 
 * @param sd_buffer  buffer donde se guardaran los datos leidos
 * @param size       cantidad de bytes a leer
 * @param offset     offset desde donde se comenzara a leer
 * @param sd_file_size  tamanno del archivo de la SD
 * @return cantidad de bytes leidos, 0 si no se pudo leer
*/
uint32_t sd_read(uint8_t * sd_buffer, size_t size, uint32_t offset, uint32_t sd_file_size);

/** Leer una fila del archivo CSV que comienza en la posición de byte específica (offset)
 * @param offset Posición en bytes donde comienza la fila a leer
 * @param sd_buffer Buffer donde se almacenará la fila leída
 * @return Número de bytes leídos o 0 si hubo error
 */
uint32_t sd_read_csv_row(size_t offset, char * sd_buffer);

/** Buscar el date_time en el archivo de la SD 
 * @param date_time  cadena con el formato ISO 8601 UTC: "yyyy-mm-ddThh:mm:ssZ"
 * @return 0 si no se encuentra, el offset de la linea si se encuentra
*/
uint32_t sd_date_time_search(char * date_time);

#endif /* _SD_CARD_H_ */