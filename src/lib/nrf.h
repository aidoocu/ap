/**
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */

#ifndef _NRF24_H_
#define _NRF24_H_

#include <Arduino.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>

#define NFR_CE D3
#define NRF_CSN D4

void nrf_init(void);

bool nrf_check(char * request);

#endif /* _NRF24_H_ */