#ifndef _APP_CONF_H    /* Guard against multiple inclusion */
#define _APP_CONF_H

#include "third_party/rtos/FreeRTOS/Source/include/task.h"
#include "app_control.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

/* TODO:  Include other files here if needed. */


/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

    //#define MODE_DEV  /*Comment this out for MODE_RELEASE*/
#ifndef MODE_DEV
#define MODE_RELEASE
#endif


    /*COMMENT OUT FOR RELEASE*/
    //#define LOOP_DISABLE /*Uncomment this to run the app tasks once and not print too much on UART*/
#define APP_PRINT_TRACE_DISABLE /*Reduce noise in the UART console to see WLAN stack other prints*/
    //#define APP_PRINT_INFO_DISABLE /*Reduce noise in the UART console to see WLAN stack other prints*/


#define SPI1_APP_ENABLE
#if defined(SPI1_APP_ENABLE) && !defined(LOOP_DISABLE)
#define SPI1_APP_LOOP
#endif

#define SPI2_APP_ENABLE
#if defined(SPI2_APP_ENABLE) && !defined(LOOP_DISABLE)
#define SPI2_APP_LOOP
#endif

#if !defined(XPRJ_pic32mz_w1_wfiiot_freertos)
    #define I2C_APP_ENABLE
    #if defined(I2C_APP_ENABLE) && !defined(LOOP_DISABLE)
    #define I2C_APP_LOOP
    #endif
#else
    #define I2C_SENSOR_APP_ENABLE
    #if defined(I2C_SENSOR_APP_ENABLE) && !defined(LOOP_DISABLE)
    #define I2C_SENSOR_APP_LOOP
    //#define I2C_SENSOR_FULL_THROTTLE 
    #endif
#endif
    
#define ADCHS_APP_ENABLE
#if defined(ADCHS_APP_ENABLE) && !defined(LOOP_DISABLE)
#define ADCHS_APP_LOOP
#endif

#define ECC608_APP_ENABLE
#if defined (ECC608_APP_ENABLE) && !defined(LOOP_DISABLE)
#define ECC608_APP_LOOP
#endif

#define USB_APP_ENABLE
#if defined(USB_APP_ENABLE) && !defined(LOOP_DISABLE)
#define USB_APP_LOOP
#endif

#define PWM_APP_ENABLE

#define RTCC_APP_ENABLE
#if defined(RTCC_APP_ENABLE) && !defined(LOOP_DISABLE)
#define RTCC_APP_LOOP
#endif


#define DO_PRAGMA(x) _Pragma (#x)
#define TODO(x) DO_PRAGMA(message ("TODO - " #x))

#if defined(MODE_RELEASE) && (defined(LOOP_DISABLE) || defined(APP_PRINT_INFO_DISABLE)) 
#error "Mandatory features disabled in release build"
#endif

#define SUPPORT_VT100 
#ifdef SUPPORT_VT100 
#define TERM_GREEN "\x1B[32m"
#define TERM_RED   "\x1B[31m"
#define TERM_YELLOW "\x1B[33m"
#define TERM_CYAN "\x1B[36m"
#define TERM_WHITE "\x1B[47m"
#define TERM_RESET "\x1B[0m"
#define TERM_BG_RED "\x1B[41m" 
#define TERM_BOLD "\x1B[1m" 
#define TERM_UL "\x1B[4m"

#define TERM_CTRL_RST "\x1B\x63"
#define TERM_CTRL_CLRSCR "\x1B[2J"
#else
#define TERM_GREEN 
#define TERM_RED   
#define TERM_YELLOW 
#define TERM_CYAN 
#define TERM_WHITE 
#define TERM_RESET 
#define TERM_BG_RED  
#define TERM_BOLD  
#define TERM_UL 

#define TERM_CTRL_RST 
#define TERM_CTRL_CLRSCR 

#endif

#if !defined(APP_PRINT_INFO_DISABLE)
#define PRINTF_CRIT_INFO(f_, ...)\
do{\
 taskENTER_CRITICAL();\
 printf((f_), ##__VA_ARGS__);\
 taskEXIT_CRITICAL();\
}while(0)
#else
#define PRINTF_CRIT_INFO(f_, ...)
#endif


#if !defined(APP_PRINT_TRACE_DISABLE)
#define PRINTF_CRIT_TRACE(f_, ...)\
do{\
 taskENTER_CRITICAL();\
 printf((f_), ##__VA_ARGS__);\
 taskEXIT_CRITICAL();\
}while(0)
#else
#define PRINTF_CRIT_TRACE(f_, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
