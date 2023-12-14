/*
 * IntegerItem.cpp
 *
 *  Created on: 2.2.2016
 *      Author: krl
 */

#include "IntegerItem.h"
#include <cstdio>

IntegerItem::IntegerItem(LiquidCrystal *lcd_, std::string editTitle, void (*value_changed)(int value), int _min, int _max, int _step):
lcd(lcd_), title(editTitle), min(_min), max(_max), step(_step) {
	value = 0;
	edit = 0;
	focus = false;
	callback = value_changed;
}

IntegerItem::~IntegerItem() {
}

bool IntegerItem::event(menuEvent e){
	bool handled = true;
	switch(e) {
	case ok:
		if(getFocus()) {
			accept();
			setFocus(false);
		}
		else {
			setFocus(true);;
		}
		break;
	case back:
		if(getFocus()) {
			cancel();
			setFocus(false);
		}
		else {
			handled = false;
		}
		break;
	case show:
		break;
	case up:
		if(getFocus()) increment();
		else handled = false;
		break;
	case down:
		if(getFocus()) decrement();
		else handled = false;
		break;
	}
	if(handled) display();

	return handled;
}

void IntegerItem::increment() {
	edit += step;
	if(edit > max){
		edit = max;
	}

}

void IntegerItem::decrement() {
	edit -= 10;
	if(edit < min){
		edit = min;
	}
}

void IntegerItem::accept() {
	save();
}

void IntegerItem::cancel() {
	edit = value;
}


void IntegerItem::setFocus(bool focus) {
	this->focus = focus;
}

bool IntegerItem::getFocus() {
	return this->focus;
}

void IntegerItem::display() {
	//lcd->clear();
	lcd->setCursor(0, 0);
	lcd->print("                 ");
	lcd->setCursor(0, 1);
	lcd->print("                 ");

	lcd->setCursor(0,0);
	lcd->print(title);
	lcd->setCursor(0,1);
	char s[17];
	if(focus) {
		snprintf(s, 17, "     [%4d]     ", edit);
	}
	else {
		snprintf(s, 17, "      %4d      ", edit);
	}
	lcd->print(s);
	lcd->setCursor(0, 1);
}


void IntegerItem::save() {
	// set current value to be same as edit value
	value = edit;
	if(callback != nullptr){
		callback(value);
	}
	// todo: save current value for example to EEPROM for permanent storage
}


int IntegerItem::getValue() {
	return value;
}
void IntegerItem::setValue(int value) {
	if(value > max){
		value = max;
	}
	else if(value < min){
		value = min;
	}
	edit = value;
	save();
}

