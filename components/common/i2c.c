#include <driver/i2c.h>
#include <esp_log.h>

/* Constants ******************************************************************/

const uint32_t i2c_timeout_ticks = pdMS_TO_TICKS(1000);

/* Private Functions **********************************************************/

esp_err_t priv_i2c_init(uint8_t scl_io, uint8_t sda_io, uint32_t freq_hz, 
                        uint8_t i2c_bus, const char *tag)
{
  /* The I2C configuration structure */
  i2c_config_t conf = {
    .mode             = I2C_MODE_MASTER,    /* Set the I2C mode to master */
    .sda_io_num       = sda_io,             /* Set the GPIO number for SDA (data line) */
    .sda_pullup_en    = GPIO_PULLUP_ENABLE, /* Enable internal pull-up for SDA line */
    .scl_io_num       = scl_io,             /* Set the GPIO number for SCL (clock line) */
    .scl_pullup_en    = GPIO_PULLUP_ENABLE, /* Enable internal pull-up for SCL line */
    .master.clk_speed = freq_hz,            /* Set the I2C master clock frequency */
  };

  /* Configure the I2C bus with the settings specified in 'conf' */
  esp_err_t err = i2c_param_config(i2c_bus, &conf);
  if (err != ESP_OK) {
    ESP_LOGE(tag, "I2C param config failed: %s", esp_err_to_name(err));
    return err;  /* Return the error code if configuration fails */
  }

  /* Install the I2C driver for the master mode; no RX/TX buffers are required */
  return i2c_driver_install(i2c_bus, conf.mode, 0, 0, 0);
}

esp_err_t priv_i2c_write_byte(uint8_t data, uint8_t i2c_bus, 
                              uint8_t i2c_address, const char *tag)
{
  /* Create an I2C command link handle */
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  /* Start I2C communication */
  i2c_master_start(cmd);

  /* Send the I2C address with the write flag */
  i2c_master_write_byte(cmd, i2c_address << 1 | I2C_MASTER_WRITE, true);

  /* Write the data byte to the device */
  i2c_master_write_byte(cmd, data, true);

  /* Stop I2C communication */
  i2c_master_stop(cmd);

  /* Execute the I2C command */
  esp_err_t ret = i2c_master_cmd_begin(i2c_bus, cmd, i2c_timeout_ticks);

  /* Delete the command link after execution */
  i2c_cmd_link_delete(cmd);

  /* Check for errors in the I2C command */
  if (ret != ESP_OK) {
    ESP_LOGE(tag, "I2C write failed: %s", esp_err_to_name(ret));
  }

  return ret; /* Return the error status or ESP_OK */
}

esp_err_t priv_i2c_read_bytes(uint8_t *data, size_t len, uint8_t i2c_bus, 
                              uint8_t i2c_address, const char *tag)
{
  /* Create an I2C command link handle */
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  /* Start I2C communication */
  i2c_master_start(cmd);

  /* Send the I2C address with the read flag */
  i2c_master_write_byte(cmd, i2c_address << 1 | I2C_MASTER_READ, true);

  /* Read multiple bytes if length is greater than 1 */
  if (len > 1) {
    /* Read len-1 bytes with an ACK after each byte */
    i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
  }

  /* Read the last byte with a NACK to signal the end of the transmission */
  i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);

  /* Stop I2C communication */
  i2c_master_stop(cmd);

  /* Execute the I2C command */
  esp_err_t ret = i2c_master_cmd_begin(i2c_bus, cmd, i2c_timeout_ticks);

  /* Delete the command link after execution */
  i2c_cmd_link_delete(cmd);

  /* Check for errors in the I2C command */
  if (ret != ESP_OK) {
    ESP_LOGE(tag, "I2C read failed: %s", esp_err_to_name(ret));
  }

  return ret; /* Return the error status or ESP_OK */
}
