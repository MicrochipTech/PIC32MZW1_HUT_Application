/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_wifi.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_WIFI_Initialize" and "APP_WIFI_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_WIFI_STATES" definition).  Both
    are defined here for convenience.
 *******************************************************************************/

#ifndef _APP_WIFI_H
#define _APP_WIFI_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "FreeRTOS.h"
#include "task.h"
#include "app_config.h"
#include "definitions.h"
#include "system/sys_time_h2_adapter.h"


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
    // DOM-IGNORE-END

    typedef enum {
        OPEN = 0,
        WPA2,
        WPAWPA2MIXED,
        WEP,
        NONE
    } WIFI_AUTH;
    
extern char* WIFI_AUTH_STRING[];

#if defined(MODE_RELEASE)
#define DEFAULT_SSID "DEMO_AP_RIO2"
#define DEFAULT_SSID_PSK " "
#define DEFAULT_AUTH_MODE (WIFI_AUTH)OPEN
#elif defined(MODE_DEV)
#define DEFAULT_SSID "DEMO_AP_RIO2"
#define DEFAULT_SSID_PSK " "
#define DEFAULT_AUTH_MODE (WIFI_AUTH)OPEN
#endif 

#define WIFI_DEFAULT_REG_DOMAIN "GEN"

    typedef enum {
        /* Application's state machine's initial state. */
        APP_WIFI_STATE_INIT = 0,
        APP_WIFI_STATE_WDRV_INIT_READY,
        APP_WIFI_STATE_SET_REG_DOMAIN,
        APP_WIFI_STATE_WAIT_SET_REG_DOMAIN,
        APP_WIFI_TCPIP_WAIT_FOR_TCPIP_INIT,
        APP_WIFI_TCPIP_ERROR,
        APP_WIFI_CONFIG,
        APP_WIFI_CONNECT,
        APP_WIFI_IDLE,
        APP_WIFI_RECONNECT,
        APP_NO_WIFI_IDLE,
    } APP_WIFI_STATES;

    void APP_WIFI_Initialize(void);
    void APP_WIFI_Tasks(void);

#endif /* _APP_WIFI_H */

    //DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

