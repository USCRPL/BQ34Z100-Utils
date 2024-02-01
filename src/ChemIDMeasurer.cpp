//
// Created by jamie on 12/10/2020.
//

#include "ChemIDMeasurer.h"
#include <cinttypes>

#include "pins.h"

ChemIDMeasurer::ChemIDMeasurer():
soc(p9, p10, 100000),
acpPin(p22),
chgPin(p23),
progPin(p15),
shdnPin(p21)
{
	//Initially keep charger in shdn
	shdnPin.write(1);

	// Disable MCU pullup resistors as they are enough to screw with the signal value.
	chgPin.mode(PinMode::PullNone);
	acpPin.mode(PinMode::PullNone);
	progPin.mode(PinMode::PullNone);
}

void ChemIDMeasurer::activateCharger()
{
	shdnPin.write(0);
}

void ChemIDMeasurer::deactivateCharger()
{
	shdnPin.write(1);
}

void ChemIDMeasurer::setState(ChemIDMeasurer::State newState)
{
	state = newState;
	stateTimer.reset();
}

void ChemIDMeasurer::runMeasurement()
{
	totalTimer.start();
	stateTimer.start();

	while(state != State::DONE)
	{
		// read data
		uint16_t voltage_mV = soc.getVoltage();
		int32_t current_mA = soc.getCurrent();
		double temperatureC = soc.getTemperature();
		char const * comment = "";

		// update based on state
		switch (state)
		{
			case State::INIT:
				// Print header
				printf("Elapsed Time (s), Voltage (mV), Current (mA), Temperature (deg C), SoC (%%), Comments\n");
				setState(State::CHARGE);
				activateCharger();
				comment = "Activating charger and entering CHARGE";
				break;

			case State::CHARGE:
				// threshold current = C/10
				if(current_mA < DESIGNCAP/10)
				{
					deactivateCharger();
					setState(State::RELAX_CHARGED);
					comment = "Deactivating charger and entering RELAX_CHARGED";
				}
				break;

			case State::RELAX_CHARGED:
				if(stateTimer.elapsed_time() > 2h)
				{
					setState(State::DISCHARGE);
					comment = "Done relaxing and entering discharge -- please disconnect charger and connect C/10 load now.";
				}
				break;

			case State::DISCHARGE:
				// Change states once we hit the termination voltage
				if(voltage_mV < ZEROCHARGEVOLT * CELLCOUNT)
				{
					setState(State::RELAX_DISCHARGED);
					comment = "Done discharging -- please remove C/10 load now.";
				}
				// BQ34Z100 reports positive current always, but TI's tool expects discharging to
				// be negative current.
				current_mA *= -1;
				break;

			case State::RELAX_DISCHARGED:
				if(stateTimer.elapsed_time() > 5h)
				{
					setState(State::DONE);
					comment = "Done!";
				}
				break;

			default:
				// will never be hit but here to silence warning
				break;
		}


		// print data column
		printf("%" PRIi64 ", %" PRIi16 ", %" PRIi32 ", %f, %" PRIu8 ", %s\n",
			std::chrono::duration_cast<std::chrono::seconds>(totalTimer.elapsed_time()).count(),
			voltage_mV, current_mA, temperatureC, soc.getSOC(), comment);

		// wait, update freq is every 5 seconds
		ThisThread::sleep_for(5s);
	}
}

ChemIDMeasurer measurer;

int main()
{
	measurer.runMeasurement();
	return 0;
}