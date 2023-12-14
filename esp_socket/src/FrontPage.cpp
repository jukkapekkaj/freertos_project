/*
 * FrontPage.cpp
 *
 */

#include "FrontPage.h"
#include <cstdio>

FrontPage::FrontPage(LiquidCrystal *lcd_,
		int _co2,
		int _co2_target,
		int _humidity,
		int _temperature,
		int _valve):
lcd(lcd_), co2(_co2), co2_target(_co2_target), humidity(_humidity), temperature(_temperature), valve(_valve) {
	focus = false;
	values_changed = false;
}

FrontPage::~FrontPage() {
}

bool FrontPage::event(menuEvent e){
	bool handled = true;

	switch(e) {
	case ok:
		handled = false;
		break;
	case back:
		handled = false;
		break;
	case show:
		break;
	case up:
		handled = false;
		break;
	case down:
		handled = false;
		break;
	}

	if(handled) display();

	return handled;
}

void FrontPage::increment() {
}

void FrontPage::decrement() {

}

void FrontPage::accept() {

}

void FrontPage::cancel() {

}


void FrontPage::setFocus(bool focus) {

}

bool FrontPage::getFocus() {
	return this->focus;
}

void FrontPage::display() {

		char top[17];
		char bottom[17];

		snprintf(top, 17, "CO2:%-4d -> %-4d", co2, co2_target);
		snprintf(bottom, 17, "T%-2d RH%-3d V%-3d%%", temperature, humidity, valve);

		lcd->setCursor(0,0);
		lcd->print(top);
		lcd->setCursor(0, 1);
		lcd->print(bottom);



}


void FrontPage::save() {

}


void FrontPage::set_co2(int val){
	if(val != co2){
		co2 = val;
		values_changed = true;
	}
}

void FrontPage::set_co2_target(int val){
	if(val != co2_target){
		co2_target = val;
		values_changed = true;
	}

}

void FrontPage::set_humidity(int val){
	if(val != humidity){
		humidity = val;
		values_changed = true;
	}
}

void FrontPage::set_temperature(int val){
	if(val != temperature){
		temperature = val;
		values_changed = true;
	}

}

void FrontPage::set_valve(int val){
	if(val != valve){
		valve = val;
		values_changed = true;
	}

}

