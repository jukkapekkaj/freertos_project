/*
 * HMP60.cpp
 *
 *  Created on: Nov 30, 2023
 *      Author: rabin
 */

#include "HMP60.h"

static void idle_delay()
{
	vTaskDelay(1);
}

HMP60::HMP60()
	:node_hmp(241),
	 humidity_reg(&node_hmp, 256, true),
	 temp_reg(&node_hmp, 257, true),
	 status_reg(&node_hmp, 512, true)
{
	// TODO Auto-generated constructor stub
	this->node_hmp.begin(9600);
	this->node_hmp.idle(idle_delay);
}

HMP60::~HMP60() {}

int HMP60::readHumidity()
{
	int humidity = humidity_reg.read()/10;
	return humidity;
}

int HMP60::readTemp()
{
	int temp = temp_reg.read()/10;
	return temp;
}

int HMP60::getStatus()
{
	int status = status_reg.read();
	if(status == 1) {
		return 0;
	}
	return status;
}

