#include "bsp_eeprom.h"

/**
 * @brief EEPROM configuration table indexed by eeprom_model_t, this table is
 *        used in eeprom_init_dev() to initialize the eeprom_dev_t instance.
 */
static const eeprom_config_t EEP_CONFIG_TABLE[EEP_MODEL_MAX] = {
  [EEP_MODEL_AT24C01] = {
    .addr_pin_mask = 0x0E,
    .addr_pin_shift = 1,
    .page_size = EEP_PAGE_8B, 
    .memaddr_size = I2C_MEMADD_SIZE_8BIT, 
    .total_size = 128
  },
  [EEP_MODEL_AT24C02] = {
    .addr_pin_mask = 0x0E,
    .addr_pin_shift = 1,
    .page_size = EEP_PAGE_8B, 
    .memaddr_size = I2C_MEMADD_SIZE_8BIT, 
    .total_size = 256
  },
  [EEP_MODEL_AT24C04] = {
    .addr_pin_mask = 0x0C,
    .addr_pin_shift = 2,
    .page_size = EEP_PAGE_16B, 
    .memaddr_size = I2C_MEMADD_SIZE_8BIT, 
    .total_size = 512
  },
  [EEP_MODEL_AT24C08] = {
    .addr_pin_mask = 0x08,
    .addr_pin_shift = 3,
    .page_size = EEP_PAGE_16B, 
    .memaddr_size = I2C_MEMADD_SIZE_8BIT, 
    .total_size = 1024
  },
  [EEP_MODEL_AT24C16] = {
    .addr_pin_mask = 0x00,
    .addr_pin_shift = 0,
    .page_size = EEP_PAGE_16B, 
    .memaddr_size = I2C_MEMADD_SIZE_8BIT, 
    .total_size = 2048
  },
  [EEP_MODEL_AT24C32] = {
    .addr_pin_mask = 0x0E,
    .addr_pin_shift = 1,
    .page_size = EEP_PAGE_32B, 
    .memaddr_size = I2C_MEMADD_SIZE_16BIT, 
    .total_size = 4096
  },
  [EEP_MODEL_AT24C64] = {
    .addr_pin_mask = 0x0E,
    .addr_pin_shift = 1,
    .page_size = EEP_PAGE_32B, 
    .memaddr_size = I2C_MEMADD_SIZE_16BIT, 
    .total_size = 8192
  },
  [EEP_MODEL_AT24C128] = {
    .addr_pin_mask = 0x0E,
    .addr_pin_shift = 1,
    .page_size = EEP_PAGE_64B, 
    .memaddr_size = I2C_MEMADD_SIZE_16BIT, 
    .total_size = 16384
  },
  [EEP_MODEL_AT24C256] = {
    .addr_pin_mask = 0x0E,
    .addr_pin_shift = 1,
    .page_size = EEP_PAGE_64B, 
    .memaddr_size = I2C_MEMADD_SIZE_16BIT, 
    .total_size = 32768
  },
  [EEP_MODEL_AT24C512] = {
    .addr_pin_mask = 0x0E,
    .addr_pin_shift = 1,
    .page_size = EEP_PAGE_128B, 
    .memaddr_size = I2C_MEMADD_SIZE_16BIT,
    .total_size = 65536
  }
};

/**
  * @brief calculate the maximum number of address pins based on eeprom model.
  * @param model: eeprom_model_t enum value to select the EEPROM model
  * @retval maximum number of address pins for the selected EEPROM model
  */
uint8_t eeprom_get_max_pins(eeprom_model_t model)
{
  uint8_t mask = EEP_CONFIG_TABLE[model].addr_pin_mask;
  uint8_t count = 0;
  while (mask)
  {
    count += (mask & 1);
    mask >>= 1;
  }
  return (1 << count);
}

/**
  * @brief  init EEPROM device instance with selected model's configuration
  * @param  dev: pointer to eeprom_dev_t instance to initialize
  * @param  hi2cX: pointer to I2C handle (e.g., &hi2c1)
  * @param  selected_model: eeprom_model_t enum value to select the EEPROM model and its configuration
  * @param  device_addr: the actual I2C device address to use (lower bits will be masked based on model's addressing scheme)
  * @retval None
  */
i2c_state_t eeprom_init_dev(eeprom_dev_t *dev, I2C_HandleTypeDef *hi2cX, eeprom_model_t selected_model, uint8_t pin_index)
{
  if (selected_model >= EEP_MODEL_MAX)
  {
    LOG_ERR("Invalid EEPROM model selected!\n");
    return I2C_STATE_INIT_DEV_ERROR;
  }

  if (!dev || !hi2cX)
  {
    LOG_ERR("I2C handle or dev pointer is NULL!\n");
    return I2C_STATE_NULL_PTR;
  }

  uint8_t max_pins = eeprom_get_max_pins(selected_model);
  if (pin_index >= max_pins)
  {
    LOG_ERR("Invalid pin index for this model! The range is 0 to %d\n", max_pins - 1);
    return I2C_STATE_INIT_DEV_ERROR;
  }

  dev->model = selected_model;
  dev->hi2cX = hi2cX;
  dev->config = &EEP_CONFIG_TABLE[selected_model];
  dev->device_base_addr = 0xA0 | ((pin_index << dev->config->addr_pin_shift) & dev->config->addr_pin_mask);
  LOG_DEBUG("EEPROM initialized device_base_addr: 0x%02X\n", dev->device_base_addr);
  return I2C_STATE_OK;
}

/**
  * @brief  helper function to calculate the actual I2C device address and internal memory address based on 
            the current address and the EEPROM model's addressing scheme.
  * @param  dev: pointer to eeprom_dev_t instance
  * @param  cur_addr: current EEPROM memory address to access
  * @param  p_dev_addr: pointer to store the calculated I2C device address
  * @param  p_mem_addr: pointer to store the calculated internal memory address
  * @retval None
  */
static void _eep_calculate_address(eeprom_dev_t *dev, uint32_t cur_addr, uint8_t *p_dev_addr, uint16_t *p_mem_addr)
{
  uint8_t page_bits = 0;
  switch (dev->model)
  {
    case EEP_MODEL_AT24C01:
    case EEP_MODEL_AT24C02:
    {
      *p_dev_addr = dev->device_base_addr;
      *p_mem_addr = (uint16_t)(cur_addr & 0xFF);
      break;
    }
    case EEP_MODEL_AT24C04:
    {
      page_bits = (uint8_t)((cur_addr >> 8) & BIT0);
      *p_dev_addr = dev->device_base_addr | (page_bits << 1);
      *p_mem_addr = (uint16_t)(cur_addr & 0xFF);
      break;
    }
    case EEP_MODEL_AT24C08:
    {
      page_bits = (uint8_t)((cur_addr >> 8) & (BIT1 | BIT0));
      *p_dev_addr = dev->device_base_addr | (page_bits << 1);
      *p_mem_addr = (uint16_t)(cur_addr & 0xFF);
      break;
    }
    case EEP_MODEL_AT24C16:
    {
      page_bits = (uint8_t)((cur_addr >> 8) & (BIT2 | BIT1 | BIT0));
      *p_dev_addr = dev->device_base_addr | (page_bits << 1);
      *p_mem_addr = (uint16_t)(cur_addr & 0xFF);
      break;
    }
    default: // 16-bit address
    {
      *p_dev_addr = dev->device_base_addr;
      *p_mem_addr = (uint16_t)(cur_addr & 0xFFFF);
      break;
    }
  }
}

/**
  * @brief  helper function to wait until EEPROM is ready after write operation by polling the EEPROM device
  * @param  dev: pointer to eeprom_dev_t instance
  * @param  dev_addr: the I2C device address to poll for readiness
  * @retval i2c_state_t (I2C_STATE_OK, I2C_STATE_ERROR, etc.)
  */
static i2c_state_t _eep_wait_ready(eeprom_dev_t *dev, uint8_t dev_addr) {
  uint8_t retry = 3;
  while (retry--)
  {
    if (HAL_I2C_IsDeviceReady(dev->hi2cX, dev_addr, 2, EEP_WRITE_CYCLE_MS) == HAL_OK)
    {
      return I2C_STATE_OK;
    }
    HAL_Delay(1); 
  }
  return I2C_STATE_TIMEOUT;
}

/**
  * @brief  common high-flexibility EEPROM continuous write function that handles page roll over and write cycle timing
  * @param  dev: pointer to initialized eeprom_dev_t instance
  * @param  addr: starting EEPROM memory address to write to
  * @param  p_data: pointer to data buffer to write
  * @param  len: length of data to write
  * @retval i2c_state_t (I2C_STATE_OK, I2C_STATE_ERROR, etc.)
 */
i2c_state_t eeprom_write(eeprom_dev_t *dev, uint32_t addr, uint8_t *p_data, uint32_t len)
{
  if (!dev || !dev->hi2cX || !dev->config || !p_data)
  {
    LOG_ERR("NULL parameters for EEPROM write!\n");
    return I2C_STATE_NULL_PTR;
  } 
  else if (len == 0) 
  {
    LOG_ERR("Zero length for EEPROM write!\n");
    return I2C_STATE_ZERO_LEN;
  }

  uint32_t cur_addr = addr;
  uint32_t bytes_left = len;
  uint8_t *p_buf = p_data;

  while (bytes_left > 0)
  {
    if (cur_addr >= dev->config->total_size) 
    {
      LOG_ERR("Memory address out of range! cur_addr=0x%04X, total_size=0x%04X\n", (unsigned int)cur_addr, (unsigned int)dev->config->total_size);
      return I2C_STATE_MEM_ADDR_ERROR;
    }

    /* handle page boundary: calculate how many bytes can be written in the current page before rolling over */
    uint32_t page_mask = (uint32_t)(dev->config->page_size - 1);
    uint32_t page_start = cur_addr & (~page_mask);
    uint32_t page_end = page_start + dev->config->page_size;
    uint32_t space_in_page = page_end - cur_addr;
    uint32_t bytes_to_write = (bytes_left < space_in_page) ? bytes_left : space_in_page;

    uint8_t dev_addr;
    uint16_t mem_addr;
    _eep_calculate_address(dev, cur_addr, &dev_addr, &mem_addr);

    if (i2c_bus_reg_write(dev->hi2cX, dev_addr, mem_addr, (uint16_t)dev->config->memaddr_size, p_buf, bytes_to_write) != I2C_STATE_OK)
    {
      LOG_ERR("EEPROM write error!\n");
      return I2C_STATE_TX_ERROR;
    }

    if (_eep_wait_ready(dev, dev_addr) != I2C_STATE_OK)
    {
      LOG_ERR("EEPROM write timeout error!\n");
      return I2C_STATE_TIMEOUT;
    }

    bytes_left -= bytes_to_write;
    p_buf += bytes_to_write;
    cur_addr += bytes_to_write;
  }
  return I2C_STATE_OK;
}

/**
  * @brief  common high-flexibility EEPROM continuous read function that handles block boundary for 8-bit address devices
  * @param  dev: pointer to initialized eeprom_dev_t instance
  * @param  addr: starting EEPROM memory address to read from
  * @param  p_data: pointer to data buffer to read
  * @param  len: length of data to read
  * @retval i2c_state_t (I2C_STATE_OK, I2C_STATE_ERROR, etc.)
 */
i2c_state_t eeprom_read(eeprom_dev_t *dev, uint32_t addr, uint8_t *p_data, uint32_t len)
{
  if (!dev || !dev->hi2cX || !dev->config || !p_data)
  {
    LOG_ERR("NULL parameters for EEPROM read!\n");
    return I2C_STATE_NULL_PTR;
  } 
  else if (len == 0) 
  {
    LOG_ERR("Zero length for EEPROM read!\n");
    return I2C_STATE_ZERO_LEN;
  }

  uint32_t cur_addr = addr;
  uint32_t bytes_left = len;
  uint8_t *p_buf = p_data;

  while (bytes_left > 0)
  {
    if (cur_addr >= dev->config->total_size)
    {
      LOG_ERR("Memory address out of range! cur_addr=0x%04X, total_size=0x%04X\n", (unsigned int)cur_addr, (unsigned int)dev->config->total_size);
      return I2C_STATE_MEM_ADDR_ERROR;
    }

    uint8_t dev_addr;
    uint16_t mem_addr;
    _eep_calculate_address(dev, cur_addr, &dev_addr, &mem_addr);

    /* handle block boundary for 8-bit address devices */
    uint32_t bytes_to_read;
    if (dev->config->memaddr_size == I2C_MEMADD_SIZE_8BIT)
    {
      uint32_t max_read = 256 - (mem_addr & 0xFF);
      bytes_to_read = (bytes_left < max_read) ? bytes_left : max_read;
    }
    else
    {
      bytes_to_read = bytes_left;
    }

    if (i2c_bus_reg_read(dev->hi2cX, dev_addr, mem_addr, (uint16_t)dev->config->memaddr_size, p_buf, bytes_to_read) != I2C_STATE_OK)
    {
      LOG_ERR("EEPROM read error!\n");
      return I2C_STATE_RX_ERROR; 
    }

    bytes_left -= bytes_to_read;
    p_buf += bytes_to_read;
    cur_addr += bytes_to_read;
  }
  return I2C_STATE_OK;
}
