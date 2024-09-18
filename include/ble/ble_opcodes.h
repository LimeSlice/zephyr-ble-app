/**
 * @file  ble_opcodes.h
 * @brief Bluetooth custom opcode characteristic handling via Nordic's UART GATT
 * service (NUS).
 *
 * An OpCode can be transferred on over BLE using the following format:
 *
 * Index | Name
 * ------|--------
 * 0     | Opcode
 * 1:n   | Data
 */

#ifndef APP_BLE_OPCODES_H_
#define APP_BLE_OPCODES_H_

/**
 * @defgroup ble_opcodes OpCodes Service
 * @ingroup ble
 * @{
 * @brief Bluetooth custom opcode characteristic handling via Nordic's UART GATT
 * service (NUS).
 */

/* ----------------------------------------------------------------------------
 * Defintions
 * --------------------------------------------------------------------------*/

#define BLE_OPCODES_OP_POS                (0)
#define BLE_OPCODES_DATA_POS              (1)

/* ----------------------------------------------------------------------------
 * OpCode Defintions
 * --------------------------------------------------------------------------*/

#define BLE_OPCODES_OP_PING               (0x01)
#define BLE_OPCODES_OP_COUNTER            (0x02)
#define BLE_OPCODES_OP_DELAY              (0x03)
#define BLE_OPCODES_OP_TEMPERATURE        (0x04)
#define BLE_OPCODES_OP_TEMP_SENSOR_CONFIG (0x05)

// OpCode PING

#define BLE_OPCODES_PING_TX_LEN           (1)
#define BLE_OPCODES_PING_RX_LEN           (1)

// OpCode COUNTER

#define BLE_OPCODES_COUNTER_TX_LEN        (1)
#define BLE_OPCODES_COUNTER_RX_LEN        (5)

/// @brief Counter (RX) data union for filling fields and retrieving raw bytes
union ble_opcodes_counter_rx_data
{
  uint8_t raw[BLE_OPCODES_COUNTER_RX_LEN];  ///< Raw data bytes
  struct
  {
    uint8_t opcode;  ///< OpCode - always BLE_OPCODES_OP_COUNTER
    uint32_t count;  ///< Count value to respond with
  } fields;
};

// OpCode DELAY

#define BLE_OPCODES_DELAY_TX_LEN       (1)
#define BLE_OPCODES_DELAY_RX_LEN       (1)

// OpCode TEMPERATURE

#define BLE_OPCODES_TEMPERATURE_TX_LEN (1)
#define BLE_OPCODES_TEMPERATURE_RX_LEN (9)

/// @brief Temperature (RX) data union for filling fields and retrieving raw
/// bytes
union ble_opcodes_temperature_rx_data
{
  uint8_t raw[BLE_OPCODES_TEMPERATURE_RX_LEN];  ///< Raw data bytes
  struct
  {
    uint8_t opcode;           ///< OpCode - always BLE_OPCODES_OP_TEMPERATURE
    int32_t temp_whole_nb;    ///< Whole number for temperature in Celsius
    int32_t temp_decimal_nb;  ///< Decimal number for temperature in Celsius
  } fields;
};

// OpCode TEMP_SENSOR_CONFIG

#define BLE_OPCODES_TEMP_SENSOR_CONFIG_TX_LEN (28)
#define BLE_OPCODES_TEMP_SENSOR_CONFIG_RX_LEN (5)

/// @brief Temperature Sensor Configuration (TX) data union for filling fields
/// and retrieving raw bytes
union ble_opcodes_temp_sensor_config_tx_data
{
  uint8_t raw[BLE_OPCODES_TEMP_SENSOR_CONFIG_TX_LEN];  ///< Raw data bytes
  struct
  {
    uint8_t opcode;                 ///< OpCode - always BLE_OPCODES_OP_TEMP_SENSOR_CONFIG
    uint16_t config_reg_value;      ///< MCP9808 configuration register value
    uint8_t resolution_reg_value;   ///< MCP9808 resolution register value
    int32_t upper_temp_whole_nb;    ///< MCP9808 upper temp whole number
    int32_t upper_temp_decimal_nb;  ///< MCP9808 upper temp decimal number
    int32_t lower_temp_whole_nb;    ///< MCP9808 lower temp whole number
    int32_t lower_temp_decimal_nb;  ///< MCP9808 lower temp decimal number
    int32_t critical_temp_whole_nb;    ///< MCP9808 critical temp whole number
    int32_t critical_temp_decimal_nb;  ///< MCP9808 critical temp decimal number
  } fields;
};

/// @brief Temperature Sensor Configuration (RX) data union for filling fields
/// and retrieving raw bytes
union ble_opcodes_temp_sensor_config_rx_data
{
  uint8_t raw[BLE_OPCODES_TEMP_SENSOR_CONFIG_RX_LEN];  ///< Raw data bytes
  struct
  {
    uint8_t opcode;  ///< OpCode - always BLE_OPCODES_OP_TEMP_SENSOR_CONFIG

    /// 0 if successfully updated MCP9808, otherwise a negative error code will
    /// be returned.
    int return_code;
  } fields;
};

/* ----------------------------------------------------------------------------
 * Function Prototypes
 * --------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Register BLE OpCodes listener
 *
 * @note Must be done prior to initializing the BLE stack
 *
 * @return 0 on success, negative error code if failed
 *         bt_nus_cb_register
 */
int ble_opcodes_register_listener(void);

#ifdef __cplusplus
}  // extern "C"
#endif

/** @} */  // ble_opcodes

#endif /* APP_BLE_OPCODES_H_ */
