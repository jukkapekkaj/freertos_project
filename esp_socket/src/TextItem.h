/*
 * TextItem.h
 *
 *  Created on: 29.11.2023
 *      Author: jpj1
 */

#ifndef TEXTITEM_H_
#define TEXTITEM_H_

#include "MenuItem.h"
#include "LiquidCrystal.h"
#include <string>
#include <string.h>

class TextItem : public MenuItem {
public:
	TextItem(LiquidCrystal *_lcd, std::string _title, void (*value_changed)(std::string &text) = nullptr);
	virtual ~TextItem();
	bool event(menuEvent e);
	void increment();
	void decrement();
	void accept();
	void cancel();
	void setFocus(bool focus);
	bool getFocus();
	void display();
	void save();
	std::string getValue();
	void setValue(std::string text);

private:
	static const int lcd_width;
	static const char chars[];
	static const int end;
	bool focus;
	std::string title;
	std::string value;
	std::string edit;
	//std::string chars;
	int new_char_pos;
	int cursor_pos;
	int edit_pos;
	bool edit_char;
	char current_char;
	int start_print_index;
	LiquidCrystal *lcd;
	void (*callback)(std::string &text);
};

#endif /* TEXTITEM_H_ */
