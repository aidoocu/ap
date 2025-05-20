#include <Arduino.h>
#include <stdlib.h>
#include <SPI.h>

/* #ifdef __AVR_ATmega328P__
#include <avr/wdt.h>
#endif */

//#ifdef ESP826
//#include <esp_task_wdt.h>
//#endif

#include "lib/ap_config.h"

static char data_buffer[BUFFER_SIZE];
static char * payload_progress;		//Valorar si es necesario que sea char o se puede poner uint8_t
static unsigned long global_timer;

                    	/* 10 min */ /* 5 seg */
#define GLOBAL_PERIOD 600000 /* 10000 */

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

bool update_sd(void) {

	/* Get data */
	char rtc_date_time[DATE_TIME_BUFF];
	get_time(rtc_date_time); //time;
	
	char battery_voltage[5];
	dtostrf(analogRead(BATT_PIN) * VOLT_MAX_REF / 1023, 4, 2, battery_voltage); //voltage

	char local_block[] = { 'A', 'P', ',', '\"', '\"', ',','\"', '\"', ',', '\"', '\"', ',', '\"', '\"', ',', '\"', '\"','\0' };
	char received_buffer[RECV_LENGTH];
	char * store_to_sd = received_buffer;

	/* 	Si el buffer es vacio significa que no hay nada para procesar, 
		entonces se guarda el bloque local */
	if (data_buffer[0] == '\0') {
		store_to_sd = local_block;
	/* Si no esta vacio hay que guardar su contenido en la memoria */
	} else {
		/* Si el buffer es mayor a 21 bytes, significa que hay un problema con el payload */
		/* !!!!! En este caso hay que hacer una verificacion del payload completo antes de guardarlo */
		if(strlen(data_buffer) > 21)
			return false;

		sprintf(store_to_sd, "%s", data_buffer);

	}

	/* dd/mm/aa,hh:mm:ss,v.mv,id,tt.t,hh,v.mv,v.mv - (43 + 1 '\0') */
	sprintf(data_buffer, "%s,%s,%s", rtc_date_time, battery_voltage, store_to_sd);

	Serial.print("record: ");
	Serial.println(data_buffer);
	delay(20);

	/* 	Si la operacion en la memoria no es exitosa cae en un lazo infinito hasta que
		el micro se reinicia por WDT */
 	if(!sd_record(data_buffer)) {
		return false;
	}
	
	/* No esta de mas resetear el WDT */
	wdt_reset();

	/* Debug */
	//uint32_t data_lenght_recorded = sd_record(data_buffer);
 	//Serial.print(data_lenght_recorded);
	Serial.print(" - ");
	Serial.println(sd_file_size());
	delay(10);

	return true;

}


void setup(){

	Serial.begin(9600);

	Serial.println("Starting...");

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
	#ifdef __AVR_ATmega328P__
	analogReference(INTERNAL);
	#endif

	global_timer = millis() + 10000;

	/* habilitando el WDT */
	wdt_enable(WDTO_2S);

	/* parpadeo de LED as "hello"*/
	pinMode(LED_BUILTIN, OUTPUT);
	/*  */
	for (size_t i = 0; i < 3; i++) {
		digitalWrite(LED_BUILTIN, HIGH);
		delay(50);
		digitalWrite(LED_BUILTIN, LOW);
		delay(50);
	}


	Serial.println("Setup done!");

}

void loop() {

	eth_update_cnx();
	
	/* Resetear el WDT */
	wdt_reset();

	/* Chequea la radio y actualiza en la SD si hay algo nuevo */
  	if (nrf_check(data_buffer)) {
		update_sd();
	}

	/** Ejecucion de tareas de forma periodica */
	if(millis() > global_timer){

								  
		global_timer = millis() + GLOBAL_PERIOD;

		/* 	Sennalamos que el buffer esta vacio para que no lo 
		procese update_sd(); */
		data_buffer[0] = '\0';

		/* y actualizamos la SD */
		update_sd();

	}
	
	if (eth_check(data_buffer)) {


		/** y comenzamos a buscar los comandos en el payload 
		 * 	que son GET para las solicitudes SET para el seteo 
		 *  de parametros */

		/* ------------------------------------------ */
		/* Campo GET */
		if(strstr(data_buffer, "GET")){

			/** Buscar el parámetro offset en la solicitud, 
			Ejemplo de solicitud: "GET offset=1024" */
			payload_progress = strstr(data_buffer, "offset=");

			/* Si no hay un campo "offset=", como se llego aqui desde un GET */
			if (!payload_progress) {

				/* se devuelve que el recurso solicitado no existe */
				sprintf(data_buffer, ACK_ERR_NO_OFFSET);

			/* Si hay un campo "offset=" se procesa */
			} else {

				/* Saltamos la cadena "offset=" */
				payload_progress = payload_progress + 7;

				/* Lo convertimos a entero sin signo */ 
				char * endptr;
				eth_offset = strtol(payload_progress, &endptr, 10);
    			if ((* endptr != '\0') || (!isdigit((unsigned char)*payload_progress))) {
					/* 	El offset no es un numero entero positivo, por lo que no se puede procesar */
					sprintf(data_buffer, ACK_ERR_NO_OFFSET);

				} else {
					/* 	El offset es un numero entero positivo, por lo que se puede procesar */
					Serial.print("i: ");
					Serial.println(eth_offset);
					//delay(20);

					/* 	verificamos que sea consistente, asi que buscamos el tamanno del archivo 
						en la SD para hacer las comparaciones */
					uint32_t file_size = sd_file_size();

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
					el tamanno del archivo por lo que es valido para ser procesado. Se ha determinado
					que NO es igual que el tamnno del archivo por lo que hay datos nuevos que mandar */	
					} else {

						/* creamos un offset temporal  para guardar cuanto en realidad se leyo */
						uint32_t bytes_readed = 0;

						/* reutilizamos payload_progress para copiar los datos desde la SD hasta el chunk */
						sprintf(data_buffer, ACK_OK_RESP);
						payload_progress = &data_buffer[0] + HEADER_OK_RESP_LENGTH; // descontamos el cierre de cadena

						/* Se lee el archivo desde el offset  */
						bytes_readed = sd_read((uint8_t *)(payload_progress), (MAX_SD_READ_CHUNK), eth_offset, file_size);
						/* Poner el cierre de cadena */
						payload_progress[bytes_readed] = '\0';

						/* Si sd_read devuelve un 0 es que no se pudo leer el archivo asi que chunk se reescribe */
						if(!bytes_readed)
							/* El archivo no se abrio */
							sprintf(data_buffer, ACK_ERR_SERVER);
					}
				} /* offset contiene un numero */

			} /* Campo offset existe */
		
		} /* Campo GET */
		
		/* ------------------------------------------ */

		/* Campo SET */
		else if(strstr(data_buffer, "SET")){ 

			Serial.println(data_buffer);
			delay(50);

			/** Buscar el parámetro date_time en la solicitud, 
			Ejemplo de solicitud: "SET date_time=05,09,11,6,16,05,25" */
			if(strstr(data_buffer, "date_time=")) {

				/* El formato seria asi second, minute, hour, dayOfWeek, dayOfMonth, month, year
				 	SET date_time=ss,mm,hh,w,dd,mo,yy
					SET date_time=05,09,11,6,16,05,25 */				
				char * rtc_date_time = &data_buffer[14];

				/* Se setea la nueva fecha_hora */
				if (set_date_time(rtc_date_time)) {

					/* se da la respuesta al extremo */
					sprintf(data_buffer, ACK_SET_DATE_TIME);

					/* Debug */
					Serial.print("Setted: ");
					get_time(rtc_date_time);			
					Serial.println(rtc_date_time);
					delay(20);

				} else {

					/* No se pudo setear la fecha y hora */
					sprintf(data_buffer, ACK_BAD_DATETIME);
				}
				
			} else {

				/* No hay un campo valido dentro del "SET" */
				sprintf(data_buffer, ACK_ERR_NO_SET );

			}

		} /* Campo SET */

		/* ------------------------------------------ */

		/* Si se recibio algo que no se esperaba, la cadena se daclara vacia */
		else {
			data_buffer[0] = '\0';
		}

		Serial.println(data_buffer);
		delay(50);

		/* Sending to ethernet */
		eth_send(data_buffer);	

	} /* Algo se recibio */

}