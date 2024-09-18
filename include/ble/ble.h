/**
 * @file  ble.h
 * @brief Device's Bluetooth initialization, setup and handling
 */

#ifndef APP_BLE_BLE_H_
#define APP_BLE_BLE_H_

/**
 * @defgroup ble_gen General BLE
 * @ingroup ble
 * @{
 * @brief General BLE functionality
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Initialize BLE stack and start advertising
 *
 * @return Initialization error. Zero for success, non-zero otherwise.
 */
int ble_init(void);

#ifdef __cplusplus
}  // extern "C"
#endif

/** @} */  // ble_gen

#endif /* APP_BLE_BLE_H_ */
