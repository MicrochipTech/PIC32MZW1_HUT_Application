/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_rtcc.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_RTCC_Initialize" and "APP_RTCC_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_RTCC_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_RTCC_H
#define _APP_RTCC_H

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
#include "definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

typedef enum
{
    /* Application's state machine's initial state. */
    APP_RTCC_STATE_INIT,
            APP_RTCC_STATE_WAIT_ALARM,
            APP_RTCC_STATE_ERROR
    /* TODO: Define states used by the application state machine. */

} APP_RTCC_STATES;


typedef struct
{
    /* The application's current state */
    APP_RTCC_STATES state;
} APP_RTCC_DATA;


void APP_RTCC_Initialize ( void );
void APP_RTCC_Tasks( void );

#endif /* _APP_RTCC_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

