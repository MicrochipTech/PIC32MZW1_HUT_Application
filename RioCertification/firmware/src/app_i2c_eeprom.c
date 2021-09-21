/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_i2c_eeprom.c

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
// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app_i2c_eeprom.h"
#include "user.h"
#include <string.h>
#include "app_config.h"

#define APP_EEPROM_MEMORY_ADDR                      0x0000

APP_I2C_EEPROM_DATA appData;

void APP_I2C_EEPROM_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_I2C_EEPROM_STATE_INIT;
    app_controlData.moduleError.eI2CEEPROM=true;
#if APP_EEPROM_NUM_ADDR_BYTES == 2
    appData.txBuffer[0] = (APP_EEPROM_MEMORY_ADDR >> 8);
    appData.txBuffer[1] = APP_EEPROM_MEMORY_ADDR;
    memcpy(&appData.txBuffer[2], (const void*)APP_EEPROM_TEST_DATA, APP_EEPROM_TEST_DATA_SIZE);
#else
    appData.txBuffer[0] = APP_EEPROM_MEMORY_ADDR;
    memcpy(&appData.txBuffer[1], (const void*)APP_EEPROM_TEST_DATA, APP_EEPROM_TEST_DATA_SIZE);
#endif
}

void APP_I2C_EEPROM_Tasks ( void )
{
#ifndef I2C_APP_ENABLE
    return;
#else
    /* Check the application's current state. */
    switch(appData.state)
    {
        case APP_I2C_EEPROM_STATE_INIT:
            app_controlData.moduleError.eI2CEEPROM=true;
            /* Open I2C driver instance */
            appData.drvI2CHandle = DRV_I2C_Open( DRV_I2C_INDEX_0, DRV_IO_INTENT_READWRITE);

            if(appData.drvI2CHandle != DRV_HANDLE_INVALID)
            {
                PRINTF_CRIT_INFO(TERM_YELLOW"I2C_EEPROM:"TERM_RESET" Init Wait.\r\n");
                appData.state = APP_I2C_EEPROM_STATE_READY_WAIT;
            }
            else
            {
                PRINTF_CRIT_INFO("I2C_EEPROM: Init"TERM_RED" Error.\r\n"TERM_RESET);
                appData.state = APP_I2C_EEPROM_STATE_ERROR;
            }
            break;
            
        case APP_I2C_EEPROM_STATE_READY_WAIT:
            
            if (DRV_I2C_WriteTransfer( appData.drvI2CHandle, APP_EEPROM_SLAVE_ADDR, (void *)appData.txBuffer, 1) == true)
            {
                PRINTF_CRIT_INFO("I2C_EEPROM: Start write\r\n");
                appData.state = APP_I2C_EEPROM_STATE_WRITE;
            }
            else
            {
                //EEPROM is not ready. Keep checking until it is ready to receive commands.
                //Some EEPROMs need stabilization time before they can start responding to commands.
                PRINTF_CRIT_INFO("I2C_EEPROM: "TERM_YELLOW"Waiting for EEPROM\r\n"TERM_RESET);
            }            
            break;

        case APP_I2C_EEPROM_STATE_WRITE:

            /* Write data to EEPROM */
            if (DRV_I2C_WriteTransfer( appData.drvI2CHandle, APP_EEPROM_SLAVE_ADDR, (void *)appData.txBuffer, (APP_EEPROM_NUM_ADDR_BYTES + APP_EEPROM_TEST_DATA_SIZE)) == true)
            {
                /* Poll EEPROM busy status. EEPROM will NAK while internal write is in progress*/
                while (DRV_I2C_WriteTransfer( appData.drvI2CHandle, APP_EEPROM_SLAVE_ADDR, (void *)&appData.dummyData, 1 ) == false);
                PRINTF_CRIT_INFO("I2C_EEPROM: Start I2C process loop\r\n");
                appData.state = APP_I2C_EEPROM_STATE_READ;
            }
            else
            {
                appData.state = APP_I2C_EEPROM_STATE_ERROR;
            }
            break;

        case APP_I2C_EEPROM_STATE_READ:
            /* Read data from EEPROM */
            if (DRV_I2C_WriteReadTransfer(appData.drvI2CHandle, APP_EEPROM_SLAVE_ADDR, (void*)appData.txBuffer, APP_EEPROM_NUM_ADDR_BYTES, (void *)appData.rxBuffer, APP_EEPROM_TEST_DATA_SIZE) == true)
            {
                //printf("I2C_EEPROM: verify\r\n");
                appData.state = APP_I2C_EEPROM_STATE_VERIFY;
            }
            else
            {
                appData.state = APP_I2C_EEPROM_STATE_ERROR;
            }
            break;

        case APP_I2C_EEPROM_STATE_VERIFY:
            /* Compare the read data with the written data */
            if (memcmp(appData.rxBuffer, &appData.txBuffer[APP_EEPROM_NUM_ADDR_BYTES], APP_EEPROM_TEST_DATA_SIZE) == 0)
            {
                app_controlData.moduleError.eI2CEEPROM=false;
                PRINTF_CRIT_TRACE("I2C_EEPROM: Verify"TERM_GREEN" Success\r\n"TERM_RESET);
                appData.state = APP_I2C_EEPROM_STATE_SUCCESS;
            }
            else
            {
                PRINTF_CRIT_TRACE("I2C_EEPROM: Verify"TERM_RED" Error.\r\n"TERM_RESET);
                appData.state = APP_I2C_EEPROM_STATE_ERROR;
            }
#ifdef I2C_APP_LOOP
            appData.state = APP_I2C_EEPROM_STATE_READ;
#else
            DRV_I2C_Close(appData.drvI2CHandle);
#endif
            break;

        case APP_I2C_EEPROM_STATE_SUCCESS:
            app_controlData.moduleError.eI2CEEPROM=false;
            appData.state = APP_I2C_EEPROM_STATE_IDLE;
            break;

        case APP_I2C_EEPROM_STATE_ERROR:
            app_controlData.moduleError.eI2CEEPROM=true;
            //retry on error
            DRV_I2C_Close(appData.drvI2CHandle);
            appData.state=APP_I2C_EEPROM_STATE_INIT; 
            break;

        case APP_I2C_EEPROM_STATE_IDLE:
            vTaskSuspend(NULL);
            break;
    }
#endif //I2C_APP_ENABLE
}


/*******************************************************************************
 End of File
 */
