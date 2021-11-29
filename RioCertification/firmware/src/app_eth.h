/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_eth.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_ETH_Initialize" and "APP_ETH_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_ETH_STATES" definition).  Both
    are defined here for convenience.
 *******************************************************************************/

#ifndef _APP_ETH_H
#define _APP_ETH_H

#include <stdint.h>
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

    #define TCPIP_HUT_REMOTE_IP_ADDR "255.255.255.255"
    #define TCPIP_HUT_LOCAL_SERVER_PORT 8000
    #define TCPIP_HUT_REMOTE_SERVER_PORT 8000

    typedef enum {
        /* Application's state machine's initial state. */
        APP_ETH_STATE_INIT = 0,
        ETH_APP_TCPIP_WAIT_FOR_IP,
        ETH_APP_TCPIP_DONE,
        ETH_APP_PORT_OPEN,
        ETH_APP_WAIT_FOR_CMD_RSP,
        ETH_APP_TCPIP_ERROR,
        /* TODO: Define states used by the application state machine. */

    } APP_ETH_STATES;

    typedef struct {
        /* The application's current state */
        APP_ETH_STATES state;

   		short 	   uSkt;
   		
        short 	   tSkt;
        
		unsigned char  cmdBuffer[1024];

   		unsigned char  cmdResponse[1500];

        /* TODO: Define any additional data used by the application. */

    } APP_ETH_DATA;


    void APP_ETH_Initialize(void);
    void APP_ETH_Tasks(void);



#endif /* _APP_ETH_H */

    //DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

