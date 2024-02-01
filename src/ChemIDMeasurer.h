//
// Class to collect data needed for measuring the Chem ID of a battery
//

#ifndef PLZDONTEXPLODE_CHEMIDMEASURER_H
#define PLZDONTEXPLODE_CHEMIDMEASURER_H

#include <BQ34Z100.h>

class ChemIDMeasurer
{
    I2C i2c;
	BQ34Z100 soc;

	// charger control pins
	DigitalIn chgPin;
	DigitalOut shdnPin;

	Timer totalTimer;
	Timer stateTimer;

	enum class State
	{
		INIT, // Initial state.
		CHARGE, // First, charge to full power until charge current <= C/100.
		RELAX_CHARGED, // Relax for two hours to reach open circuit voltage
		DISCHARGE, // Discharge at C/10 until the term voltage is reached
		RELAX_DISCHARGED, // Relax for five hours to reach open circuit voltage
		DONE // Measurement finished
	};
	State state = State::INIT;

	// Turn the charger on
	void activateCharger();

	// Turn the charger off.
	void deactivateCharger();

	// Change current state.
	void setState(State newState);

public:
	ChemIDMeasurer();

	/**
	 * Loop to run the ID measurement
	 */
	void runMeasurement();
};


#endif //PLZDONTEXPLODE_CHEMIDMEASURER_H
