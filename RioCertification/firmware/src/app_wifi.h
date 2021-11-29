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
    

#define WIFI_DEFAULT_REG_DOMAIN "GEN"

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

