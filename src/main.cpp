#include <Arduino.h>
#include <stdlib.h>
#include <SPI.h>

#include "lib/ap_config.h"

static char data_buffer[BUFFER_SIZE];
static char * payload_progress;
static unsigned long global_timer;

static uint32_t eth_offset = 0;

/* Esta funcion verifica si fuera necesario la integridad del offset y lo convierte en entero */
uint32_t str_to_int(char * str_offset) {

	/* Saltar caracteres no numéricos (espacios, letras, etc.) */
	uint8_t i = 0;
	char offset_temp[10];

	while ((i < sizeof(offset_temp) - 1) && (isdigit((unsigned char)str_offset[i]))){
		offset_temp[i] = str_offset[i];
		i++;
	}

	/*  Hasta aqui se cierra una cadena de largo max sizeof(offset_temp) - 1 y solo 
		contiene digitos */
	offset_temp[i] = '\0';

	/* y se combierte en un uint32 */
	return (atoi(offset_temp));

}

void update_sd(void) {

	/* Get data */
	char rtc_date_time[DATE_TIME_BUFF];
	time(rtc_date_time); //time;
	
	char battery_voltage[5];
	dtostrf(analogRead(BATT_PIN) * VOLT_MAX_REF / 1023, 4, 2, battery_voltage); //voltage

	char local_block[] = { 'A', 'P', ',', '\"', '\"', ',','\"', '\"', ',', '\"', '\"', ',', '\"', '\"', ',', '\"', '\"','\0' };
	char received_buffer[RECV_LENGTH];
	char * store_to_sd = received_buffer;

	if (data_buffer[0] == '\0') {
		store_to_sd = local_block;
	} else {
		sprintf(received_buffer, "%s", data_buffer);
	}

	/* dd/mm/aa,hh:mm:ss,v.mv,id,tt.t,hh,v.mv,v.mv - (43) */
	sprintf(data_buffer, "%s,%s,%s", rtc_date_time, battery_voltage, store_to_sd);

	Serial.print("record: ");
	Serial.println(data_buffer);
	delay(20);

	//sd_record(data_buffer);
	
	/* Debug */
/* 	Serial.print(sd_record(data_buffer));
	Serial.print(" - ");
	Serial.println(sd_file_size());
	delay(10); */

	Serial.println(sd_record(data_buffer));

	delay(10);
}


void setup(){

	Serial.begin(9600);

	/* RTC init */
	URTCLIB_WIRE.begin();

    SPI.begin();

	/* SD init */
	sd_init();

	/* Eth init */
	eth_init();

	/* Radio init */
	nrf_init();

	/* Referecia del ADC a 1.1V */
	analogReference(INTERNAL);	

	global_timer = millis() + 5000;

}

void loop() {

	//eth_update_cnx();

	/* Chequea la radio y actualiza en la SD si hay algo nuevo */
  	if (/* nrf_check(data_buffer) */ (millis() + 10) > global_timer){
		Serial.println("u");
		nrf_check(data_buffer);
		//Serial.println(data_buffer);
		update_sd();
		delay(10);
	}
	
	if (0/* eth_check(data_buffer) */ /*  millis() > global_timer */) {

		//Serial.println(data_buffer);
		//delay(20);

		/** y comenzamos a buscar los comandos en el payload 
		 * 	que son GET para las solicitudes SET para el seteo 
		 *  de parametros */
		if(strstr(data_buffer, "GET")){

			//Serial.println(data_buffer);
			//delay(50);

			/** Buscar el parámetro offset en la solicitud, 
			Ejemplo de solicitud: "GET offset=1024" */
			payload_progress = strstr(data_buffer, "offset=");

			//Serial.println(payload_progress);
			//delay(20);

			/* Si no hay un campo "offset=", como se llego aqui desde un GET */
			if (!payload_progress) {

				/* se devuelve que el recurso solicitado no existe */
				sprintf(data_buffer, ACK_ERR_NO_OFFSET);

			/* Si hay un campo "offset=" se procesa */
			} else {

				/* Saltamos la cadena "offset=" */
				payload_progress = payload_progress + 7;

				/* Lo convertimos a entero sin signo */
				//eth_offset = str_to_int(payload_progress);
				eth_offset = atoi(payload_progress);

 				Serial.print("i: ");
				Serial.println(eth_offset);
				//delay(20);

				/* 	verificamos que sea consistente, asi que buscamos el tamanno del archivo 
					en la SD para hacer las comparaciones */
				uint32_t file_size = sd_file_size();
				//uint32_t file_size = 1100;

 				//Serial.print("s: ");
				//Serial.println(file_size);
				//delay(20);

				/* 	Si file_size es 0 puede ser o que hay un error al abrir el archivo o esta esta 
					completamente en blanco. El segundo caso no es un error en si mismo, aunque implica
					nada que enviar de respuesta. Aun asi se enviara un error de servidor */
				if(!file_size) {
					/* Error de servidor */
					sprintf(data_buffer, ACK_ERR_SERVER);

				/* El offset recibido debe estar entre 0 y el tamaño actual del archivo en la tarjeta SD */
				} else if (eth_offset > file_size) { 
					/* offset mas grande que archivo */
					sprintf(data_buffer, ACK_ERR_OFFSET);

				/* Si el offset recibido es igual al tamanno del archivo en la SD es que no hay datos nuevos */	
				} else if (eth_offset == file_size) { 
					/* Nada nuevo que actualizar */
					sprintf(data_buffer, ACK_OK_UPDATED);

				/*  Hasta aqui se ha encontrado un offset, que es un numero entero positivo entre 0 
				el tamanno del archivo por lo que es valido para ser procesado. No se ha determinado
				que NO es igual que el tamnno del archivo por lo que hay datos nuevos que mandar */	
				} else {

					/* creamos un offset temporal  para guardar cuanto en realidad se leyo */
					uint32_t bytes_readed = 0;

					/* reutilizamos payload_progress para copiar los datos desde la SD hasta el chunk */
					sprintf(data_buffer, ACK_OK_RESP);
					payload_progress = &data_buffer[0] + HEADER_OK_RESP_LENGTH; // descontamos el cierre de cadena

					//Serial.println(data_buffer);
					//delay(20);

					/* Se lee el archivo desde el offset  */
					bytes_readed = sd_read(payload_progress, (MAX_SD_READ_CHUNK), eth_offset, file_size);
					/* Poner el cierre de cadena */
					payload_progress[bytes_readed] = '\0';

					/* Serial.print(bytes_readed);
					Serial.print(" - ");
					Serial.println(data_buffer);
					delay(50); */

					/* Si sd_read devuelve un 0 es que no se pudo leer el archivo asi que chunk se reescribe */
					if(!bytes_readed)
						/* El archivo no se abrio */
						sprintf(data_buffer, ACK_ERR_SERVER);
				}

			} /* Campo offset */

			/* Debug */
			//Serial.println(data_buffer);
			//delay(50);

			/* Sending to ethernet */
			eth_send(data_buffer);		
		
		} /* Campo GET */
		if(strstr(data_buffer, "SET")){

			//
		}

	} /* Algo se recibio */

	/** Ejecucion de tareas de forma periodica */
 	if(millis() > global_timer){

		Serial.println("d");
									/* 5 min */
		global_timer = millis() + /* 300000 */ 5000;

		/* 	Sennalamos que el buffer esta vacio para que no lo 
			procese update_sd(); */
		data_buffer[0] = '\0';

		delay(10);

		/* y actualizamos la SD */
		//update_sd();

	}

}