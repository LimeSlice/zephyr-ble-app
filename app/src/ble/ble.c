/**
 * @file  ble.c
 * @brief Device's Bluetooth initialization, setup and handling
 */

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/printk.h>
#include <zephyr/types.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/settings/settings.h>

#include <ble/ble.h>
#include <ble/ble_opcodes.h>

/**
 * @ingroup ble_gen
 * @{
 */

/// BLE advertising data
static const struct bt_data advertising_data[] = {
  BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
  BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_DIS_VAL)),
};

/// BLE scan response data
static const struct bt_data scan_response_data[] = {
  BT_DATA(
    BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

/**
 * @brief A new connection has been established.
 *
 * This callback notifies the application of a new connection.
 * In case the err parameter is non-zero it means that the
 * connection establishment failed.
 *
 * @note If the connection was established from an advertising set then the
 * advertising set cannot be restarted directly from this callback. Instead use
 * the connected callback of the advertising set.
 *
 * @param conn New connection object.
 * @param err  HCI error. Zero for success, non-zero otherwise.
 *
 * @p err can mean either of the following:
 * - @ref BT_HCI_ERR_UNKNOWN_CONN_ID Creating the connection started by
 *   @ref bt_conn_le_create was canceled either by the user through
 *   @ref bt_conn_disconnect or by the timeout in the host through
 *   @ref bt_conn_le_create_param timeout parameter, which defaults to
 *   @kconfig{CONFIG_BT_CREATE_CONN_TIMEOUT} seconds.
 * - @p BT_HCI_ERR_ADV_TIMEOUT High duty cycle directed connectable
 *   advertiser started by @ref bt_le_adv_start failed to be connected
 *   within the timeout.
 */
static void ble_connected(struct bt_conn *conn, uint8_t err);

/**
 * @brief A connection has been disconnected.
 *
 * This callback notifies the application that a connection
 * has been disconnected.
 *
 * When this callback is called the stack still has one reference to
 * the connection object. If the application in this callback tries to
 * start either a connectable advertiser or create a new connection
 * this might fail because there are no free connection objects
 * available.
 *
 * To avoid this issue it is recommended to either start connectable
 * advertise or create a new connection using @ref k_work_submit or
 * increase @kconfig{CONFIG_BT_MAX_CONN}.
 *
 * @param conn   Connection object.
 * @param reason BT_HCI_ERR_* reason for the disconnection.
 */
static void ble_disconnected(struct bt_conn *conn, uint8_t reason);

/**
 * @brief Load BLE runtime settings
 *
 * @see /zephyr/subsys/bluetooth/services/dis.c for DIS runtime settings
 */
static void ble_load_runtime_settings(void);

/// Register callbacks for connection events.
BT_CONN_CB_DEFINE(conn_callbacks) = {
  .connected = ble_connected,
  .disconnected = ble_disconnected,
};

int ble_init(void)
{
  int err;

  // Register services prior to enabling BLE
  err = ble_opcodes_register_listener();
  if (err)
  {
    return err;
  }

  // Enable BLE
  err = bt_enable(NULL);
  if (err)
  {
    printk("Bluetooth init failed (err %d)\n", err);
    return err;
  }

  // Load Settings (if enabled)
  if (IS_ENABLED(CONFIG_BT_SETTINGS))
  {
    settings_load();
  }

  // Load runtime settings (if enabled)
  if (IS_ENABLED(CONFIG_SETTINGS_RUNTIME))
  {
    ble_load_runtime_settings();
  }

  printk("Bluetooth initialized\n");

  err = bt_le_adv_start(BT_LE_ADV_CONN_ONE_TIME,
                        advertising_data,
                        ARRAY_SIZE(advertising_data),
                        scan_response_data,
                        ARRAY_SIZE(scan_response_data));
  if (err)
  {
    printk("Advertising failed to start (err %d)\n", err);
    return err;
  }

  printk("Advertising successfully started\n");

  return 0;
}

static void ble_connected(struct bt_conn *conn, uint8_t err)
{
  ARG_UNUSED(conn);
  if (err)
  {
    printk("Connection failed, err 0x%02x %s\n", err, bt_hci_err_to_str(err));
  }
  else
  {
    printk("Connected\n");
  }
}

static void ble_disconnected(struct bt_conn *conn, uint8_t reason)
{
  ARG_UNUSED(conn);
  printk("Disconnected, reason 0x%02x %s\n", reason, bt_hci_err_to_str(reason));
}

static void ble_load_runtime_settings(void)
{
#if defined(CONFIG_BT_DIS_SETTINGS)
  settings_runtime_set("bt/dis/model", CONFIG_BT_DIS_MODEL, sizeof(CONFIG_BT_DIS_MODEL));
  settings_runtime_set("bt/dis/manuf", CONFIG_BT_DIS_MANUF, sizeof(CONFIG_BT_DIS_MANUF));
#if defined(CONFIG_BT_DIS_SERIAL_NUMBER)
  settings_runtime_set("bt/dis/serial",
                       CONFIG_BT_DIS_SERIAL_NUMBER_STR,
                       sizeof(CONFIG_BT_DIS_SERIAL_NUMBER_STR));
#endif
#if defined(CONFIG_BT_DIS_SW_REV)
  settings_runtime_set("bt/dis/sw",
                       CONFIG_BT_DIS_SW_REV_STR,
                       sizeof(CONFIG_BT_DIS_SW_REV_STR));
#endif
#if defined(CONFIG_BT_DIS_FW_REV)
  settings_runtime_set("bt/dis/fw",
                       CONFIG_BT_DIS_FW_REV_STR,
                       sizeof(CONFIG_BT_DIS_FW_REV_STR));
#endif
#if defined(CONFIG_BT_DIS_HW_REV)
  settings_runtime_set("bt/dis/hw",
                       CONFIG_BT_DIS_HW_REV_STR,
                       sizeof(CONFIG_BT_DIS_HW_REV_STR));
#endif
#endif
}

/** @} */  // ble_gen
