/*
 * PlaintextMQTTExample.h
 *
 *  Created on: 6.12.2023
 *      Author: jpj1
 */

#ifndef MQTT_DEMO_PLAINTEXTMQTTEXAMPLE_H_
#define MQTT_DEMO_PLAINTEXTMQTTEXAMPLE_H_


#include "main.h"

extern QueueHandle_t menu_event_queue;
extern QueueHandle_t greenhouse_status_q;
extern QueueHandle_t mqtt_information_q;
extern QueueHandle_t co2_target_values;
extern TimerHandle_t enable_interrupt_timer;
extern SemaphoreHandle_t s;

//extern "C" {
// void vStartSimpleMQTTDemo( void ); // ugly - should be in a header
//}




#endif /* MQTT_DEMO_PLAINTEXTMQTTEXAMPLE_H_ */
