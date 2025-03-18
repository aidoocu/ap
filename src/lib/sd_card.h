/** 
 * 
 * 
 * 
 */

#ifndef _SD_CARD_H_
#define _SD_CARD_H_

#include <Arduino.h>
#include <SPI.h>
//#include <SD.h>
#include "sdfat/SdFat.h"

#include "ap_config.h"

#define SD_CSN 8

void sd_init(void);

uint32_t sd_file_size(void);

uint32_t sd_record(char * sd_buffer);


uint32_t sd_read(char * sd_buffer, uint16_t size, uint32_t offset, uint32_t sd_file_size);



#endif /* _SD_CARD_H_ */