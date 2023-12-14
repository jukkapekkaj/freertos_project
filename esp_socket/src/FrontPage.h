
#ifndef FRONTPAGE_H_
#define FRONTPAGE_H_

#include "LiquidCrystal.h"
#include "MenuItem.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

class FrontPage : public MenuItem {
public:
	FrontPage(LiquidCrystal *lcd_,
			int _co2 = 0,
			int _co2_target = 0,
			int _humidity = 0,
			int _temperature = 0,
			int _valve = 0
			);
	virtual ~FrontPage();
	bool event(menuEvent e);
	void increment();
	void decrement();
	void accept();
	void cancel();
	void setFocus(bool focus);
	bool getFocus();
	void display();
	void set_co2(int val);
	void set_co2_target(int val);
	void set_humidity(int val);
	void set_temperature(int val);
	void set_valve(int val);
private:
	void save();
	void displayEditValue();
	LiquidCrystal *lcd;
	bool focus;
	int co2;
	int co2_target;
	int humidity;
	int temperature;
	int valve;
	bool values_changed;
};


#endif /* FRONTPAGE_H_ */
