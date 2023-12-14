/*
 * TextItem.cpp
 *
 *  Created on: 29.11.2023
 *      Author: jpj1
 */

#include "TextItem.h"
#include "board.h"

const char TextItem::chars[] =
		")=?-_:; ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.,!\"'*#%&/()=?-_:; ABCDEF";
//              |---------------- These are the characters user can select ----------------------|
// Chars on left and right of those are just appended to the string
// so I can display possible characters from the same array and I don't have to copy characters anywhere

const int TextItem::end = strlen(chars) - 7 - 7 - 1;
const int TextItem::lcd_width = 16;


TextItem::TextItem(LiquidCrystal *_lcd, std::string _title, void (*value_changed)(std::string &text)) {
	// TODO Auto-generated constructor stub
	lcd = _lcd;
	title = _title;
	focus = false;
	new_char_pos = 0;
	edit_char = false;
	cursor_pos = -1;
	edit_pos = 0;
	callback = value_changed;
	start_print_index = 0;

}

TextItem::~TextItem() {
	// TODO Auto-generated destructor stub
}

bool TextItem::event(menuEvent e){
	bool handled = true;
	switch(e) {
	case ok:
		if(focus) {
			accept();
			//setFocus(false);
		}
		else {
			setFocus(true);
		}
		break;
	case back:
		if(focus) {
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
		if(focus) increment();
		else handled = false;
		break;
	case down:
		if(focus) decrement();
		else handled = false;
		break;
	}
	if(handled) display();

	return handled;
}

void TextItem::increment() {
	if(edit_char){
		new_char_pos++;
		if(new_char_pos > end){
			new_char_pos = 0;
		}
	}
	else {
		edit_pos++;
		if(cursor_pos == -1){
			cursor_pos = 0;
			edit_pos = 0;
			start_print_index = 0;
		}
		else if(edit_pos > edit.size()){
			cursor_pos = -1;
			edit_pos = 0;
		}
		else {
			if(cursor_pos < 15){
				cursor_pos++;
			}
			else {
				start_print_index++;
			}
		}
	}


}

void TextItem::decrement() {
	if(edit_char){
		new_char_pos--;
		if(new_char_pos < 0){
			new_char_pos = end;
		}
	}
	else {
		edit_pos--;
		if(cursor_pos == -1){
			edit_pos = edit.size();
			if(edit.size() > 15){
				cursor_pos = 15;
				start_print_index = edit.size() - 1 - 14;
			}
			else {
				cursor_pos = edit_pos;
				start_print_index = 0;
			}

		}
		else if(edit_pos < 0){
			cursor_pos = -1;
			edit_pos = 0;
		}
		else {
			if(cursor_pos > 0){
				cursor_pos--;
			}
			else {
				start_print_index--;
			}

		}
	}
}

void TextItem::accept() {
	if(cursor_pos == -1){
		save();
		setFocus(false);
	}
	else if(edit_char){
		std::string new_text;
		char new_character = chars[new_char_pos + 7];
		// Selecting space removes character and every character after that
		if(new_character != ' '){
			new_text = edit.substr(0, edit_pos) + new_character;
			// Add end of text after modified character
			if(edit_pos < (int)(edit.size() - 1)){
				new_text += edit.substr(edit_pos+1, std::string::npos);
			}
		}
		else {
			new_text = edit.substr(0, edit_pos);
		}

		edit = new_text;
		edit_char = false;
	}
	else {
		edit_char = true;
	}

}

void TextItem::cancel() {
	edit = value;
}


void TextItem::setFocus(bool focus) {
	this->focus = focus;
	if(focus){
		start_print_index = 0;
		edit_pos = 0;
		cursor_pos = -1;
	}
}

bool TextItem::getFocus() {
	return this->focus;
}

void TextItem::display() {
	const int buffer_size = lcd_width + 1;
	char top[buffer_size];
	char bottom[buffer_size];
	// Select what to print on top row of screen
	if(focus && cursor_pos == -1) {
		snprintf(top, buffer_size, "[%s]                ", title.c_str());
	}
	else if(edit_char){
		snprintf(top, buffer_size, " %-15s", &chars[new_char_pos]);
	}
	else {
		snprintf(top, buffer_size, "%-16s", title.c_str());
	}

	// Select what to print on bottom row of screen
	snprintf(bottom, buffer_size, "%-16s", &edit.c_str()[start_print_index]);


	lcd->setCursor(0,0);
	lcd->print(top);
	lcd->setCursor(0,1);
	lcd->print(bottom);

	if(focus){
		if(edit_char){
			lcd->noCursor();
			lcd->setCursor(8, 0);
			lcd->blink();
		}
		else {
			lcd->noBlink();
			if(cursor_pos == -1){
				lcd->noCursor();
			}
			else {
				lcd->setCursor(cursor_pos, 1);
				lcd->cursor();
			}
		}
	}
	else {
		lcd->noCursor();
	}
}


void TextItem::save() {
	// set current value to be same as edit value

		value = edit;
		DEBUGOUT("SAVED. New value is: %s\r\n", value.c_str());
		if(callback != nullptr){
			callback(this->value);
		}

	// todo: save current value for example to EEPROM for permanent storage
}

void TextItem::setValue(std::string text) {
	edit = text;
	save();
}


std::string TextItem::getValue() {
	return value;
}



