/*
 * memory.cpp
 *
 *  Created on: 5.12.2023
 *      Author: jpj1
 */

#include "board.h"

#include "memory.h"
#include <string.h>
#include "crc_15xx.h"
#include "FreeRTOS.h"
#include "task.h"



void memory_initialize(void) {
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_EEPROM);
	Chip_SYSCTL_PeriphReset(RESET_EEPROM);

	Chip_CRC_Init();
	Chip_CRC_UseCRC16();
}

uint32_t eeprom_read(uint32_t eeprom_address, uint8_t *dst, uint32_t length){
	static IAP iap_entry = (IAP) IAP_LOCATION;
    uint32_t buffer[5] = {0};
    uint32_t return_buffer[5] = {0};

    buffer[0] = 62; // read EEPROM
    buffer[1] = eeprom_address;
    buffer[2] = (uint32_t) dst;
    buffer[3] = length;
    buffer[4] = SystemCoreClock / 1000;

    vTaskSuspendAll();
    iap_entry(buffer, return_buffer);
    xTaskResumeAll();

    return return_buffer[0];
}

uint32_t eeprom_write(uint32_t eeprom_address, uint8_t *src, uint32_t length){
	static IAP iap_entry = (IAP) IAP_LOCATION;
	uint32_t buffer[5] = {0};
	uint32_t return_buffer[5] = {0};

	buffer[0] = 61; // write EEPROM
	buffer[1] = eeprom_address;
	buffer[2] = (uint32_t) src;
	buffer[3] = length;
	buffer[4] = SystemCoreClock / 1000;

	vTaskSuspendAll();
    iap_entry(buffer, return_buffer);
    xTaskResumeAll();

    return return_buffer[0];
}

bool load_and_validate_string(uint32_t eeprom_address, char *dst){
	char buffer[TEXT_LENGTH];
	int result = eeprom_read(eeprom_address, (uint8_t *)buffer, TEXT_LENGTH);
	if (result != 0){
		return false;
	}
	bool valid_string = false;
	for(int i = 0; i < (TEXT_LENGTH - CRC_LENGTH - 1); i++){
		if(buffer[i] == '\0'){
			if(i > 0){
				valid_string = true;
				break;
			}
			else { // Cant be valid string if first character is null terminating character
				return false;
			}

		}
	}

	if(valid_string){
		uint32_t crc = Chip_CRC_CRC8((uint8_t*)buffer, strlen(buffer) + 1 + CRC_LENGTH);
		if(crc == 0){
			memcpy(dst, buffer, strlen(buffer) + 1);
			return true;
		}
		else {
			return false;
		}
	}
	return false;
}

bool save_string(uint32_t eeprom_address, const char *src){
	char buffer[TEXT_LENGTH];
	uint32_t length = strlen(src);
	memcpy(buffer, src, length + 1);
	uint32_t crc = Chip_CRC_CRC8((uint8_t *)buffer, length + 1);
	// Append CRC to end
	buffer[length + 1] = crc >> 8;
	buffer[length + 2] = crc;
	int result = eeprom_write(eeprom_address, (uint8_t*)buffer, length + 1 + CRC_LENGTH); // string length + terminating null + two CRC bytes

	return result == 0;
}




