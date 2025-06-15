/** 
 * 
 * 
 * 
 * 
 * 
 */

#ifndef _ETH_H_
#define _ETH_H_


#include <Arduino.h>
//#include <EtherCard.h>

/* Para reducir el consumo de memoria se ha desabilidato UDP y se han reducido varios parametros 
en uipethernet-conf.h */
#include "eth_enc/EthernetENC.h"
#include "eth_enc/EthernetUdp.h"

#define ETHER_CSN D0

#define TCP_PORT 2409

//#define ETH_BUFF 280

#define UDP_TX_PACKET_MAX_SIZE 128


void eth_init(void);

void eth_update_cnx(void);

/** 
 * @param request   direccion del puntero al comienzo del payload dentro del
 *                  buffer de ethernet
 * @return  el largo del payload (buffer que esta en request) 
 *           0 si no hay nada en request
 */
uint16_t eth_check(char * request);

/* As server */
void eth_response(char * send_buffer);

/* As client */
bool eth_request_connect(void);
bool eth_server_income(char * response_buffer);
void eth_send(char * send_buffer, size_t length = 0);
void eth_disconnect(void);

/** Envia datos al cliente
 * @param data_buffer  buffer con los datos a enviar
 * @return true si se pudo enviar los datos, false si no se pudo conectar al servidor
 * * @note Esta funcion espera a que se establezca la conexion con el servidor antes de enviar los datos
 * * @note El buffer debe estar terminado en '\0' para que se envíe correctamente
 * * @note El tamaño del buffer no es necesario, ya que se usa strlen para obtener el largo
 * * @todo El buffer se asume en todo momento que es data_buffer, incluido su tamano maximo por lo que 
 *         habria que implementar una verificacion de limites, o de cambio de buffer o cosas asi
 */
bool eth_client_send(char * data_buffer/* , size_t length */);
#endif /* ETH_H_ */