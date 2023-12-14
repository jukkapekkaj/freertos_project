/*
 * GMP252.cpp
 *
 *  Created on: Nov 30, 2023
 *      Author: rabin
 */

#include "GMP252.h"

static void idle_delay()
{
	vTaskDelay(1);
}

GMP252::GMP252()
	:node_gmp(240),
	 co2_reg(&node_gmp, 256, true),
	 status_reg(&node_gmp, 2048, true)
{
	// TODO Auto-generated constructor stub
	this->node_gmp.begin(9600);
	this->node_gmp.idle(idle_delay);
}

GMP252::~GMP252() {}

int GMP252::readCO2()
{
	int co2 = co2_reg.read();
	return co2;
}

int GMP252::getStatus()
{
	int status = status_reg.read();
	//if(status != 1) {
	//	return -1;
	//}
	return status;
}
