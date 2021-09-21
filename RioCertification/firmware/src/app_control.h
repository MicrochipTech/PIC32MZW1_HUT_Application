/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_control.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_CONTROL_Initialize" and "APP_CONTROL_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_CONTROL_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_CONTROL_H
#define _APP_CONTROL_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "FreeRTOS.h"
#include "task.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif

typedef enum
{
    APP_CONTROL_STATE_INIT=0,
    APP_CONTROL_STATE_SERVICE_TASKS,
    APP_CONTROL_STATE_ERROR
} APP_CONTROL_STATES;

#define CTRL_MAX_REGDOM_SIZE 16
#define CTRL_WLAN_SSID_MAX_LEN   256
#define CTRL_WLAN_PASS_MAX_LEN   256
typedef struct __attribute__((packed, aligned(4))) {
    uint32_t signature;
    uint32_t authMode;
    char ssid[CTRL_WLAN_SSID_MAX_LEN];
    char pass[CTRL_WLAN_PASS_MAX_LEN];
    char regDom[CTRL_MAX_REGDOM_SIZE];
}CONFIG_DATA;


typedef struct{
    bool eADCHS;
    bool eCONTROL;
    bool eECC608;
    bool eETH;
    bool eI2CEEPROM;
    bool ePWM;
    bool eRTCC;
    bool eSPI2FLASH;
    bool eSST26;
    bool eUSB;
    bool eWIFI;
}APP_MODULE_ERROR;

typedef struct
{
    uintptr_t assocHandle;
    int8_t rssi;
}APP_RSSI_DATA;

typedef struct
{
    APP_CONTROL_STATES state;
    CONFIG_DATA wlanConfig;
    bool wlanConfigChange;
    bool wlanConfigvalid;
    bool wlanREGChange;
    bool connectWiFi;
    bool isWifiConnected;
    APP_RSSI_DATA rssiData;
    APP_MODULE_ERROR moduleError;
    bool moduleErrorFlag;
} APP_CONTROL_DATA;

extern APP_CONTROL_DATA app_controlData;

void APP_CONTROL_Initialize ( void );
void APP_CONTROL_Tasks( void );

#endif /* _APP_CONTROL_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

