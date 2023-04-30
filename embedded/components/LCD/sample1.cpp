//============================================================================
// Name        : pigpioLED.cpp
// Author      : CA ABID
// Version     :
// Copyright   : 2021
// Description : Test sample using LCD driver
//============================================================================

#include "LCD.h"
#include <iostream>
#include <pigpio.h>
#include <chrono>
#include <thread>

using namespace std;

int main(int argc, char *argv[]) {
	if (gpioInitialise() < 0) {
		cout << "Failure..." << endl;
		exit(-1);
	}
	// Initialize the LCD driver
	LCD lcd(1, 0x27);
    lcd.enableCursor();
    lcd.enableBlinking();
	while (1) {
        lcd.setPosition(0, 0);
        lcd.putChar(65);  // Put char 'A'
        std::this_thread::sleep_for(std::chrono::seconds(1));
        lcd.setPosition(0, 1);
        lcd.putChar(66); // Put char 'B'
        std::this_thread::sleep_for(std::chrono::seconds(1));
        lcd.setPosition(0, 0);
        lcd.putChar(67); // Put char 'C'
        std::this_thread::sleep_for(std::chrono::seconds(1));
        lcd.setPosition(0, 1);
        lcd.putChar(68); // Put char 'D'
        lcd<<"ABC"; // Put string "ABC"
        std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	
	gpioTerminate();
	return 0;
}

