#ifndef BQ34Z100G1_UTILS_PINS_H
#define BQ34Z100G1_UTILS_PINS_H

// Change these pin names to match how the BQ34 is connected on your processor
#define BQ34_I2C_SDA PB_9
#define BQ34_I2C_SCL PB_8

// Pin which activates the charger.  Used for the Chem ID Measurer and some other tests
#define ACTIVATE_CHARGER_PIN PF_1

// Values to be written to the charge pin (e.g. this can be used to change it from active low to active high)
#define CHARGER_PIN_ACTIVATE 0
#define CHARGER_PIN_DEACTIVATE 1

// Pin which gets status from the charger.  This pin indicates when charging has started and when it is complete.
#define CHARGE_STATUS_PIN PF_2

#define CHARGE_STATUS_CHARGING 0 // Level present on CHARGE_STATUS_PIN when charging
#define CHARGE_STATUS_NOT_CHARGING 1 // Level present on CHARGE_STATUS_PIN when not charging

#endif //BQ34Z100G1_UTILS_PINS_H
