/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_pwm.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_PWM_Initialize" and "APP_PWM_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_PWM_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_PWM_H
#define _APP_PWM_H

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

typedef enum
{
    /* Application's state machine's initial state. */
    APP_PWM_STATE_INIT=0,
    APP_PWM_STATE_IDLE_TASKS,
    /* TODO: Define states used by the application state machine. */

} APP_PWM_STATES;

typedef struct
{
    /* The application's current state */
    APP_PWM_STATES state;

    /* TODO: Define any additional data used by the application. */

} APP_PWM_DATA;

void APP_PWM_Initialize ( void );
void APP_PWM_Tasks( void );



#endif /* _APP_PWM_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

