/**
 * @file  ble_opcodes.c
 * @brief Bluetooth custom opcode characteristic handling via Nordic's UART GATT
 * service (NUS)
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/services/nus.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <ble/ble_opcodes.h>

/**
 * @ingroup ble_opcodes
 * @{
 */

/* ----------------------------------------------------------------------------
 * Local Definitions
 * --------------------------------------------------------------------------*/

LOG_MODULE_REGISTER(ble_opcodes, CONFIG_BLE_OPCODES_LOG_LEVEL);

/* ----------------------------------------------------------------------------
 * Local typedefs, structs, and enums
 * --------------------------------------------------------------------------*/

/// @brief OpCode descriptor
struct ble_opcodes_op_desc
{
  uint8_t opcode;
  uint32_t tx_length;
  int (*handler)(struct bt_conn *conn, uint8_t *data);
};

/* ----------------------------------------------------------------------------
 * Static Function Prototypes
 * --------------------------------------------------------------------------*/

/**
 * @brief GATT Service notification subscription changed.
 *
 * @param enabled Flag that is true if notifications were enabled, false
 *                if they were disabled.
 * @param ctx     User context provided in the callback structure.
 */
static void ble_opcodes_notifications_enabled(bool enabled, void *ctx);

/**
 * @brief GATT Service received data for the OpCode service.
 *
 * @param conn Peer Connection object.
 * @param data Pointer to buffer with data received.
 * @param len  Size in bytes of data received.
 * @param ctx  User context provided in the callback structure.
 */
static void ble_opcodes_received(struct bt_conn *conn,
                                 const void *data,
                                 uint16_t len,
                                 void *ctx);

/**
 * @brief OpCode handler for the PING opcode
 *
 * @param conn Peer connection object
 * @param data Verified length buffer with PING payload contained
 *
 * @return 0 if successful, negative error code otherwise
 */
static int ble_opcodes_op_ping_handler(struct bt_conn *conn, uint8_t *data);

/**
 * @brief OpCode handler for the COUNTER opcode
 *
 * @param conn Peer connection object
 * @param data Verified length buffer with COUNTER payload contained
 *
 * @return 0 if successful, negative error code otherwise
 */
static int ble_opcodes_op_counter_handler(struct bt_conn *conn, uint8_t *data);

/**
 * @brief OpCode handler for the DELAY opcode
 *
 * @param conn Peer connection object
 * @param data Verified length buffer with DELAY payload contained
 *
 * @return 0 if successful, negative error code otherwise
 */
static int ble_opcodes_op_delay_handler(struct bt_conn *conn, uint8_t *data);

/* ----------------------------------------------------------------------------
 * Local Variables
 * --------------------------------------------------------------------------*/

/// NUS callback listener
static struct bt_nus_cb ble_opcodes_listener = {
  .notif_enabled = ble_opcodes_notifications_enabled,
  .received = ble_opcodes_received,
};

/// @brief OpCode Descriptor table outlining the different OpCodes and
/// descriptors
static const struct ble_opcodes_op_desc ble_opcodes_op_table[] = {
  {
    .opcode = BLE_OPCODES_OP_PING,
    .tx_length = BLE_OPCODES_PING_TX_LEN,
    .handler = ble_opcodes_op_ping_handler,
  },
  {
    .opcode = BLE_OPCODES_OP_COUNTER,
    .tx_length = BLE_OPCODES_COUNTER_TX_LEN,
    .handler = ble_opcodes_op_counter_handler,
  },
  {
    .opcode = BLE_OPCODES_OP_DELAY,
    .tx_length = BLE_OPCODES_DELAY_TX_LEN,
    .handler = ble_opcodes_op_delay_handler,
  },
};

/* ----------------------------------------------------------------------------
 * Function Definitions
 * --------------------------------------------------------------------------*/

int ble_opcodes_register_listener(void)
{
  int err;

  err = bt_nus_cb_register(&ble_opcodes_listener, NULL);
  if (err)
  {
    LOG_ERR("Failed to register OpCodes BLE Listener: %d\n", err);
    return err;
  }

  return 0;
}

static void ble_opcodes_notifications_enabled(bool enabled, void *ctx)
{
  ARG_UNUSED(ctx);

  LOG_INF("Notifications %s", (enabled ? "enabled" : "disabled"));
}

static void ble_opcodes_received(struct bt_conn *conn,
                                 const void *data,
                                 uint16_t len,
                                 void *ctx)
{
  ARG_UNUSED(conn);
  ARG_UNUSED(ctx);

  uint8_t payload[CONFIG_BT_L2CAP_TX_MTU] = { 0 };

  memcpy(payload, data, MIN(sizeof(payload) - 1, len));

  LOG_HEXDUMP_INF(data, len, "Received payload");

  for (int table_idx = 0;
       table_idx < (sizeof(ble_opcodes_op_table) / sizeof(ble_opcodes_op_table[0]));
       table_idx++)
  {
    const struct ble_opcodes_op_desc *desc = &ble_opcodes_op_table[table_idx];

    if ((payload[BLE_OPCODES_OP_POS] == desc->opcode) && (len == desc->tx_length))
    {
      const int ret = desc->handler(conn, payload);
      if (ret)
      {
        LOG_ERR("Error %d from OpCode 0x%02X", ret, payload[BLE_OPCODES_OP_POS]);
      }
      return;
    }
  }

  LOG_ERR("Invalid OpCode received 0x%02X", payload[BLE_OPCODES_OP_POS]);
}

static int ble_opcodes_op_ping_handler(struct bt_conn *conn, uint8_t *data)
{
  LOG_INF("%s()", __func__);

  return bt_nus_send(NULL, data, BLE_OPCODES_PING_RX_LEN);
}

static int ble_opcodes_op_counter_handler(struct bt_conn *conn, uint8_t *data)
{
  ARG_UNUSED(data);

  LOG_INF("%s()", __func__);

  static union ble_opcodes_counter_rx_data op_data =
    (union ble_opcodes_counter_rx_data) { .fields = {
                                            .opcode = BLE_OPCODES_OP_COUNTER,
                                            .count = 0,
                                          } };

  // Increment the count
  op_data.fields.count++;

  return bt_nus_send(NULL, op_data.raw, BLE_OPCODES_COUNTER_RX_LEN);
}

static int ble_opcodes_op_delay_handler(struct bt_conn *conn, uint8_t *data)
{
  LOG_INF("%s()", __func__);

  k_msleep(CONFIG_BLE_OPCODES_OP_DELAY_TIME_MS);
  return bt_nus_send(NULL, data, BLE_OPCODES_DELAY_RX_LEN);
}

/** @} */  // ble_opcodes
