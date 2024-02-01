/*
    USC RPL HAMSTER v2.3 BQ34Z100 Test Suite
    Contributors: Arpad Kovesdy
*/

#pragma once

#include "BQ34Z100.h"
#include "mbed.h"
#include "pins.h"

class SOCTestSuite {
public:
   void outputStatus();
   void sensorReset();
   void displayData();
   void testHamsterConnection();
   void startCal();
   void stopCal();
   void startIt();
   void writeSettings();
   void discharge();
   void relaxEmpty();
   void charge();
   void relaxFull();
   void calibrateVoltage();
   void calibrateCurrent();
   void resetVoltageCalibration();
   void testFloatConversion();
   void readVoltageCurrent();

private:
	void outputFlashInt(uint8_t* flash, int index, int len);
};
