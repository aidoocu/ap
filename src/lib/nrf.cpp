/** 
 * 
 * 
 * 
 * 
 * 
 * 
 */


#include "nrf.h"
#include "ap_config.h"

static uint8_t nrf_address[] = {0xE8, 0xE8, 0xF0, 0xF0, 0xE1};

static RF24 nrf_radio(NFR_CE, NRF_CSN);

void nrf_init(void) {
    
    if(!nrf_radio.begin())
        Serial.println('2');

    nrf_radio.openReadingPipe(1, nrf_address);
    nrf_radio.startListening();

}

bool nrf_check(char * request) {

    if (nrf_radio.available()) {

		/* Se lee hasta el caracter RECV_LENGTH y ... */
		nrf_radio.read(request, RECV_LENGTH);
        
        /* Debug */
        //char request_buf[] = {"MR,28.1,67,3.22,1.76"};
        //sprintf(request, "%s", request_buf);

		/* en RECV_LENGTH se cierra la cadena al largo de los datos */
		request[RECV_LENGTH - 1] = '\0';

        /** @note En realidad el '\0' debe venir en el caracter RECV_LENGTH, 
         * pero por seguridad se fuerza el cierre de la cadena
         */
        //Serial.println(request);
        //delay(20);

        return true;
    }

    return false;
    
}