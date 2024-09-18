/**
 * @file  main.c
 * @brief Application main entry point
 */

#include <stdbool.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

#include <ble/ble.h>

int main(void)
{
  int err;

  err = ble_init();

  if (err)
  {
    return err;
  }

  return 0;
}
