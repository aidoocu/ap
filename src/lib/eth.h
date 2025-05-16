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



void eth_send(char * send_buffer);

#endif /* ETH_H_ */