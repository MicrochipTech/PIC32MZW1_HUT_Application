/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_usb.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_USB_Initialize" and "APP_USB_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_USB_STATES" definition).  Both
    are defined here for convenience.
 *******************************************************************************/

#ifndef _APP_USB_H
#define _APP_USB_H

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
#define APP_READ_BUFFER_SIZE                                512
#define APP_USB_SWITCH_DEBOUNCE_COUNT_FS                    75
#define APP_USB_SWITCH_DEBOUNCE_COUNT_HS                    100

    typedef enum {
        /* Application's state machine's initial state. */
        APP_USB_STATE_INIT = 0,

        /* Application waits for device configuration*/
        APP_USB_STATE_WAIT_FOR_CONFIGURATION,

        /* Wait for the TX to get completed */
        APP_USB_STATE_SCHEDULE_WRITE,

        /* Wait for the write to complete */
        APP_USB_STATE_WAIT_FOR_WRITE_COMPLETE,
        
        /* Wait for the RX to get completed */
        APP_USB_STATE_SCHEDULE_READ,

        /* Wait for the read to complete */
        APP_USB_STATE_WAIT_FOR_READ_COMPLETE,
        
        /* Wait for command response to be sent to host */
        APP_USB_STATE_WAIT_FOR_CMD_RESPONSE_DONE,
                
        /*No loop - Done*/
        APP_USB_STATE_DONE,

        /* Application Error state*/
        APP_USB_STATE_ERROR

    } APP_USB_STATES;

    typedef struct {
        /* Device layer handle returned by device layer open function */
        USB_DEVICE_HANDLE deviceHandle;

        /* Application's current state*/
        APP_USB_STATES state;

        /* Set Line Coding Data */
        USB_CDC_LINE_CODING setLineCodingData;

        /* Device configured state */
        bool isConfigured;

        /* Get Line Coding Data */
        USB_CDC_LINE_CODING getLineCodingData;

        /* Control Line State */
        USB_CDC_CONTROL_LINE_STATE controlLineStateData;

        /* Read transfer handle */
        USB_DEVICE_CDC_TRANSFER_HANDLE readTransferHandle;

        /* Write transfer handle */
        USB_DEVICE_CDC_TRANSFER_HANDLE writeTransferHandle;

        /* True if a character was read */
        bool isReadComplete;

        /* True if a character was written*/
        bool isWriteComplete;

        /* True is switch was pressed */
        bool isSwitchPressed;

        /* True if the switch press needs to be ignored*/
        bool ignoreSwitchPress;

        /* Flag determines SOF event occurrence */
        bool sofEventHasOccurred;

        /* Break data */
        uint16_t breakData;

        /* Switch debounce timer */
        unsigned int switchDebounceTimer;

        /* Switch debounce timer count */
        unsigned int debounceCount;

        /* Application CDC read buffer */
        uint8_t * cdcReadBuffer;

        /* Application CDC Write buffer */
        uint8_t * cdcWriteBuffer;

        /* Number of bytes read from Host */
        uint32_t numBytesRead;
    } APP_USB_DATA;
    void APP_USB_Initialize(void);

    void APP_USB_Tasks(void);



#endif /* _APP_USB_H */

    //DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

