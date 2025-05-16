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
