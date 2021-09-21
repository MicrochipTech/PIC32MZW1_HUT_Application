/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_usb.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "stdlib.h"
#include "app_usb.h"
#include "app_config.h"

#define RAND_UPPER 255
#define RAND_LOWER 50

extern unsigned int hut_peripheral;

APP_USB_DATA app_usbData;

uint8_t CACHE_ALIGN switchPromptUSB[150] ;
uint8_t CACHE_ALIGN commandResponse[1024] ;

uint8_t CACHE_ALIGN cdcReadBuffer[APP_READ_BUFFER_SIZE];
uint8_t CACHE_ALIGN cdcWriteBuffer[APP_READ_BUFFER_SIZE];

/*******************************************************
 * USB CDC Device Events - Application Event Handler
 *******************************************************/

USB_DEVICE_CDC_EVENT_RESPONSE APP_USBDeviceCDCEventHandler
(
        USB_DEVICE_CDC_INDEX index,
        USB_DEVICE_CDC_EVENT event,
        void * pData,
        uintptr_t userData
        ) {
    APP_USB_DATA * app_usbDataObject;
    USB_CDC_CONTROL_LINE_STATE * controlLineStateData;
    USB_DEVICE_CDC_EVENT_DATA_READ_COMPLETE * eventDataRead;

    app_usbDataObject = (APP_USB_DATA *) userData;

    switch (event) {
        case USB_DEVICE_CDC_EVENT_GET_LINE_CODING:

            /* This means the host wants to know the current line
             * coding. This is a control transfer request. Use the
             * USB_DEVICE_ControlSend() function to send the data to
             * host.  */

            USB_DEVICE_ControlSend(app_usbDataObject->deviceHandle,
                    &app_usbDataObject->getLineCodingData, sizeof (USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_LINE_CODING:

            /* This means the host wants to set the line coding.
             * This is a control transfer request. Use the
             * USB_DEVICE_ControlReceive() function to receive the
             * data from the host */

            USB_DEVICE_ControlReceive(app_usbDataObject->deviceHandle,
                    &app_usbDataObject->setLineCodingData, sizeof (USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_CONTROL_LINE_STATE:

            /* This means the host is setting the control line state.
             * Read the control line state. We will accept this request
             * for now. */

            controlLineStateData = (USB_CDC_CONTROL_LINE_STATE *) pData;
            app_usbDataObject->controlLineStateData.dtr = controlLineStateData->dtr;
            app_usbDataObject->controlLineStateData.carrier = controlLineStateData->carrier;

            USB_DEVICE_ControlStatus(app_usbDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);

            break;

        case USB_DEVICE_CDC_EVENT_SEND_BREAK:

            /* This means that the host is requesting that a break of the
             * specified duration be sent. Read the break duration */

            app_usbDataObject->breakData = ((USB_DEVICE_CDC_EVENT_DATA_SEND_BREAK *) pData)->breakDuration;

            /* Complete the control transfer by sending a ZLP  */
            USB_DEVICE_ControlStatus(app_usbDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);

            break;

        case USB_DEVICE_CDC_EVENT_READ_COMPLETE:

            /* This means that the host has sent some data*/
            eventDataRead = (USB_DEVICE_CDC_EVENT_DATA_READ_COMPLETE *) pData;
            app_usbDataObject->isReadComplete = true;
            app_usbDataObject->numBytesRead = eventDataRead->length;
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:

            /* The data stage of the last control transfer is
             * complete. For now we accept all the data */

            USB_DEVICE_ControlStatus(app_usbDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_SENT:

            /* This means the GET LINE CODING function data is valid. We don't
             * do much with this data in this demo. */
            break;

        case USB_DEVICE_CDC_EVENT_WRITE_COMPLETE:

            /* This means that the data write got completed. We can schedule
             * the next read. */

            app_usbDataObject->isWriteComplete = true;
            break;

        default:
            break;
    }

    return USB_DEVICE_CDC_EVENT_RESPONSE_NONE;
}

/***********************************************
 * Application USB Device Layer Event Handler.
 ***********************************************/
void APP_USBDeviceEventHandler
(
        USB_DEVICE_EVENT event,
        void * eventData,
        uintptr_t context
        ) {
    USB_DEVICE_EVENT_DATA_CONFIGURED *configuredEventData;

    switch (event) {
        case USB_DEVICE_EVENT_SOF:

            /* This event is used for switch debounce. This flag is reset
             * by the switch process routine. */
            app_usbData.sofEventHasOccurred = true;

            break;

        case USB_DEVICE_EVENT_RESET:
            app_usbData.isConfigured = false;
            break;

        case USB_DEVICE_EVENT_CONFIGURED:

            /* Check the configuration. We only support configuration 1 */
            configuredEventData = (USB_DEVICE_EVENT_DATA_CONFIGURED*) eventData;

            if (configuredEventData->configurationValue == 1) {
                /* Register the CDC Device application event handler here.
                 * Note how the app_usbData object pointer is passed as the
                 * user data */

                USB_DEVICE_CDC_EventHandlerSet(USB_DEVICE_CDC_INDEX_0, APP_USBDeviceCDCEventHandler, (uintptr_t) & app_usbData);

                /* Mark that the device is now configured */
                app_usbData.isConfigured = true;
            }

            break;

        case USB_DEVICE_EVENT_POWER_DETECTED:
            /* VBUS was detected. We can attach the device */
            USB_DEVICE_Attach(app_usbData.deviceHandle);

            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:

            /* VBUS is not available any more. Detach the device. */
            USB_DEVICE_Detach(app_usbData.deviceHandle);
            break;

        case USB_DEVICE_EVENT_SUSPENDED:
            break;

        case USB_DEVICE_EVENT_RESUMED:
        case USB_DEVICE_EVENT_ERROR:
        default:

            break;
    }
}

/*****************************************************
 * This function is called in every step of the
 * application state machine.
 *****************************************************/

bool APP_StateReset(void) {
    /* This function returns true if the device
     * was reset  */

    bool retVal;

    if (app_usbData.isConfigured == false) {
        app_usbData.state = APP_USB_STATE_WAIT_FOR_CONFIGURATION;
        app_usbData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        app_usbData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        app_usbData.isReadComplete = true;
        app_usbData.isWriteComplete = true;
        retVal = true;
    } else {
        retVal = false;
    }

    return (retVal);
}

void APP_USB_Initialize(void) {
    /* Place the App state machine in its initial state. */
    app_usbData.state = APP_USB_STATE_INIT;
    app_controlData.moduleError.eUSB=false;

    /* Device Layer Handle  */
    app_usbData.deviceHandle = USB_DEVICE_HANDLE_INVALID;

    /* Device configured status */
    app_usbData.isConfigured = false;

    /* Initial get line coding state */
    app_usbData.getLineCodingData.dwDTERate = 230400;
    app_usbData.getLineCodingData.bParityType = 0;
    app_usbData.getLineCodingData.bParityType = 0;
    app_usbData.getLineCodingData.bDataBits = 8;

    /* Read Transfer Handle */
    app_usbData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

    /* Write Transfer Handle */
    app_usbData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

    /* Initialize the read complete flag */
    app_usbData.isReadComplete = true;

    /*Initialize the write complete flag*/
    app_usbData.isWriteComplete = true;

    /* Initialize Ignore switch flag */
    app_usbData.ignoreSwitchPress = false;

    /* Reset the switch debounce counter */
    app_usbData.switchDebounceTimer = 0;

    /* Reset other flags */
    app_usbData.sofEventHasOccurred = false;

    /* To know status of Switch */
    app_usbData.isSwitchPressed = false;

    /* Set up the read buffer */
    app_usbData.cdcReadBuffer = &cdcReadBuffer[0];

    /* Set up the read buffer */
    app_usbData.cdcWriteBuffer = &cdcWriteBuffer[0];
}
#if !defined(USB_APP_LOOP) && defined(USB_APP_ENABLE)
    uint8_t nonLooopCount = 2;
#endif

void peripheral_application_msg_usb(unsigned char *msg)
{
    memset(commandResponse, 0, sizeof(commandResponse));

    if (strlen(msg) < 1024)
    {
        memcpy(commandResponse, msg, strlen(msg));
    }
    else
        memcpy(commandResponse, msg, 1024);
        
    app_usbData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

    USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0,
                    &app_usbData.writeTransferHandle, commandResponse, strlen((char*)commandResponse),
                    USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);        
}
    
void APP_USB_Tasks(void) {
#ifndef USB_APP_ENABLE
    return;
#else
    static int counter = 0;
    counter++;
        
    if(counter == 10000000)
    {
        printf("Invoking APP_USB_Task State %d\n", app_usbData.state);
        counter = 0;
    }
        
    switch (app_usbData.state) {
        case APP_USB_STATE_INIT:
            PRINTF_CRIT_INFO(TERM_YELLOW"USB"TERM_RESET": Starting task\r\n");
            printf("Starting USB Task\n");
            app_controlData.moduleError.eUSB=false;
            /* Open the device layer */
            app_usbData.deviceHandle = USB_DEVICE_Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE);

            if (app_usbData.deviceHandle != USB_DEVICE_HANDLE_INVALID) {
                /* Register a callback with device layer to get event notification (for end point 0) */
                USB_DEVICE_EventHandlerSet(app_usbData.deviceHandle, APP_USBDeviceEventHandler, 0);
                app_usbData.state = APP_USB_STATE_WAIT_FOR_CONFIGURATION;
            } else {
                /* The Device Layer is not ready to be opened. We should try
                 * again later. */
            }

            break;

        case APP_USB_STATE_WAIT_FOR_CONFIGURATION:

            /* Check if the device was configured */
            if (app_usbData.isConfigured) {
                app_usbData.state = APP_USB_STATE_SCHEDULE_WRITE;
                printf("Scheduling USB Write\n");
            }

            break;

        case APP_USB_STATE_SCHEDULE_WRITE:

            if (APP_StateReset()) {
                break;
            }

            /* Setup the write */

            app_usbData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
            app_usbData.isWriteComplete = false;
            app_usbData.state = APP_USB_STATE_WAIT_FOR_WRITE_COMPLETE;

            //random color
            int r = (rand() % (RAND_UPPER - RAND_LOWER + 1)) + RAND_LOWER; 
            int g = (rand() % (RAND_UPPER - RAND_LOWER + 1)) + RAND_LOWER; 
            int b = (rand() % (RAND_UPPER - RAND_LOWER + 1)) + RAND_LOWER; 

            sprintf((char*)switchPromptUSB,"\nCMD>>");
            
            USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0,
                    &app_usbData.writeTransferHandle, switchPromptUSB, strlen((char*)switchPromptUSB),
                    USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);

            break;

        case APP_USB_STATE_WAIT_FOR_WRITE_COMPLETE:

            if (APP_StateReset()) {
                printf("App State Reset\n");
                break;
            }
            
            if (app_usbData.isWriteComplete == true) {
                app_usbData.state = APP_USB_STATE_SCHEDULE_READ;
                app_usbData.isWriteComplete = false;
            }

            break;
        
        case APP_USB_STATE_WAIT_FOR_CMD_RESPONSE_DONE:
            
            if (APP_StateReset()) {
                printf("App State Reset\n");
                break;
            }
            
            if (app_usbData.isWriteComplete == true) {
                app_usbData.isWriteComplete = false;
                app_usbData.state = APP_USB_STATE_SCHEDULE_WRITE;
            }

            break;
        
        case APP_USB_STATE_SCHEDULE_READ:
            app_usbData.isReadComplete = false;
            memset(app_usbData.cdcReadBuffer, 0, APP_READ_BUFFER_SIZE);
            USB_DEVICE_CDC_Read(USB_DEVICE_CDC_INDEX_0,
                    &app_usbData.readTransferHandle, app_usbData.cdcReadBuffer, 
                    APP_READ_BUFFER_SIZE);
            app_usbData.state = APP_USB_STATE_WAIT_FOR_READ_COMPLETE;
            
            break;
        case APP_USB_STATE_WAIT_FOR_READ_COMPLETE:
            if (app_usbData.isReadComplete == true) {
                app_usbData.isReadComplete = false;
                app_usbData.state = APP_USB_STATE_WAIT_FOR_CMD_RESPONSE_DONE;
                /* Set hut_peripheral to 2 which indicates command
                 * was received over USB.
                 */
                hut_peripheral = 2;
                hut_application_input_cmd(app_usbData.cdcReadBuffer, strlen(app_usbData.cdcReadBuffer));
            }

            break;
        case APP_USB_STATE_DONE:

            break;
        case APP_USB_STATE_ERROR:
            app_controlData.moduleError.eUSB=true;
            break;
        default:

            break;
    }
#endif  //USB_APP_ENABLE
}


/*******************************************************************************
 End of File
 */
