#ifndef _DEBUG_H_
#define _DEBUG_H_

#if (ENABLE_UART_DEBUG == 1)
    #define COMMAND_BUF_SIZE 64
#else
    #define COMMAND_BUF_SIZE 1
#endif

#ifndef ENABLE_DEBUG_EEPROM
    #define ENABLE_DEBUG_EEPROM 0
#endif

#ifndef ENABLE_DEBUG_SG90
    #define ENABLE_DEBUG_SG90 0
#endif

#ifndef ENABLE_DEBUG_LED
    #define ENABLE_DEBUG_LED 0
#endif

#if (ENABLE_UART_DEBUG == 1)
extern uint8_t rx_buffer[COMMAND_BUF_SIZE];
extern void debug_command_handler(void);
#endif

#endif /* _DEBUG_H_ */