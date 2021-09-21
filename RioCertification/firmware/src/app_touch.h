/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_touch.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_TOUCH_Initialize" and "APP_TOUCH_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_TOUCH_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_TOUCH_H
#define _APP_TOUCH_H

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

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* Application's state machine's initial state. */
    APP_TOUCH_STATE_INIT=0,
    APP_TOUCH_STATE_SERVICE_TASKS,
    /* TODO: Define states used by the application state machine. */

} APP_TOUCH_STATES;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */


void APP_TOUCH_Initialize ( void );
void APP_TOUCH_Tasks( void );

#ifdef __cplusplus
}
#endif

#endif /* _APP_TOUCH_H */

