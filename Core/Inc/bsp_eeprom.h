#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "i2c.h"

/**
 * @brief EEPROM page size
 */
typedef enum
{
    EEP_PAGE_8B   = 8,
    EEP_PAGE_16B  = 16,
    EEP_PAGE_32B  = 32,
    EEP_PAGE_64B  = 64,
    EEP_PAGE_128B = 128,
    EEP_PAGE_MAX
} eeprom_page_size_t;

/**
 * @brief EEPROM supported models enumeration
 */
typedef enum
{
    EEP_MODEL_AT24C01 = 0,
    EEP_MODEL_AT24C02,
    EEP_MODEL_AT24C04,
    EEP_MODEL_AT24C08,
    EEP_MODEL_AT24C16,
    EEP_MODEL_AT24C32,
    EEP_MODEL_AT24C64,
    EEP_MODEL_AT24C128,
    EEP_MODEL_AT24C256,
    EEP_MODEL_AT24C512,
    EEP_MODEL_MAX
} eeprom_model_t;

/**
 * @brief EEPROM configuration structure
 */
typedef struct {
    uint8_t             addr_pin_mask;    // mask for the variable bits in the I2C device address based on the model's addressing scheme
    uint8_t             addr_pin_shift;
    uint8_t             memaddr_size;     // internal register address size (e.g., I2C_MEMADD_SIZE_8BIT (higher bits are encoded in the I2C device address))
    eeprom_page_size_t  page_size;
    uint32_t            total_size;       // total capacity in bytes (e.g., 1024 for AT24C08)
} eeprom_config_t;

/**
 * @brief EEPROM device structure that binds an I2C handle and a specific EEPROM configuration
 */
typedef struct {
    I2C_HandleTypeDef       *hi2cX;
    eeprom_model_t          model;
    const eeprom_config_t   *config;
    uint8_t                 device_base_addr; // i2c device base address (e.g., 0xA0 for AT24Cxx)
} eeprom_dev_t;

#define DEFAULT_I2C (&hi2c1)
#define EEP_WRITE_CYCLE_MS   5

/* --------------- exported API ------------------ */
extern uint8_t     eeprom_get_max_pins(eeprom_model_t model);
extern i2c_state_t eeprom_init_dev(eeprom_dev_t *dev, I2C_HandleTypeDef *hi2cX, eeprom_model_t selected_model, uint8_t pin_index);
extern i2c_state_t eeprom_write(eeprom_dev_t *dev, uint32_t addr, uint8_t *p_data, uint32_t len);
extern i2c_state_t eeprom_read(eeprom_dev_t *dev, uint32_t addr, uint8_t *p_data, uint32_t len);

#endif /* __EEPROM_H__ */