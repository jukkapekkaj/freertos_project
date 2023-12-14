/*
 * HMP60.h
 *
 *  Created on: Nov 30, 2023
 *      Author: rabin
 */

#ifndef HMP60_H_
#define HMP60_H_

#include "ModbusMaster.h"
#include "ModbusRegister.h"

class HMP60 {
public:
	HMP60();
	virtual ~HMP60();
	int readHumidity();
	int readTemp();
	int getStatus();
private:
	ModbusMaster node_hmp;
	ModbusRegister humidity_reg;
	ModbusRegister temp_reg;
	ModbusRegister status_reg;
};

#endif /* HMP60_H_ */
