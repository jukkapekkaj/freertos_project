
#ifndef INTEGERITEM_H_
#define INTEGERITEM_H_

#include "LiquidCrystal.h"
#include "MenuItem.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <string>

class IntegerItem : public MenuItem {
public:
	IntegerItem(LiquidCrystal *lcd_,
			std::string editTitle,
			void (*value_changed)(int value),
			int _min = 0,
			int _max = 10000,
			int _step = 10);
	virtual ~IntegerItem();
	bool event(menuEvent e);
	void increment();
	void decrement();
	void accept();
	void cancel();
	void setFocus(bool focus);
	bool getFocus();
	void display();
	int getValue();
	void setValue(int value);
private:
	void save();
	void displayEditValue();
	LiquidCrystal *lcd;
	void (*callback)(int value);
	std::string title;
	int value;
	int edit;
	bool focus;
	int min;
	int max;
	int step;
};


#endif /* INTEGERITEM_H_ */
