/*
 * memory.h
 *
 *  Created on: 5.12.2023
 *      Author: jpj1
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdint.h>
#include <stdbool.h>

#define TEXT_LENGTH 	64
#define CRC_LENGTH		2
#define IAP_LOCATION	0x03000205UL

typedef void (*IAP)(uint32_t[], uint32_t[]);





void memory_initialize();

uint32_t eeprom_read(uint32_t eeprom_address, uint8_t *dst, uint32_t length);
uint32_t eeprom_write(uint32_t eeprom_address, uint8_t *src, uint32_t length);

bool load_and_validate_string(uint32_t eeprom_address, char *dst);
bool save_string(uint32_t eeprom_address, const char *src);




#endif /* MEMORY_H_ */
