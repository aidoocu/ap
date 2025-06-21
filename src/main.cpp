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

                    	/* 5 seg */ /* 10 min */ 
#define GLOBAL_PERIOD   /* 5000 */  600000

static uint32_t eth_offset = 0;

/* Variable para guardar el voltaje de la bateria */
static float battery_voltage = 0.0;
/* Contador para forzar la actualización cada 5 ciclos */
static uint8_t battery_update_counter = 0;

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
	
	/* Buffer para crear el bloque local que in */
	char store_to_sd[RECV_LENGTH + 1]; // +1 para ',' en caso de los motes

	/* 	Si el buffer es vacio significa que no hay nada para procesar, 
		entonces se guarda el bloque local */
	if (data_buffer[0] == '\0') {
		char battery_voltage[5];
		dtostrf(analogRead(BATT_PIN) * VOLT_MAX_REF / 1023, 4, 2, battery_voltage); //voltage
		sprintf(store_to_sd, "AP,,,,,%s", battery_voltage);
	/* Si no esta vacio hay que guardar su contenido en la memoria */
	} else {
		/* Si el buffer es mayor a RECV_LENGTH bytes, significa que hay un problema con el payload */
		/* !!!!! (ToDo) En este caso hay que hacer una verificacion del payload completo antes de guardarlo */
		if(strlen(data_buffer) > RECV_LENGTH)
			return false;
		/* Ponemos una coma al final para recrear la columna de batt */
		snprintf(store_to_sd, sizeof(store_to_sd), "%s,", data_buffer);

	}

	/* Limpiamos el buffer para que no se procese de nuevo */
	data_buffer[0] = '\0'; 
	/* yyyy-mm-ddThh:mm:ssZ,id,tt.t,hh,v.mv,v.mv,v.mv - (46 + 1 '\0') */
	sprintf(data_buffer, "%s,%s", rtc_date_time, store_to_sd);

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

/** Esta funcion busca un campo en el payload (data_buffer) y copia su valor en value 
 * * @param field El nombre del campo a buscar en el payload
 * * @param value El buffer donde se guardara el valor del campo encontrado
 * * @return El largo del valor extraido, 0 si no se encontro el campo
*/
uint32_t get_payload_field(const char * field, char * value) {

	/* 	Busca el campo en el payload y copia su valor en value */
	char * field_ptr = strstr(data_buffer, field);
	if (field_ptr == NULL) {
		value[0] = '\0'; // Si no se encuentra el campo, se devuelve una cadena vacía
		Serial.print("Campo no encontrado: ");
		Serial.println(field);
		return 0;
	}

	/* Avanzamos hasta el valor (después del primer':') */
	field_ptr = strchr(field_ptr, ':');
	if (field_ptr == NULL) {
		value[0] = '\0'; // Si no hay ':', se devuelve una cadena vacía
		Serial.print("Formato inválido: ");
		return 0;
	}

	/* Saltar el ':' mas posibles espacios en blanco y comillas iniciales si tuviera*/
	field_ptr++;
	while (*field_ptr == ' ' || *field_ptr == '\t' || *field_ptr == '"')
		field_ptr++;

	/* Extraer el valor hasta la comilla final, la coma, salto de línea o fin de cadena */
	int i = 0;
	while (i < DATE_TIME_BUFF - 1 &&
	      *field_ptr != '\0' &&
	      *field_ptr != '"' &&
	      *field_ptr != '\r' &&
	      *field_ptr != '\n' &&
	      *field_ptr != ',') {
		value[i++] = *field_ptr++;
	}
	
	value[i] = '\0'; // Cerrar la cadena

	/* Debug */
/* 	Serial.print("Campo encontrado: ");
	Serial.print(field);
	Serial.print(" - Valor: ");
	Serial.println(value);
	Serial.print("Largo del valor: ");
	Serial.println(i); */
	
	/* Retornamos el largo del valor extraido */
	return i;
}

/** Esta funcion realiza una solicitud al servidor para obtener la fecha y hora de la ultima actualizacion 
 * 	y a partir de la respuesta actualiza el con los datos locales recogidos
 * 	@note Esta funcion espera una respuesta del servidor (bloqueante) y no retorna hasta que la recibe respuesta
 * 	o timeout.
 */
void server_last_update_request(void){

	/* ---------------------------- Solicitud de último update ---------------------------- */

	/* Debug */
	Serial.println("Iniciando actualizacion del servidor...");
	/* 	Se envia un GET al servidor para que este responda con la fecha y hora de la ultima actualizacion */
	/* 	El servidor debera responder con un payload de la forma: "yyyy-mm-ddThh:mm:ssZ" (formato ISO 8601 UTC) */
	
	data_buffer[0] = '\0'; // Limpiamos el buffer

	/* Si estamos conectados a la WiFi y esta activa, usamos el cliente de WiFi   */
	if(wifi_is_connected()) {

		/* Se ensambla el request */
		sprintf(data_buffer, GET_LAST_MEASUREMENT_HEADER, HTTPS_RENDER_HOST);
		
		/* Enviamos la solicitud al servidor */
		if(!wifi_client_send(data_buffer)) {
			/* Si no se pudo enviar la solicitud al servidor, no hay nada que hacer */
			Serial.println("Fallo al enviar la solicitud de timestamp al servidor");
			return;
		}
	} else {

		/* Si no estamos conectados a la WiFi, usamos el cliente Ethernet */
		sprintf(data_buffer, GET_LAST_MEASUREMENT_HEADER, HTTP_LOCAL_HOST);

		/* Enviamos la solicitud al servidor */
		if(!eth_client_send(data_buffer)) {
			/* Si no se pudo enviar la solicitud al servidor, no hay nada que hacer */
			Serial.println("Fallo al enviar la solicitud de timestamp al servidor");
			return;
		}
	}

	char timestamp[DATE_TIME_BUFF];
	/* Si el campo no se encontró, o no es del largo correcto nada mas que hace */
	if (get_payload_field("timestamp", timestamp) != DATE_TIME_BUFF - 1) {
		Serial.println("Timestamp no encontrado o invalido");
		return;
	}

	/* Verificamos que el timestamp sea válido */
	if (!validate_date_time(timestamp)) {
		Serial.println("Formato de timestamp inválido");
		return;
	}

	/* ---------------------------- Búsqueda en la SD del último timestamp ---------------------------- */

	/* 	El buffer de datos se va a llenar con el encabezado y los datos del CSV 
		para enviarlo al servidor */
	char data_buffer_body[POST_CSV_BODY_LENGTH]; // Buffer para los datos del CSV

	/* !!!!! ToDo 
	
		El mecaniasmo de cahe no esta bien implementado, y ahora mismo hace nada con el offset y el 
		timestamp guardados y la memoria se lee buscando siempre la ocurrencia del timestamp.
	
	*/
	/* Mecanismo para asociar un offset a la última fecha/hora despachada.
	   La función sd_date_time_next_line_search ya implementa un sistema de caché 
	   que guarda y recupera automáticamente el último timestamp y su offset correspondiente.
	   De esta forma cuando se recibe una petición con el mismo timestamp, se recupera 
	   directamente el offset sin necesidad de buscar en todo el archivo. */
	size_t offset = 0;
	size_t file_size = sd_file_size(); // Tamaño del archivo en la SD

	/* Si el timestamp es válido, buscamos la línea en la SD que lo contiene */
	if(strcmp(timestamp, "0000-00-00T00:00:00Z") == 0) {
		/* Si el timestamp es el inicial, no hay datos en el servidor por lo que se
		enviará el CSV completo */
		Serial.println("Timestamp inicial, se enviará el CSV completo");
		/* Señalamos que se enviará el CSV desde el principio */
		offset = 1;
	} else {
		offset = sd_date_time_next_line_search(timestamp);
		/* Si no se encontró un offset válido */
		if (!offset) {
			Serial.println("No se encontró el offset del timestamp en la SD");
			return;
		}
	}
	/* Debug - Mostrar el offset encontrado */
	Serial.print("Offset encontrado: ");
	Serial.println(offset);
	
	/* Aqui vamos a preparar el csv de respuesta. Para ello vamos a tomar la primera
	fila del archivo CSV que contiene los encabezados para que servidor pueda separar 
	las columnas por variables */

	/* ---------------------------- Preparación del CSV para enviar ---------------------------- */

	/* A partir de aqui vamos a ir llenando el buffer con los datos del CSV */

	/* Mientras que haya datos en el archivo se enviaran */
	while((offset - 1) < file_size) {

		/* Leemos la primera fila del CSV, que contiene los encabezados */
		uint8_t data_buffer_pointer = sd_read_csv_row(0, data_buffer_body);

		/* Si no se pudo leer la primera fila, no hay nada que hacer */
		if (!data_buffer_pointer) {
			Serial.println("No se pudo leer la primera fila del CSV");
			return;
		}

		/* Si es el primer offset, se salta el encabezado */
		if(offset == 1) {
			offset += data_buffer_pointer;
		}

		/* Cambiamos el '\0' por un salto de línea al final de la primera fila */
		data_buffer_body[data_buffer_pointer] = '\n';
		/* Aumentamos el largo del buffer leido */
		data_buffer_pointer++;
		/* Ponemos el cierre de cadena */
		data_buffer[data_buffer_pointer] = '\0';
		/* No se avanza pues ese cierre o se usara como terminador en caso que no haya mas datos
			o sera sustituido por sd_read_csv_rows con el primer caracter de la proxima fila */

		/* Leemos las filas desde el offset encontrado hasta llenar el buffer o fin de archivo 
		se usan dos variables para evitar que se envien filas vacias pero esto puede quedar mas elegante*/
		size_t bytes_readed_in_rows = sd_read_csv_rows(offset, data_buffer_body + data_buffer_pointer, POST_CSV_BODY_LENGTH - data_buffer_pointer);
		offset += bytes_readed_in_rows;
		if (!bytes_readed_in_rows) {
			Serial.println("No se pudo leer las filas del CSV");
			return;
		}

		/* Aumentamos el offset para que sea el inicio de la siguiente fila (se salta el '\n' de la fila leida) */
		offset++;

		/* Ahora es que se crea el encabezado que debera contener el campo Content-Length */
		data_buffer[0] = '\0';



		/* Si estamos conectados a la WiFi y esta activa, usamos el cliente de WiFi   */
		if(wifi_is_connected()) {

			/* Se ensambla el request */
			sprintf(data_buffer, POST_CSV_HEADER, HTTPS_RENDER_HOST, strlen(data_buffer_body));
			strcat(data_buffer, data_buffer_body); // Concatenamos el buffer de datos al encabezado
			
			/* Enviamos la solicitud al servidor */
			if(!wifi_client_send(data_buffer)) {
				/* Si no se pudo enviar la solicitud al servidor, no hay nada que hacer */
				Serial.println("Update fail WiFi");
				return;
			}
		} else {

			/* Si no estamos conectados a la WiFi, usamos el cliente Ethernet */
			sprintf(data_buffer, POST_CSV_HEADER, HTTP_LOCAL_HOST, strlen(data_buffer_body));
			strcat(data_buffer, data_buffer_body); // Concatenamos el buffer de datos al encabezado

			/* Enviamos la solicitud al servidor */
			if(!eth_client_send(data_buffer)) {
				/* Si no se pudo enviar la solicitud al servidor, no hay nada que hacer */
				Serial.println("Update fail Eth");
				return;
			}
		}

	}

	/* Limpiamos el buffer de datos */
	data_buffer[0] = '\0';
	/* Actualizamos el offset para la proxima vez */
	//send_offset = offset + sd_file_size(); // Actualizamos el offset para la proxima vez

}


void setup(){

	Serial.begin(115200);

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

	/* Inicializamos la WiFi */
	wifi_init();

	/* Referecia del ADC a 1.1V */
	#ifdef __AVR_ATmega328P__
	analogReference(INTERNAL);
	#endif

	/* habilitando el WDT */
	wdt_enable(WDTO_8S);

	/* parpadeo de LED as "hello"*/
	pinMode(LED_BUILTIN, OUTPUT);
	/*  */
	for (size_t i = 0; i < 3; i++) {
		digitalWrite(LED_BUILTIN, HIGH);
		delay(50);
		digitalWrite(LED_BUILTIN, LOW);
		delay(50);
	}

	server_last_update_request();

	global_timer = millis() + 10000;

	Serial.println("Setup done!");

}

void loop() {

	eth_update_cnx();
	
	/* Resetear el WDT */
	wdt_reset();
	
	/** Ejecucion de tareas de forma periodica */
	if(millis() > global_timer){

								  
		global_timer = millis() + GLOBAL_PERIOD;

		/* 	Sennalamos que el buffer esta vacio para que no lo 
		procese update_sd(); */
		data_buffer[0] = '\0';

		/* Solo se actualizan los datos si hay un cambio significativo o cada 5 ciclos */
		float new_battery_voltage = analogRead(BATT_PIN) * VOLT_MAX_REF / 1023;
		
		/* Incrementar el contador en cada ciclo */
		battery_update_counter++;

		/* Actualizar si hay cambio significativo o si han pasado 5 ciclos */
		if (fabs(battery_voltage - new_battery_voltage) > 0.1 || battery_update_counter >= 5) { 
			battery_voltage = new_battery_voltage;
			/* Reiniciar el contador después de una actualización */
			battery_update_counter = 0;
			update_sd();
		}

		/* verificamos si hay algo que hay actualizar en el servidor y se actualiza de ser el caso */
		server_last_update_request();

	}

	/* Chequea la radio y actualiza en la SD si hay algo nuevo */
  	if (nrf_check(data_buffer)) {
		update_sd();
		server_last_update_request();
	}

	/* Verificamos si hay datos nuevos en la conexión Ethernet */
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
            Ejemplo de solicitud: "SET date_time=2025-06-09T14:32:10Z" */
            if(strstr(data_buffer, "date_time=")) {
                // El formato debe ser ISO 8601 UTC: yyyy-mm-ddThh:mm:ssZ
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
		eth_response(data_buffer);	

	} /* Algo se recibio */

}