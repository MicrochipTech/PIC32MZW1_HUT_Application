/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_sst26.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_SST26_Initialize" and "APP_SST26_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_SST26_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_SST26_H
#define _APP_SST26_H

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

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END
    

/* SST26 Flash Commands */
#define APP_SST26_CMD_ENABLE_RESET                      0x66
#define APP_SST26_CMD_MEMORY_RESET                      0x99
#define APP_SST26_CMD_STATUS_REG_READ                   0x05
#define APP_SST26_CMD_CONFIG_REG_READ                   0x35
#define APP_SST26_CMD_MEMORY_READ                       0x03
#define APP_SST26_CMD_MEMORY_HIGH_SPEED_READ            0x0B
#define APP_SST26_CMD_ENABLE_WRITE                      0x06
#define APP_SST26_CMD_DISABLE_WRITE                     0x04
#define APP_SST26_CMD_4KB_SECTOR_ERASE                  0x20
#define APP_SST26_CMD_BLOCK_ERASE                       0xD8
#define APP_SST26_CMD_CHIP_ERASE                        0xC7
#define APP_SST26_CMD_PAGE_PROGRAM                      0x02
#define APP_SST26_CMD_JEDEC_ID_READ                     0x9F
#define APP_SST26_CMD_GLOBAL_BLOCK_PROTECTION_UNLOCK    0x98

#define APP_SST26_STATUS_BIT_RES_0                      (0x01 << 0)
#define APP_SST26_STATUS_BIT_WEL                        (0x01 << 1)
#define APP_SST26_STATUS_BIT_WSE                        (0x01 << 2)
#define APP_SST26_STATUS_BIT_WSP                        (0x01 << 3)
#define APP_SST26_STATUS_BIT_WPLD                       (0x01 << 4)
#define APP_SST26_STATUS_BIT_SEC                        (0x01 << 5)
#define APP_SST26_STATUS_BIT_RES_6                      (0x01 << 6)
#define APP_SST26_STATUS_BIT_BUSY                       (0x01 << 7)

#define APP_SST26_PAGE_PROGRAM_SIZE_BYTES               256
#define APP_SST26_CS_ENABLE()                           SPI1_CS_Clear()
#define APP_SST26_CS_DISABLE()                          SPI1_CS_Set()

#define APP_SST26_MEM_ADDR                              0x10000

/* Application's state machine enum */
typedef enum {
    APP_SST26_STATE_INITIALIZE,
    APP_SST26_STATE_WAIT_MIN_POWER_UP_TIME,
    APP_SST26_STATE_RESET,
    APP_SST26_STATE_GLOBAL_BLK_PROTECTION_UNLOCK,
    APP_SST26_STATE_JEDEC_ID_READ,
    APP_SST26_STATE_SECTOR_ERASE,
    APP_SST26_STATE_READ_STATUS,
    APP_SST26_STATE_PAGE_PROGRAM,
    APP_SST26_STATE_MEMORY_READ,
    APP_SST26_STATE_VERIFY,
    APP_SST26_STATE_XFER_SUCCESSFUL,
    APP_SST26_STATE_XFER_ERROR,
    APP_SST26_STATE_IDLE,
} APP_SST26_STATES;

typedef struct {
    APP_SST26_STATES state;
    APP_SST26_STATES nextState;
    uint8_t transmitBuffer[APP_SST26_PAGE_PROGRAM_SIZE_BYTES + 5];
    uint8_t manufacturerID;
    uint16_t deviceID;
    uint8_t isCSDeAssert;
    volatile bool isTransferDone;
} APP_SST26_DATA;


void APP_SST26_Initialize ( void );
void APP_SST26_Tasks( void );


#endif /* _APP_SST26_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

