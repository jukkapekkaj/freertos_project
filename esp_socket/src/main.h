/*
 * main.h
 *
 *  Created on: 9.12.2023
 *      Author: jpj1
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#define SIG_A_PORT	0
#define SIG_A_PIN	5
#define SIG_B_PORT	0
#define SIG_B_PIN	6

#define ROTARY_ENCODER_PORT	1
#define ROTARY_ENCODER_PIN	8

typedef struct{
	volatile uint32_t CON;
	volatile uint32_t STAT;
	volatile uint32_t CONF;
	volatile uint32_t POS;
	volatile uint32_t MAXPOS;
	volatile uint32_t CMPOS0;
	volatile uint32_t CMPOS1;
	volatile uint32_t CMPOS2;
	volatile uint32_t INXCNT;
	volatile uint32_t INXCMP0;
	volatile uint32_t LOAD;
	volatile uint32_t TIME;
	volatile uint32_t VEL;
	volatile uint32_t CAP;
	volatile uint32_t VELCOMP;
	volatile uint32_t FILTERPHA;
	volatile uint32_t FILTERPHB;
	volatile uint32_t FILTERINX;
	volatile uint32_t WINDOW;
	volatile uint32_t INXCMP1;
	volatile uint32_t INXCMP2;
	volatile uint32_t NOT_USED[993];
	volatile uint32_t IEC;
	volatile uint32_t IES;
	volatile uint32_t INTSTAT;
	volatile uint32_t IE;
	volatile uint32_t CLR;
	volatile uint32_t SET;
}LPC_QEI_T;

#define TEXT_LENGTH 	64
#define CRC_LENGTH		2

#define SSID_MEM_ADDR				(TEXT_LENGTH * 0)
#define PASSWORD_MEM_ADDR			(TEXT_LENGTH * 1)
#define BROKER_MEM_ADDR				(TEXT_LENGTH * 2)


#define LPC_QEI  ((LPC_QEI_T *) LPC_QEI_BASE)

typedef struct {
	int co2;
	int humidity;
	int temperature;
	int valve;
	int target_co2;
	//int errors;

} greenhouse_status;

typedef struct {
	char broker[TEXT_LENGTH];
	char wifi_ssid[TEXT_LENGTH];
	char wifi_password[TEXT_LENGTH];
} mqtt_information;



#endif /* MAIN_H_ */
