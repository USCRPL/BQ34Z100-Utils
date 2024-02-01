/*
    USC RPL HAMSTER v2.3 BQ34Z100 Test Suite
    Contributors: Arpad Kovesdy
*/
#include "SOCTestSuite.h"

#include <cinttypes>

I2C i2c(I2C_SDA, I2C_SCL);
BQ34Z100 soc(i2c, 100000);

DigitalIn chgPin(CHARGE_STATUS_PIN);
DigitalOut shdnPin(ACTIVATE_CHARGER_PIN);

// helper function to print a bitfield prettily.
void printBitfield(uint16_t value, const char* name, const char** bitDescriptions)
{
	printf("\n");
	printf("%s: 0x%" PRIx16 " (0b", name, value);
	for (int i = 15; i >= 0; i--)
	{
		printf("%i", (value >> i) & 1);
	}
	printf(")\n");

	const size_t maxBit = sizeof(value) * 8 - 1;
	for (int i = maxBit; i>=0; i--)
	{
		// Description array is in reverse order numerically
		char const * description = bitDescriptions[maxBit - i];

		uint8_t bitValue = (value >> i) & 1;
		if(description != nullptr)
		{
			printf("- %s: %" PRIu8 "\n", description, bitValue);
		}
	}
}

void SOCTestSuite::outputStatus()
{
    uint16_t status_code = soc.getStatus();

    // Descriptions for each bit of the status bytes
    const char* statusBitDescs[] = {
    		nullptr,
    		"Full Access Sealed (FAS)",
    		"Sealed (SS)",
    		"Calibration Enabled (CALEN)",
    		"Coulomb Counter Calibrating (CCA)",
    		"Board Calibration Active (BCA)",
    		"Valid Data Flash Checksum (CSV)",
    		nullptr,
    		nullptr,
    		nullptr,
    		"Full Sleep Mode (FULLSLEEP)",
    		"Sleep Mode (SLEEP)",
    		"Impedance Track using Constant Power (LDMD)",
    		"Ra Updates Disabled (RUP_DIS)",
    		"Voltage OK for Qmax Updates (VOK)",
    		"Qmax Updates Enabled (QEN)"
    };

    printBitfield(status_code, "Control Status", statusBitDescs);

	// Descriptions for each bit of the flags bytes
	const char* flagsBitDescs[] = {
			"Overtemperature in Charge (OTC)",
			"Overtemperature in Discharge (OTD)",
			"High Battery Voltage (BATHI)",
			"Low Battery Voltage (BATLOW)",
			"Charge Inhibited (CHG_INH)",
			"Charging Not Allowed (XCHG)",
			"Full Charge (FC)",
			"Charge Allowed (CHG)",
			"Open Circuit Voltage Measurement Performed (OCVTAKEN)",
			nullptr,
			nullptr,
			"Update Cycle Needed (CF)",
			nullptr,
			"SoC Threshold 1 Reached (SOC1)",
			"SoC Threshold Final Reached (SOCF)",
			"Discharge Detected (DSG)"
	};
	const char* flagsBBitDescs[] = {
			"State of Health Calc Active (SOH)",
			"LiFePO4 Relax Enabled (LIFE)",
			"Waiting for Depth of Discharge Measurement (FIRSTDOD)",
			nullptr,
			nullptr,
			"Depth of Discharge at End of Charge Updated (DODEOC)",
			"Remaining Capacity Changed (DTRC)",
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr
	};

	std::pair<uint16_t, uint16_t> flags = soc.getFlags();
	printBitfield(flags.first, "Flags", flagsBitDescs);
	printBitfield(flags.second, "FlagsB", flagsBBitDescs);


    uint8_t updateStatus = soc.getUpdateStatus();
    printf("Update status: 0x%" PRIx8 "\n", updateStatus);
}

void SOCTestSuite::sensorReset()
{
    printf("Resetting BQ34Z100 Sensor.\r\n");
    soc.reset();

    uint16_t deviceType = soc.readDeviceType();
    if(deviceType == 0x100)
    {
        printf("BQ34Z100 detected\r\n");
    }
    else
    {
        printf("Error communicating with BQ34Z100.  Expected DEVICE_TYPE = 0x100, got 0x%" PRIx16 "\n", deviceType);
        return;
    }

    printf("Chip reads as FW_VERSION 0x%" PRIx16 ", HW version 0x%" PRIx16 "\r\n", soc.readFWVersion(), soc.readHWVersion());
}

void SOCTestSuite::displayData()
{
    ThisThread::sleep_for(10ms); //Let the device catch up
    printf("SOC: %d%%\r\n", soc.getSOC());
    printf("Voltage: %d mV\r\n", soc.getVoltage());
    printf("Current: %d mA\r\n", soc.getCurrent());
    printf("Remaining: %d mAh\r\n", soc.getRemaining());
    printf("Temperature: %.1f C\r\n", soc.getTemperature());
    printf("Max Error: %d%%\r\n", soc.getError());
    printf("Serial No: %d\r\n", soc.getSerial());
    printf("CHEM ID: %" PRIx16 "\r\n", soc.getChemID());
}

void SOCTestSuite::testHamsterConnection()
{
    printf("Testing Electrical Connection\r\n");
    int status = soc.getStatus();
    printf("Status: %d\r\n\r\n", status);
}

void SOCTestSuite::startCal()
{
    printf("Starting calibration mode\r\n");
    soc.enableCal();
    soc.enterCal();
}

void SOCTestSuite::stopCal()
{
    printf("Stopping calibration mode\r\n");
    soc.exitCal();
}

void SOCTestSuite::startIt()
{
    printf("Enabling Impedance Tracking\r\n");
    soc.ITEnable();
}

//Outputs an integer of the length provided starting from the given index in flashBytes
//Provide pointer to first element (array pointer to flashbytes)
void SOCTestSuite::outputFlashInt(uint8_t* flash, int index, int len)
{
    if (index > 31) index = index % 32;
    unsigned int result = 0;
    for (int i = 0; i<len; i++)
    {
        //result = result | ((uint32_t)flash[index+i] << 8*len);
        result |= ((uint32_t)flash[index+i] << 8*(len-i-1));
    }
    printf("%d", result);
}

void SOCTestSuite::writeSettings()
{
	if(soc.getVoltage() <= FLASH_UPDATE_OK_VOLT * CELLCOUNT)
	{
		printf("WARNING: Measured voltage is below FLASH_UPDATE_OK_VOLT, flash memory writes may not go through.  However this is expected if voltage has not been calibrated yet.");
	}

    soc.unseal();
    printf("Starting overwrite of sensor settings\r\n");
    uint8_t* flashStore = soc.getFlashBytes(); //Get address of array for later

    //Page 48
    soc.changePage(48, 0); //calls ChangePage from BQ34Z100 editing page 48 from datasheet
    soc.readFlash();

    //declares old subclass properties as per BQ34Z100 function commands
    printf("Old design capacity:");
    outputFlashInt(flashStore, 11, 2);
    printf("\r\n");
    printf("Old design energy:");
    outputFlashInt(flashStore, 13, 2);
    printf("\r\n");

    //replaces the old subclass properties with new ones as per BQ34Z100 function commands
    soc.changePage48();
    printf("New design capacity:");
    outputFlashInt(flashStore, 11, 2);
    printf("\r\n");
    printf("New design energy:");
    outputFlashInt(flashStore, 13, 2);
    printf("\r\n");

    //Page 64
    soc.changePage(64, 0); //calls ChangePage from BQ34Z100 editing page 48 from datasheet
    soc.readFlash();

    //declares old subclass properties as per BQ34Z100 function commands
    printf("Old Pack Configuration:");
    outputFlashInt(flashStore, 0, 2);
    printf("\r\n");
    printf("Old LED Config:");
    outputFlashInt(flashStore, 4, 1);
    printf("\r\n");
    printf("Old Cell Count:");
    outputFlashInt(flashStore, 7, 1);
    printf("\r\n");

    //replaces the old subclass properties with new ones as per BQ34Z100 function commands
    soc.changePage64();
    printf("New Pack Configuration:");
    outputFlashInt(flashStore, 0, 2);
    printf("\r\n");
    printf("New LED Config:");
    outputFlashInt(flashStore, 4, 1);
    printf("\r\n");
    printf("New Cell Count:");
    outputFlashInt(flashStore, 7, 1);
    printf("\r\n");

    //Page 80

    soc.changePage(80, 0); //calls ChangePage from BQ34Z100 editing page 48 from datasheet
    soc.readFlash();
    //declares old subclass properties as per BQ34Z100 function commands
    printf("Old Load Select:");
    outputFlashInt(flashStore, 0, 1);
    printf("\r\n");
    printf("Old Load Mode:");
    outputFlashInt(flashStore, 1, 1);
    printf("\r\n");

    soc.changePage(80, 53);
    soc.readFlash();
    printf("Old Cell Terminate Voltage:");
    outputFlashInt(flashStore, 53, 2);
    printf("\r\n");

    //replaces the old subclass properties with new ones as per BQ34Z100 function commands
    soc.changePage80();
    soc.changePage(80, 0);
    soc.readFlash();
    printf("New Load Select:");
    outputFlashInt(flashStore, 0, 1);
    printf("\r\n");
    printf("New Load Mode:");
    outputFlashInt(flashStore, 1, 1);
    printf("\r\n");
    printf("Res Current:");
    outputFlashInt(flashStore, 10, 2);
    printf("\r\n");
    soc.changePage(80, 53);
    soc.readFlash();
    printf("New Cell Terminate Voltage:");
    outputFlashInt(flashStore, 53, 2);
    printf("\r\n");

    //Page 82
    soc.changePage(82, 0); //calls ChangePage from BQ34Z100 editing page 48 from datasheet
    soc.readFlash();

    //declares old subclass properties as per BQ34Z100 function commands
    printf("Old QMax0:");
    outputFlashInt(flashStore, 0, 2);
    printf("\r\n");

    //replaces the old subclass properties with new ones as per BQ34Z100 function commands
    soc.changePage82();
    printf("New QMax0:");
    outputFlashInt(flashStore, 0, 2);
    printf("\r\n");

    //Print the updatestatus
    //0x02 = Qmax and Ra data are learned, but Impedance Track is not enabled.
    //This should be the standard setting for a Golden Image File
    //0x04 = Impedance Track is enabled but Qmax and Ra data are not yet learned.
    //0x05 = Impedance Track is enabled and only Qmax has been updated during a learning cycle.
    //0x06 = Impedance Track is enabled. Qmax and Ra data are learned after a successful learning
    //cycle. This should be the operation setting for end equipment.
    printf("UPDATE STATUS:");
    outputFlashInt(flashStore, 4, 1);
    printf("\r\n");
}

void SOCTestSuite::calibrateVoltage ()
{

    printf("Enter voltage across the pack: ");
    float batVoltage=-1;
    scanf("%f", &batVoltage);
    printf("Hand measured voltage: %f\r\n\n", batVoltage);
    //printf("Enter ACTUAL battery voltage in volts\r\n");
    //scanf("%f", &batVoltage);
    uint16_t batVoltage_mv = (uint16_t)(batVoltage*1000.0f);
    printf("\r\nCurrent Monitor Bus Voltage Battery Voltage (V): %f\r\n", batVoltage);

    printf("New Flash Voltage: %d\r\n", soc.calibrateVoltage(batVoltage_mv));
}

//Input current in A
void SOCTestSuite::calibrateCurrent()
{
	soc.setSenseResistor();

	soc.reset();
	ThisThread::sleep_for(200ms);

	 printf("Enter current through the sense resistor in A: ");
	 float current=-1;
	 scanf("%f", &current);
	 printf("Hand measured current%f:\r\n\n", current);

	 printf("Calibrating current shunt\r\n\r\n");
	 int16_t current_int = (int16_t)(current*1000.0f);
	 soc.calibrateShunt(current_int);
}

void SOCTestSuite::discharge() {
    printf("Discharging Battery, have a small load attached \r\n");
    int voltage = soc.getVoltage();
    //2750 is the termination voltage
    int seconds = 0;
    int current = soc.getCurrent();
    printf("Time,\tVoltage,\tCurrent\r\n");
    while (voltage > 2750) {
        printf("%d,\t%d,\t%d\r\n", seconds, voltage, current);
        voltage = soc.getVoltage();
        current = soc.getCurrent();
        ThisThread::sleep_for(10s);
    }

    printf("\r\nDischarge Complete!\r\n");

}

void SOCTestSuite::relaxEmpty() {
    printf("Relaxing the battery after a discharge (5 hours) \r\n");
    for (int i = 0; i < 10; i++) {
        ThisThread::sleep_for(1800s);
        printf("#");
    }

    printf("\r\n\nDischarge relax complete!\r\n");
}

void SOCTestSuite::charge() {

    //Release from shdn
    shdnPin.write(CHARGER_PIN_ACTIVATE);
    ThisThread::sleep_for(10s);
    if (chgPin.read() == CHARGE_STATUS_CHARGING) {
        printf("Charging has started!\r\n");
    } else {
        printf("Charging did not start! Exiting.\r\n");
        return;
    }

    int voltage = soc.getVoltage();
    Timer chargeTimer;
    chargeTimer.start();
    int current = soc.getCurrent();
    printf("Time,\tVoltage,\tCurrent\r\n");

    //Could use the CHG_I_OUT pin to read charging current, but we can also
    //just measure it with the gauge
    while (chgPin.read() == CHARGE_STATUS_CHARGING) {
        printf("%.02f,\t%d,\t%d\r\n",
        		std::chrono::duration_cast<std::chrono::duration<float>>(chargeTimer.elapsed_time()).count(),
        		voltage, current);
        voltage = soc.getVoltage();
        current = soc.getCurrent();
        ThisThread::sleep_for(10s);
    }

    printf("\r\nCharge Complete!\r\n");
	shdnPin.write(CHARGER_PIN_DEACTIVATE);

}

void SOCTestSuite::readVoltageCurrent()
{
	printf("Voltage,\tCurrent\r\n");

	while (true) {
		int voltage = soc.getVoltage();
		int current = soc.getCurrent();
		printf("%d,\t%d\r\n", voltage, current);
		ThisThread::sleep_for(100ms);
	}
}

void SOCTestSuite::relaxFull() {
    printf("Relaxing the battery after a charge (2 hours) \r\n");
    for (int i = 0; i < 10; i++) {
        ThisThread::sleep_for(720s);
        printf("#");
    }

    printf("\r\n\nCharge relax complete!\r\n");
}

void SOCTestSuite::resetVoltageCalibration()
{
    soc.resetVoltageDivider();
    printf("\r\n\nVoltage divider calibration reset.\r\n");
}

void SOCTestSuite::testFloatConversion()
{
	// test data from https://e2e.ti.com/support/power-management/f/196/p/551252/2020286?tisearch=e2e-quicksearch&keymatch=xemics#2020286
	float valueFloat = .8335f;
	uint32_t valueXemics = 0x80555E9E;

	// try converting float to xemics
	uint32_t convertedValue = BQ34Z100::floatToXemics(valueFloat);
	printf("Converted value: 0x%" PRIx32 "\n", convertedValue);

	// try converting xemics to float
	float convertedFloat = BQ34Z100::xemicsToFloat(valueXemics);
	printf("Converted float: %f\n", convertedFloat);

	printf("Expected default CC Gain: 0x%" PRIx32 "\n", BQ34Z100::floatToXemics(0.4768));
	printf("Expected default CC Delta: 0x%" PRIx32 "\n", BQ34Z100::floatToXemics(567744.56));

}


int main()
{
    //declare the test harness
    SOCTestSuite harness;

    //Initially keep charger in shdn
    shdnPin.write(CHARGER_PIN_DEACTIVATE);
	chgPin.mode(PinMode::PullNone);

    while(1){
        int test=-1;
        printf("\r\n\nBattery State of Charge Sensor Test Suite:\r\n");

        //Menu for each test item
        printf("Select a test: \n\r");
        printf("1.  Reset Sensor (Restart)\r\n");
        printf("3.  Write Settings for BQ34Z100\r\n");
        printf("4.  Calibrate Voltage\r\n");
        printf("5.  Calibrate Current\r\n");
        printf("6.  Enable Calibration Mode\r\n");
        printf("7.  Disable Calibration Mode\r\n");
        printf("8.  Enable Impedance Tracking\r\n");
        printf("9.  Discharge Battery\r\n");
        printf("10.  Relax Empty Battery\r\n");
        printf("11.  Charge Battery\r\n");
        printf("12.  Relax Full Battery\r\n");
        printf("13.  Output Status\r\n");
        printf("14.  Display Data\r\n");
        printf("15.  Test Connection\r\n");
        printf("16.  Reset Voltage Divider Calibration\r\n");
	    printf("17.  Test Float Conversion\r\n");
	    printf("18.  Read Voltage and Current Forever\r\n");
	    printf("20.  Exit Test Suite\r\n");

        scanf("%d", &test);
        printf("Running test %d:\r\n\n", test);

        //SWITCH. ADD A CASE FOR EACH TEST.
        switch(test) {
            case 1:         harness.sensorReset();                           break;
            case 3:         harness.writeSettings();                         break;
            case 4:         harness.calibrateVoltage();                      break;
            case 5:         harness.calibrateCurrent();                      break;
            case 6:         harness.startCal();                              break;
            case 7:         harness.stopCal();                               break;
            case 8:         harness.startIt();                               break;
            case 9:         harness.discharge();                             break;
            case 10:        harness.relaxEmpty();                            break;
            case 11:        harness.charge();                                break;
            case 12:        harness.relaxFull();                             break;
            case 13:        harness.outputStatus();                          break;
            case 14:        harness.displayData();                           break;
            case 15:        harness.testHamsterConnection();                 break;
            case 16:        harness.resetVoltageCalibration();               break;
	        case 17:        harness.testFloatConversion();                   break;
	        case 18:        harness.readVoltageCurrent();                    break;
	        case 20:        printf("Exiting test suite.\r\n");               return 0;
            default:        printf("Invalid test number. Please run again.\r\n"); return 1;
        }

        printf("done.\r\n");
    }
    return 0;
}
