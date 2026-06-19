#include "usart.h"
#include "tim.h"

#include "common.h"
#include "debug.h"
#include "bsp_eeprom.h"
#include "bsp_hcsr04.h"
#include "bsp_sg90.h"

#if (ENABLE_UART_DEBUG == 1)

uint8_t rx_buffer[COMMAND_BUF_SIZE];
uint8_t command_buffer[COMMAND_BUF_SIZE];
static volatile uint16_t command_len;
static volatile uint8_t rx_complete_flag;

static char param0[20];
static uint8_t param1 = 0;
static uint8_t param2 = 0;
static uint8_t param3 = 0;
static uint8_t param4 = 0;

int _write(int file, char *ptr, int len)
{
  (void)file;
  HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
  if(huart->Instance == USART1)
  {
    memcpy(command_buffer, rx_buffer, size);
    memset(rx_buffer, 0, COMMAND_BUF_SIZE);
    command_len = size;
    rx_complete_flag = 1;
    
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, rx_buffer, COMMAND_BUF_SIZE);
  }
}

static void _parse_command(void)
{
  memset(param0, 0, sizeof(param0));
  param1 = 0; param2 = 0; param3 = 0; param4 = 0;

  unsigned int t1 = 0, t2 = 0, t3 = 0, t4 = 0;

  int parsed_count = sscanf((char*)command_buffer, "%19s %x %x %x %x", param0, &t1, &t2, &t3, &t4);

  if (parsed_count >= 2) param1 = (uint8_t)t1;
  if (parsed_count >= 3) param2 = (uint8_t)t2;
  if (parsed_count >= 4) param3 = (uint8_t)t3;
  if (parsed_count >= 5) param4 = (uint8_t)t4;
}

#if (ENABLE_DEBUG_EEPROM == 1)
void debug_command_eeprom_handler(void)
{
  /* command: eeprom [operation] [HighAddr] [LowAddr] [Data] */
  static eeprom_dev_t eeprom_08;
  uint32_t addr;
  i2c_state_t res;
  eeprom_init_dev(&eeprom_08, &hi2c1, EEP_MODEL_AT24C08, 0);

  #define DEBUG_EEPROM_BUFFER_SIZE 10
  uint8_t write_buffer[DEBUG_EEPROM_BUFFER_SIZE];
  uint8_t read_buffer[DEBUG_EEPROM_BUFFER_SIZE];
  for(int i=0; i<DEBUG_EEPROM_BUFFER_SIZE; i++)
      write_buffer[i] = i;
  memset(read_buffer, 0, sizeof(read_buffer));

  switch (param1)
  {
    case 0x00:
      LOG_INFO("HAL_I2C_IsDeviceReady()=%d\n", HAL_I2C_IsDeviceReady(
        eeprom_08.hi2cX, 
        eeprom_08.device_base_addr,
        1, 15
      ));
      break;
    case 0x01:
      /* eeprom write 1 byte */
      addr = ((uint32_t)param2 << 8) | param3;
      res = eeprom_write(&eeprom_08, addr, &param4, 1);
      LOG_INFO("EEPROM write result: %d, data: %02x\n", res, (UINT)param4);
      break;
    case 0x02:
      /* eeprom read 1 byte */
      addr = ((uint32_t)param2 << 8) | param3;
      uint8_t data;
      res = eeprom_read(&eeprom_08, addr, &data, 1);
      LOG_INFO("EEPROM read result: %d, data: %02x\n", res, (UINT)data);
      break;
    case 0x06:
      /* eeprom write/read 10 byte */
      res = eeprom_write(&eeprom_08, 0x0000, write_buffer, DEBUG_EEPROM_BUFFER_SIZE);
      if (res != I2C_STATE_OK)
      {
        LOG_ERR("eeprom_write error: %d\n", res);
      }
      
      res = eeprom_read(&eeprom_08, 0x0000, read_buffer, DEBUG_EEPROM_BUFFER_SIZE);
      if (res != I2C_STATE_OK)
      {
        LOG_ERR("eeprom_read error: %d\n", res);
      }

      for (int i = 0; i < 10; i++)
      {
        if (read_buffer[i] != write_buffer[i])
        {
          LOG_ERR("EEPROM Test Failed at index %d: expected %d, got %d\n", i, write_buffer[i], read_buffer[i]);
          return;
        }
      }
      LOG_INFO("EEPROM Test success\n");
      break;
    default:
      LOG_ERR("Unknown EEPROM command: %d\n", param1);
      break;
  }
}
#endif

#if (ENABLE_DEBUG_SG90 == 1)
void debug_command_sg90_handler(void)
{
  /* command: sg90 [angle] */
  static sg90_dev_t sg90;
  sg90_init_dev(&sg90, &htim1, TIM_CHANNEL_4);
  uint16_t angle = param1;

  if (angle >= 0 && angle <= 180)
  {
    sg90_set_angle(&sg90, angle);
    LOG_INFO("sg90 angle:%d\n", angle);
  }
  else
  {
    LOG_WARN("Invalid angle:%d\n", angle);
  }
}
#endif

#if (ENABLE_DEBUG_LED == 1)
void debug_command_led_handler(void)
{
  /* command: led [state] */
  static led_state_t led1_state = LED_OFF;

  switch (param1)
  {
    case 0x00:
      LOG_INFO("Led1 turn on\n");
      led1_state = LED_ON;
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
      break;
    case 0x01:
      LOG_INFO("Led1 turn off\n");
      led1_state = LED_OFF;
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
      break;
    case 0x02:
      LOG_INFO("Led1 toggle\n");
      led1_state = (led1_state == LED_OFF) ? LED_ON : LED_OFF;
      HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
      break;
    case 0x03:
      LOG_INFO("Led1 blinking\n");
      led1_state = LED_BLINK;
      break;
    default:
      LOG_ERR("Unknown LED command: %d\n", param1);
      break;
  }
}
#endif

void debug_command_handler(void)
{
  if(rx_complete_flag)
  {
    _parse_command();
    rx_complete_flag = 0;
    LOG_DEBUG("command= %s\n", command_buffer);
    if (0)
    {

    }
#if (ENABLE_DEBUG_EEPROM == 1)
    else if(strncmp(param0, "eeprom", 6) == 0)
    {
      debug_command_eeprom_handler();
    }
#endif
#if (ENABLE_DEBUG_SG90 == 1)
    else if(strncmp(param0, "sg90", 4) == 0)
    {
      debug_command_sg90_handler();
    }
#endif
#if (ENABLE_DEBUG_LED == 1)
    else if(strncmp(param0, "led", 3) == 0)
    {
      debug_command_led_handler();
    }
#endif
    memset(command_buffer, 0, COMMAND_BUF_SIZE);
  }
}
#endif  /* (ENABLE_UART_DEBUG == 1) */