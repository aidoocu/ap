/** 
 * 
 * 
 * 
 * 
 */

#include "sd_card.h"

static File sd_file;

// Tamaño máximo de timestamp (yyyy-mm-ddThh:mm:ssZ = 20 caracteres + '\0')
#define MAX_TIMESTAMP_SIZE 21

// Variables estáticas para almacenar el último timestamp y su offset
static char last_timestamp[MAX_TIMESTAMP_SIZE] = {0};
static size_t last_offset = 0;

void sd_init(){

	if(!SD.begin(SD_CSN))
		//Aqui hay que hacer un manejo de error
		//informar al remoto que no se pudo inicializar la tarjeta
		Serial.println("SD Card Error");
	else
		Serial.println("SD Card OK");
	
	/* Para estabilizar la tarjeta */
	delay(100);

}



uint32_t sd_file_size(void){

	sd_file = SD.open(SD_DATALOG);
	
	if(sd_file) {
		size_t file_size = sd_file.size();
		sd_file.close();
		return file_size;
	}
	
	return 0;
}

uint32_t sd_record(char * sd_buffer){

	sd_file = SD.open(SD_DATALOG, O_APPEND | O_WRITE);

	if(sd_file) {
		size_t printed = sd_file.println(sd_buffer);
		sd_file.close();
		return printed;
	}

	return 0;
}

uint32_t sd_read(uint8_t * sd_buffer, size_t buff_size, uint32_t offset, uint32_t sd_file_size){

	sd_file = SD.open(SD_DATALOG);

	if(sd_file) {

		/* Posicionamos el offset dentro del archivo */
		sd_file.seek(offset, SeekSet);

		/** Leer un bloque hasta el tamanno del buffer o en su defecto
			 hasta que se acabe el archivo. */
		size_t bytes_read = min((buff_size), (sd_file_size - offset));
		
/*  		Serial.print("ofst: ");
		Serial.println(offset);
		Serial.print("f-ofst: ");
		Serial.println(sd_file_size - offset);		
		Serial.print("trd: ");
		Serial.println(bytes_read);
		delay(20); */

		bytes_read = sd_file.read(sd_buffer, bytes_read);
		sd_file.close();
		return (bytes_read);
	}
		
	// Responder error si no se puede leer el archivo
	return 0;

}

bool sd_open_and_seek(const char * filename, size_t offset) {
	sd_file = SD.open(filename);
	if (!sd_file) {
		Serial.println("Error al abrir el archivo SD");
		return false;
	}
	
	// Posicionar el puntero en el offset especificado
	if (!sd_file.seek(offset)) {
		Serial.println("Error al buscar la posición en el archivo");
		sd_file.close();
		return false;
	}
	
	return true;
}

/**
 * Lee una fila del archivo CSV que comienza en la posición de byte específica (offset)
 * @param offset Posición en bytes donde comienza la fila a leer
 * @param sd_buffer Buffer donde se almacenará la fila leída
 * @return Número de bytes leídos o 0 si hubo error
 */
uint32_t sd_read_csv_row(size_t offset, char * sd_buffer){
	sd_file = SD.open(SD_DATALOG);

	if (!sd_open_and_seek(SD_DATALOG, offset)) {
		return 0; // Error al abrir o posicionar el archivo
	}
	
	// Leer hasta encontrar un salto de línea o fin de archivo
	size_t len = sd_file.readBytesUntil('\n', sd_buffer, 127);
	sd_buffer[len] = '\0'; // Asegurarse de que termina con null
	
	sd_file.close();

	/* Debug */
	Serial.print("Fila leída: ");
	Serial.println(sd_buffer);

	return len;
}

/**
 * Lee múltiples filas del archivo CSV desde una posición de byte específica (offset)
 * hasta llenar el buffer, asegurando que solo incluya filas completas.
 * Esta función es crítica para leer datos después de la última actualización enviada al servidor.
 * 
 * La función garantiza que:
 * 1. El buffer termina con una fila completa (no cortada a mitad)
 * 2. Todas las filas están terminadas correctamente con NULL
 * 3. No se produce desbordamiento de buffer
 * 
 * @param offset Posición en bytes donde comenzar a leer (normalmente desde la siguiente línea después del último timestamp enviado)
 * @param sd_buffer Buffer donde se almacenarán las filas leídas, terminando con un '\0'
 * @param buffer_size Tamaño máximo del buffer, incluyendo el byte NULL de terminación
 * @return Número de bytes leídos o 0 si hubo error
 */
uint32_t sd_read_csv_rows(size_t offset, char * sd_buffer, uint16_t buffer_size){

	if (!sd_open_and_seek(SD_DATALOG, offset)) {
		return 0; // Error al abrir o posicionar el archivo
	}
		
	// Leer el archivo completo hasta el final o hasta que el buffer se llene
	int16_t bytes_read = sd_file.readBytes(sd_buffer, buffer_size);

	sd_file.close();

	/* Si no se leen bytes nada que hacer */
	if(!bytes_read) {
		Serial.print("no bytes readed");
		return 0;
	}

	/* 	Si se lee menos que buffer_size - 1 significa que se leyó hasta el final del archivo
		almacenar todo en el buffer como una cadena */
	if (bytes_read < buffer_size - 1) {
		Serial.print("Final del archivo alcanzado");

		/* Se cierra la cadena */
		sd_buffer[bytes_read] = '\0';
		return bytes_read;
	}
	/* Si se lee exactamente buffer_size puede pasar que se haya leido hasta el final del archivo
		pero de todas formas se tratara como si hubieran mas datos para una próxima iteración,
		pues simplemente esa última fila, aunque completa, no cabe en el buffer ante la imposibilidad
		de poner un '\0'. En este caso buscaremos un '\n' para determinar el final de la fila anterior. 
		Un caso poco probable es una linea exactamente igual que el buffer, en este caso tampoco se 
		tendrá en cuenta pues simplemente no se podrá incluir '\0'. */
	bytes_read--; // Ajustamos para comenzar en el último byte válido
	while(bytes_read > 0 && sd_buffer[bytes_read] != '\n'){
		bytes_read--;
	}

	/* Si se encontró un salto de línea o se acabo la cadena (bytes_read == 0), terminamos la cadena ahí */
	sd_buffer[bytes_read] = '\0';
	return bytes_read;
}


/**
 * Guarda en memoria el último timestamp enviado al servidor y su offset correspondiente
 * @param timestamp Cadena con el timestamp en formato ISO 8601 UTC: "yyyy-mm-ddThh:mm:ssZ"
 * @param offset Posición en bytes donde comienza la siguiente línea después del timestamp
*/
void sd_save_last_timestamp_offset(const char* timestamp, size_t offset) {
    
	// Guardar el timestamp en la variable estática
    strncpy(last_timestamp, timestamp, MAX_TIMESTAMP_SIZE - 1);
    last_timestamp[MAX_TIMESTAMP_SIZE - 1] = '\0'; // Asegurar terminación
    
    // Guardar el offset
    last_offset = offset;
    
    // Debug
    Serial.print("Guardado en memoria timestamp: ");
    Serial.print(last_timestamp);
    Serial.print(" con offset next line: ");
    Serial.println(last_offset);
}

bool sd_is_last_timestamp_cached(char* timestamp) {

	/* Verificamos si tenemos el timestamp cacheado en memoria */
	if(last_timestamp[0] != '\0' && last_offset > 0) {
		// Si el timestamp cacheado es igual al que estamos buscando, devolvemos el offset cacheado
		if (strcmp(last_timestamp, timestamp) == 0) {
			return true; // Hay un timestamp guardado y es el mismo que se busca
		}
	}
	
	return false; // No hay un timestamp guardado
}

/**
 * Busca en el archivo CSV la línea que contiene la fecha y hora especificada
 * y devuelve la posición de la siguiente línea después de la coincidencia.
 * Esta función primero comprueba si hay una versión cacheada en memoria del timestamp y su offset.
 * 
 * @param date_time Timestamp a buscar en el archivo
 * @return La posición de la siguiente línea después de la coincidencia, o 0 si no se encuentra
 */
size_t sd_date_time_next_line_search(char * date_time){

	/* Verificamos si tenemos el timestamp cacheado en memoria */
	if (sd_is_last_timestamp_cached(date_time)) {
		// Si el timestamp cacheado es igual al que estamos buscando, devolvemos el offset cacheado
		Serial.print("Timestamp cacheado encontrado con offset: ");
		Serial.println(last_offset);
		return last_offset; // Retorna el offset cacheado
	}


	// Si no tenemos caché o el timestamp es diferente, procedemos con la búsqueda normal
	sd_file = SD.open(SD_DATALOG);

	if(sd_file) {

		/* Leemos linea por linea hasta encontrar la fecha y hora */
		while (sd_file.available()) {
			// Guardamos la posición actual antes de leer la línea
			
			char line[128];
			size_t len = sd_file.readBytesUntil('\n', line, sizeof(line) - 1);
			line[len] = '\0';			/* Verificamos si la linea contiene la fecha y hora */
			if (strstr(line, date_time) != NULL) {
				/* Si se encuentra, devolvemos el offset de la SIGUIENTE línea,
				   que es la posición actual del archivo */
				size_t offset = sd_file.position();
				sd_file.close();
                
                // Guardamos el resultado en caché para futuras consultas
                sd_save_last_timestamp_offset(date_time, offset);
                
				return offset;
			}
			/** @TODO:
				Teniendo en cuenta que estamos buscando la ultima actualizacion, puede el servidor responda con
				un date_time diferente a los registrados, asi que se debe buscar al menos la linea mas "cercana" en
				tiempo, por ejemplo: el date_time contiene "06/05/25,20:17:18", pero esa linea no existe, pues para
				el servidor hubo un cambio en la fecha y hora o un error y en SD hay un registro diferente, por ejemplo
				de minutos anteriores o posteriores. En este caso la linea mas cercana en el tiempo tendra la inmediata
				en tiempo a la que se esta buscando en caso que esta no exista. */
				
		}

		sd_file.close();
	}

	/* Debug */
	Serial.println("No se encontro la fecha y hora en el archivo de la SD");
	
	return 0; // No se encontro la fecha y hora
}
