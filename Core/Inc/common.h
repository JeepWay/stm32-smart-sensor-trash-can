#ifndef __COMMON_H__
#define __COMMON_H__

#include "stm32f1xx_hal.h"

/**
  * basic include files
  */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/**
  * basic type definitions for better readability
  */
#define BYTE  uint8_t
#define WORD  uint16_t
#define DWORD uint32_t
#define UINT  unsigned int

#define BIT0  0x00000001U
#define BIT1  0x00000002U
#define BIT2  0x00000004U
#define BIT3  0x00000008U
#define BIT4  0x00000010U
#define BIT5  0x00000020U
#define BIT6  0x00000040U
#define BIT7  0x00000080U
#define BIT8  0x00000100U
#define BIT9  0x00000200U
#define BIT10 0x00000400U
#define BIT11 0x00000800U
#define BIT12 0x00001000U
#define BIT13 0x00002000U
#define BIT14 0x00004000U
#define BIT15 0x00008000U
#define BIT16 0x00010000U
#define BIT17 0x00020000U
#define BIT18 0x00040000U
#define BIT19 0x00080000U
#define BIT20 0x00100000U
#define BIT21 0x00200000U
#define BIT22 0x00400000U
#define BIT23 0x00800000U
#define BIT24 0x01000000U
#define BIT25 0x02000000U
#define BIT26 0x04000000U
#define BIT27 0x08000000U

typedef enum
{
  LED_OFF,
  LED_ON,
  LED_BLINK
} led_state_t;

/**
  * timer related
  */
void DWT_Delay_Init(void);
void delay_us(uint32_t us);

/**
  * Debug logging define and macros
  */
#define LOG_LEVEL_ERR       0
#define LOG_LEVEL_WARNING   1
#define LOG_LEVEL_NOTICE    2
#define LOG_LEVEL_INFO      3
#define LOG_LEVEL_DEBUG     4

#ifndef ENABLE_UART_DEBUG
    #define ENABLE_UART_DEBUG 0
#endif

#if (ENABLE_UART_DEBUG == 1)
    #ifndef CONFIG_UART_DEBUG_LEVEL
        #define CONFIG_UART_DEBUG_LEVEL LOG_LEVEL_DEBUG
    #endif
    #ifndef CONFIG_UART_DEBUG_ONLY_FILE_NAME
        #define CONFIG_UART_DEBUG_ONLY_FILE_NAME 1
    #endif
    #if (CONFIG_UART_DEBUG_ONLY_FILE_NAME == 1)
        #define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : \
                        __builtin_strrchr(__FILE__, '\\') ? __builtin_strrchr(__FILE__, '\\') + 1 : __FILE__)
    #else
        #define __FILENAME__ __FILE__
    #endif
#endif

#if (ENABLE_UART_DEBUG == 1)
    #if CONFIG_UART_DEBUG_LEVEL > LOG_LEVEL_DEBUG
        #error "Invalid CONFIG_UART_DEBUG_LEVEL value. It should be between 0 and 7."
    #endif

    #if CONFIG_UART_DEBUG_LEVEL >= LOG_LEVEL_ERR
        #define LOG_ERR(fmt, ...) printf("[ERROR] [%s:%d] " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
    #else
        #define LOG_ERR(fmt, ...)
    #endif

    #if CONFIG_UART_DEBUG_LEVEL >= LOG_LEVEL_WARN
        #define LOG_WARN(fmt, ...) printf("[WARN] [%s:%d] " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
    #else
        #define LOG_WARN(fmt, ...)
    #endif

    #if CONFIG_UART_DEBUG_LEVEL >= LOG_LEVEL_NOTICE
        #define LOG_NOTICE(fmt, ...) printf("[NOTICE] [%s:%d] " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
    #else
        #define LOG_NOTICE(fmt, ...)
    #endif

    #if CONFIG_UART_DEBUG_LEVEL >= LOG_LEVEL_INFO
        #define LOG_INFO(fmt, ...) printf("[INFO] [%s:%d] " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
    #else
        #define LOG_INFO(fmt, ...)
    #endif

    #if CONFIG_UART_DEBUG_LEVEL >= LOG_LEVEL_DEBUG
        #define LOG_DEBUG(fmt, ...) printf("[DEBUG] [%s:%d] " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
    #else
        #define LOG_DEBUG(fmt, ...)
    #endif

    #define LOG_RAW(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define LOG_ERR(fmt, ...)
    #define LOG_WARN(fmt, ...)
    #define LOG_NOTICE(fmt, ...)
    #define LOG_INFO(fmt, ...)
    #define LOG_DEBUG(fmt, ...)
    #define LOG_RAW(fmt, ...)
#endif

#endif /* __COMMON_H__ */