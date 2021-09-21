/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ecc608.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_ECC608_Initialize" and "APP_ECC608_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_ECC608_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_ECC608_H
#define _APP_ECC608_H

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
#include "app_config.h"
#include "app_control.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif

typedef enum
{
    APP_ECC608_STATE_INIT = 0,
    APP_ECC608_STATE_ATCA_INIT,
    APP_ECC608_STATE_READ_SERIAL,
    APP_ECC608_STATE_ERROR,
    APP_ECC608_STATE_DONE
} APP_ECC608_STATES;

typedef enum {
    ECC608_APP_TRANSFER_STATUS_SUCCESS,
    ECC608_APP_TRANSFER_STATUS_IN_PROGRESS,
    ECC608_APP_TRANSFER_STATUS_NACK,
    ECC608_APP_TRANSFER_STATUS_ERROR
} ECC608_APP_TRANSFER_STATUS;

typedef struct
{
    /* The application's current state */
    APP_ECC608_STATES state;

} APP_ECC608_DATA;

void APP_ECC608_Initialize ( void );
void APP_ECC608_Tasks( void );

#endif /* _APP_ECC608_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

