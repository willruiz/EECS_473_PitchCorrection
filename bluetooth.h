#ifndef BLUETOOTH_H
#define BLUETOOTH_H

//BLE modes: a given BLE chip is either a central device or a peripheral device
typedef uint8_t BLE_mode;
BLE_mode MODE_CENTRAL = 0;
BLE_mode MODE_PERIPHERAL = 1;

//Haptic types: our current implementation uses two peripheral types- one for above pitch and one for below
typedef uint8_t Haptic_side;
Haptic_side BLE_HAPTIC_ABOVE;
Haptic_side BLE_HAPTIC_BELOW;

/**
 * Initializes BLE stuff:
 *  -   Hardware settings
 *  -   Service and characteristics declaration
 *  -   Necessary memory allocation
 * TODO potentially need to add more parameters for additional configuration
 * @param mode MODE_CENTRAL or MODE_PERIPHERAL depending on the role of this device.
 * @param name The name of the device. Ignored if mode == MODE_CENTRAL.
 * @returns 0 on succeed, nonzero otherwise.
*/
uint8_t BLE_init(BLE_mode mode, char* name);

/**
 * Releases all memory and data associated with BLE.
 * @returns 0 on succeed, nonzero otherwise.
*/
uint8_t BLE_release();

/**
 * Adds a peripheral to pair with
*/
uint8_t BLE_add_peripheral(uint8_t *addr);

/**
 * Updates the haptic feedback characteristic(s).
 * This will typically be used by the centrals.
 * TODO we may need to change the type of value
 * @param side BLE_HAPTIC_ABOVE or BLE_HAPTIC_BELOW depending on the peripheral to update
 * @param value the value to update the side with
 * @returns 0 on succeed, nonzero otherwise.
*/
uint8_t BLE_write_haptic(Haptic_side side, uint16_t value);

/**
 * Reads the value of the haptic feedback characteristic(s).
 * This will typically be used by the peripherals.
 * TODO we may need to change the type of result
 * @param side BLE_HAPTIC_ABOVE or BLE_HAPTIC_BELOW depending on the peripheral to read from
 * @param result a pointer to the location to store the result in
 * @returns 0 on succeed, nonzero otherwise.
*/
uint8_t BLE_read_haptic(Haptic_side side, uint16_t *result);

#endif