/** 
 * 
 * 
 * 
 * 
 */

#include "sd_card.h"

#include <Arduino.h>
#include <SPI.h>

//#define SPI_SPEED 4000000  // 4 MHz

//File data_file;

// File system object.
SdFat sd_card;

// Log file.
SdFile sd_file;

void sd_init(){

	//SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0));

	/* if(!SD.begin(SPI_FULL_SPEED, SD_CSN)) */
	if(!sd_card.begin(SD_CSN, SD_SCK_MHZ(4)))
		Serial.println("E1");
	
	/* Para estabilizar la tarjeta */
	delay(100);

}



uint32_t sd_file_size(void){
	
	if(sd_file.open(SD_DATALOG, O_RDONLY)) {

		uint32_t file_size = sd_file.fileSize();
		sd_file.close();
		return file_size;

	}
	
	return 0;
}

uint32_t sd_record(char * sd_buffer){

	if(sd_file.open(SD_DATALOG, O_RDWR)){

		/* Ponerse al final del archivo */
		sd_file.seekEnd();
		uint32_t printed = sd_file.println(sd_buffer);
		sd_file.close();

		return printed;

	}

	return 0;

}

uint32_t sd_read(char * sd_buffer, size_t buff_size, uint32_t offset, uint32_t sd_file_size){

	if (sd_file.open(SD_DATALOG, O_RDONLY)) {

		/* Posicionamos el offset dentro del archivo */
		sd_file.seekSet(offset);

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

		/* Cerrar el archivo */
		sd_file.close();

		return (bytes_read);
	}
		
	// Responder error si no se puede leer el archivo
	return 0;

}
