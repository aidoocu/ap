/** 
 * 
 * 
 * 
 * 
 */

#include "sd_card.h"

static File sd_file;

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

/**
 * Lee una fila del archivo CSV que comienza en la posición de byte específica (offset)
 * @param offset Posición en bytes donde comienza la fila a leer
 * @param sd_buffer Buffer donde se almacenará la fila leída
 * @return Número de bytes leídos o 0 si hubo error
 */
uint32_t sd_read_csv_row(size_t offset, char * sd_buffer){
	sd_file = SD.open(SD_DATALOG);

	if (!sd_file) {
		Serial.println("Error al abrir el archivo SD");
		return 0;
	}
	
	// Posicionar el puntero en el offset especificado
	if (!sd_file.seek(offset)) {
		Serial.println("Error al buscar la posición en el archivo");
		sd_file.close();
		return 0;
	}
	
	// Leer hasta encontrar un salto de línea o fin de archivo
	size_t len = sd_file.readBytesUntil('\n', sd_buffer, 127);
	sd_buffer[len] = '\0'; // Asegurarse de que termina con null
	
	sd_file.close();
	return len;
}

uint32_t sd_date_time_search(char * date_time){

	sd_file = SD.open(SD_DATALOG);

	if(sd_file) {

		/* Leemos linea por linea hasta encontrar la fecha y hora */
		while (sd_file.available()) {
			char line[128];
			size_t len = sd_file.readBytesUntil('\n', line, sizeof(line) - 1);
			line[len] = '\0';

			/* Verificamos si la linea contiene la fecha y hora */
			if (strstr(line, date_time) != NULL) {
				/* Si se encuentra, devolvemos el offset de la linea */
				uint32_t offset = sd_file.position() - len - 1; // -1 por el salto de linea
				sd_file.close();
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
