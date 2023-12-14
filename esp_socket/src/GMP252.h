/*
 * GMP252.h
 *
 *  Created on: Nov 30, 2023
 *      Author: rabin
 */

#ifndef GMP252_H_
#define GMP252_H_

#include "ModbusMaster.h"
#include "ModbusRegister.h"

class GMP252 {
public:
	GMP252();
	virtual ~GMP252();
	int readCO2();
	int getStatus();
private:
	ModbusMaster node_gmp;
	ModbusRegister co2_reg;
	ModbusRegister status_reg;
};

#endif /* GMP252_H_ */
