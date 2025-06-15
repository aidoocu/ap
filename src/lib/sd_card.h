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
 * @return Número de bytes leídos + 1 (el cierre de cadena) o 0 si hubo error 
 */
uint32_t sd_read_csv_row(size_t offset, char * sd_buffer);

/** Lee múltiples filas del archivo CSV desde una posición de byte específica (offset)
 * hasta llenar el buffer, asegurando que solo incluya filas completas.
 * Esta función garantiza que el buffer termina con una línea completa y no
 * con una línea cortada a la mitad.
 * 
 * @param offset Posición en bytes donde comenzar a leer (normalmente después del último timestamp enviado)
 * @param sd_buffer Buffer donde se almacenarán las filas leídas, terminado con un '\0'
 * @param buffer_size Tamaño máximo del buffer, incluyendo el byte NULL de terminación
 * @return Número de bytes leídos o 0 si hubo error
 */
uint32_t sd_read_csv_rows(size_t offset, char * sd_buffer, uint16_t buffer_size);

/** Busca el date_time en el archivo de la SD y devuelve la posición de la próxima línea
 * Esta función busca una línea que contenga el timestamp especificado y devuelve
 * la posición de byte donde comienza la siguiente línea, para continuar leyendo desde ahí
 * 
 * @param date_time Cadena con el timestamp en formato ISO 8601 UTC: "yyyy-mm-ddThh:mm:ssZ"
 * @return 0 si no se encuentra, o la posición de bytes de la SIGUIENTE línea después de la coincidencia
 * @note Para uso con server_last_update_request: encuentra dónde continuar la lectura después del último timestamp enviado
*/
uint32_t sd_date_time_next_line_search(char * date_time);

/** Guarda en memoria el último timestamp enviado al servidor y su offset correspondiente
 * @param timestamp Cadena con el timestamp en formato ISO 8601 UTC: "yyyy-mm-ddThh:mm:ssZ"
 * @param offset Posición en bytes donde comienza la siguiente línea después del timestamp
*/
void sd_save_last_timestamp_offset(const char* timestamp, size_t offset);

/** Recupera de la memoria el último timestamp enviado al servidor y su offset
 * @param timestamp Buffer donde se copiará el último timestamp guardado
 * @return true si hay un timestamp guardado, false si no hay datos guardados
*/
size_t sd_get_last_timestamp_offset(char * timestamp);

#endif /* _SD_CARD_H_ */