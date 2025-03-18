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
	IPAddress local_ip(10, 16, 22, 241);
	//IPAddress dns(10, 16, 1, 3); //defaults to the device IP address with the last octet set to 1
	//IPAddress gateway(10, 16, 22, 1); //defaults to the device IP address with the last octet set to 1
	//IPAddress subnet(255, 255, 255, 0); defaults to 255.255.255.0

	// Inicializar Ethernet
	Ethernet.init(ETHER_CSN);  // Asignar pin CS
	Ethernet.begin(mac, local_ip);
	
	tcp_server.begin();

}

void eth_update_cnx(void) {
	Ethernet.maintain();
}

uint16_t eth_check(char * request){

	client = tcp_server.available();
	
	if(client) {

		uint16_t i = 0;

		while(client.available()) {
			request[i] = client.read();
			i++;
		}
		/* cerrando la cadena despues del request */
		request[i] = '\0';

		//delay(100);

	  	return i;

  }

	return 0;
}

void eth_send(char * send_buffer) {

	/* envio la cadena incluyendo el cierre de cadena (+ 1) */
	client.write(send_buffer, strlen(send_buffer) + 1);

	client.stop();

}