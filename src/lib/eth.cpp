/** 
 * 
 * 
 * 
 * 
 */

#include "eth.h"
#include <Arduino.h>
#include <SPI.h>



static EthernetServer tcp_server = EthernetServer(TCP_PORT);

static EthernetClient client;


/* Cuando la biblioteca EtherCard maneja paquetes TCP, 
los datos recibidos y enviados comparten el mismo buffer 
de red (Ethernet::buffer) */
//extern uint8_t Ethernet::buffer[ETH_BUFF];

void eth_init(void){

	byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
	//10.252.0.132
	//IPAddress local_ip(10, 252, 0, 241);
	//10.1.111.241
	IPAddress local_ip(10, 1, 111, 241);
	//IPAddress dns(10, 16, 1, 3); //defaults to the device IP address with the last octet set to 1
	//IPAddress gateway(10, 16, 22, 1); //defaults to the device IP address with the last octet set to 1
	//IPAddress subnet(255, 255, 255, 0); defaults to 255.255.255.0

	// Inicializar Ethernet
	Ethernet.init(ETHER_CSN);  // Asignar pin CS
	Ethernet.begin(mac, local_ip);
	
 	if (Ethernet.hardwareStatus() == EthernetNoHardware) {
		Serial.println(F("No Ethernet hardware found."));
	}

	tcp_server.begin();

}

void eth_update_cnx(void) {
	Ethernet.maintain();
}

uint16_t eth_check(char * request){

	client = tcp_server.available();
/* 
 	Serial.print("clt: ");
	Serial.println(client);
	Serial.println(client.available()); */


	if(client) {
		
		uint16_t bytes_available = client.available();

	 	Serial.print("received: ");
		Serial.print(bytes_available);
		Serial.println(" bytes");
		delay(100);

		if(bytes_available > 0) {
			/* leo el buffer de la red */
			//client.readBytes(Ethernet::buffer, bytes_available);
			/* leo el buffer de la red */
			client.readBytes(request, bytes_available);
			request[bytes_available] = '\0'; // Cerrar la cadena

			/* Debug */
			Serial.print("Req: ");
			Serial.println(request);
			delay(10);

			return bytes_available;

		}

		Serial.println("No hay nada que leer");
		client.stop();

	}
	
	return 0;

}

void eth_send(char * send_buffer) {

	/* envio la cadena incluyendo el cierre de cadena (+ 1) */
	client.write(send_buffer, strlen(send_buffer) + 1);

	client.stop();

}