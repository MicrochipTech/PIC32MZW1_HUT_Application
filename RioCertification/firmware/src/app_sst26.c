/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

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


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <string.h>
#include "app_config.h"

APP_SST26_DATA appSST26Data;
uint8_t writeDataBuffer[APP_SST26_PAGE_PROGRAM_SIZE_BYTES];
uint8_t readDataBuffer[APP_SST26_PAGE_PROGRAM_SIZE_BYTES];

#define SPI_MANUFACTURER_ID 191
#define SPI_DEVICE_ID 9794

void APP_SST26_Reset(void) {
    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_ENABLE_RESET;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, 1);
    while (appSST26Data.isTransferDone == false);

    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_MEMORY_RESET;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, 1);

    while (appSST26Data.isTransferDone == false);
}

void APP_SST26_WriteEnable(void) {
    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_ENABLE_WRITE;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, 1);

    while (appSST26Data.isTransferDone == false);
}

void APP_SST26_WriteDisable(void) {
    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_DISABLE_WRITE;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, 1);

    while (appSST26Data.isTransferDone == false);
}

void APP_SST26_SectorErase(uint32_t address) {
    APP_SST26_WriteEnable();

    appSST26Data.isTransferDone = false;

    /* The address bits from A11:A0 are don't care and must be Vih or Vil */
    address = address & 0xFFFFF000;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_4KB_SECTOR_ERASE;
    appSST26Data.transmitBuffer[1] = (address >> 16);
    appSST26Data.transmitBuffer[2] = (address >> 8);
    appSST26Data.transmitBuffer[3] = address;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, 4);

    while (appSST26Data.isTransferDone == false);
}

void APP_SST26_ChipErase(void) {
    APP_SST26_WriteEnable();

    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_CHIP_ERASE;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, 1);

    while (appSST26Data.isTransferDone == false);
}

void APP_SST26_PageProgram(uint32_t address, uint8_t* pPageData) {
    uint32_t i;

    APP_SST26_WriteEnable();

    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_PAGE_PROGRAM;
    appSST26Data.transmitBuffer[1] = (address >> 16);
    appSST26Data.transmitBuffer[2] = (address >> 8);
    appSST26Data.transmitBuffer[3] = address;

    for (i = 0; i < APP_SST26_PAGE_PROGRAM_SIZE_BYTES; i++) {
        appSST26Data.transmitBuffer[4 + i] = pPageData[i];
    }

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, (4 + APP_SST26_PAGE_PROGRAM_SIZE_BYTES));

    while (appSST26Data.isTransferDone == false);
}

void APP_SST26_MemoryRead(uint32_t address, uint8_t* pReadBuffer, uint32_t nBytes, bool isHighSpeedRead) {
    uint8_t nTxBytes;

    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[1] = (address >> 16);
    appSST26Data.transmitBuffer[2] = (address >> 8);
    appSST26Data.transmitBuffer[3] = address;

    if (isHighSpeedRead == true) {
        appSST26Data.transmitBuffer[0] = APP_SST26_CMD_MEMORY_HIGH_SPEED_READ;
        /* For high speed read, perform a dummy write */
        appSST26Data.transmitBuffer[4] = 0xFF;
        nTxBytes = 5;
    } else {
        appSST26Data.transmitBuffer[0] = APP_SST26_CMD_MEMORY_READ;
        nTxBytes = 4;
    }

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = false;
    SPI1_Write(appSST26Data.transmitBuffer, nTxBytes);

    while (appSST26Data.isTransferDone == false);

    appSST26Data.isTransferDone = false;
    appSST26Data.isCSDeAssert = true;
    SPI1_Read(pReadBuffer, nBytes);

    while (appSST26Data.isTransferDone == false);
}

uint8_t APP_SST26_StatusRead(void) {
    uint8_t status;
    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_STATUS_REG_READ;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_WriteRead(appSST26Data.transmitBuffer, 1, appSST26Data.transmitBuffer, (1 + 1));

    while (appSST26Data.isTransferDone == false);

    status = appSST26Data.transmitBuffer[1];

    return status;
}

uint8_t APP_SST26_ConfigRegisterRead(void) {
    uint8_t config_reg;
    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_CONFIG_REG_READ;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_WriteRead(appSST26Data.transmitBuffer, 1, appSST26Data.transmitBuffer, (1 + 1));

    while (appSST26Data.isTransferDone == false);

    config_reg = appSST26Data.transmitBuffer[1];

    return config_reg;
}

void APP_SST26_JEDEC_ID_Read(uint8_t* manufacturerID, uint16_t* deviceID) {
    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_JEDEC_ID_READ;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_WriteRead(appSST26Data.transmitBuffer, 1, appSST26Data.transmitBuffer, (1 + 3));

    while (appSST26Data.isTransferDone == false);

    *manufacturerID = appSST26Data.transmitBuffer[1];
    *deviceID = (appSST26Data.transmitBuffer[2] << 8UL) | appSST26Data.transmitBuffer[3];
}

void APP_SST26_GlobalWriteProtectionUnlock(void) {
    APP_SST26_WriteEnable();

    appSST26Data.isTransferDone = false;
    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_GLOBAL_BLOCK_PROTECTION_UNLOCK;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, 1);

    while (appSST26Data.isTransferDone == false);
}

void APP_SST26_MinPowerOnDelay(void) {
    uint32_t i;

    /* Cheap delay. 
     * Based on the CPU frequency, ensure the delay is at-least 100 microseconds. 
     */
    for (i = 0; i < 100000; i++) {
        asm("NOP");
    }
}

/* This function will be called by SPI PLIB when transfer is completed */
void APP_SST26_SPIEventHandler(uintptr_t context) {
    uint8_t* isCSDeAssert = (uint8_t*) context;

    if (*isCSDeAssert == true) {
        /* De-assert the chip select */
        APP_SST26_CS_DISABLE();
    }

    appSST26Data.isTransferDone = true;
}

void APP_SST26_Initialize(void) {
    uint32_t i;

    APP_SST26_CS_DISABLE();
    
    appSST26Data.state = APP_SST26_STATE_INITIALIZE;
    app_controlData.moduleError.eSST26=false;
    /* Fill up the test data */
    for (i = 0; i < APP_SST26_PAGE_PROGRAM_SIZE_BYTES; i++) {
        writeDataBuffer[i] = i;
    }
}
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

void APP_SST26_Tasks(void) {
#ifndef SPI1_APP_ENABLE
    return;
#else
    uint8_t status;
    /* Check the application's current state. */
    switch (appSST26Data.state) {
        case APP_SST26_STATE_INITIALIZE:
            app_controlData.moduleError.eSST26=false;
            APP_SST26_Initialize();
            /* Register a callback with the SPI PLIB and pass a pointer to the isCSDeAssert variable as the context */
            SPI1_CallbackRegister(APP_SST26_SPIEventHandler, (uintptr_t) & appSST26Data.isCSDeAssert);
            PRINTF_CRIT_INFO(TERM_YELLOW"APP_SST26_Tasks(SPI1)"TERM_RESET": Flash init start\r\n");
            appSST26Data.state = APP_SST26_STATE_WAIT_MIN_POWER_UP_TIME;
            break;

        case APP_SST26_STATE_WAIT_MIN_POWER_UP_TIME:
            APP_SST26_MinPowerOnDelay();
            appSST26Data.state = APP_SST26_STATE_RESET;
            break;

        case APP_SST26_STATE_RESET:
            APP_SST26_Reset();
            //PRINTF_CRIT("SPI1_SST26: reset Done\r\n");
            appSST26Data.state = APP_SST26_STATE_GLOBAL_BLK_PROTECTION_UNLOCK;
            break;

        case APP_SST26_STATE_GLOBAL_BLK_PROTECTION_UNLOCK:
            APP_SST26_GlobalWriteProtectionUnlock();
            //PRINTF_CRIT("SPI1_SST26: ID Read\r\n");
            appSST26Data.state = APP_SST26_STATE_JEDEC_ID_READ;
            break;

        case APP_SST26_STATE_JEDEC_ID_READ:
            APP_SST26_JEDEC_ID_Read(&appSST26Data.manufacturerID, &appSST26Data.deviceID);
            if((SPI_MANUFACTURER_ID == appSST26Data.manufacturerID)&&(SPI_DEVICE_ID==appSST26Data.deviceID)){
                PRINTF_CRIT_INFO("SPI1_SST26: Erase\r\n");
                appSST26Data.state = APP_SST26_STATE_SECTOR_ERASE;
            }
            else{
                PRINTF_CRIT_INFO("SPI1_SST26: "TERM_YELLOW"Device not found\r\n"TERM_RESET);
#ifdef SPI1_APP_LOOP                
                appSST26Data.state = APP_SST26_STATE_WAIT_MIN_POWER_UP_TIME;
#else
                appSST26Data.state = APP_SST26_STATE_XFER_ERROR;
#endif
            }
            break;

        case APP_SST26_STATE_SECTOR_ERASE:
            APP_SST26_SectorErase(APP_SST26_MEM_ADDR);
            appSST26Data.state = APP_SST26_STATE_READ_STATUS;
            appSST26Data.nextState = APP_SST26_STATE_PAGE_PROGRAM;
            PRINTF_CRIT_INFO("SPI1_SST26: Write\r\n");
            break;

        case APP_SST26_STATE_READ_STATUS:
            status = APP_SST26_StatusRead();
            if ((status & APP_SST26_STATUS_BIT_BUSY) == 0) {
                appSST26Data.state = appSST26Data.nextState;
            }
            break;

        case APP_SST26_STATE_PAGE_PROGRAM:
            APP_SST26_PageProgram(APP_SST26_MEM_ADDR, &writeDataBuffer[0]);
            //printf("SPI1_SST26: Read\r\n");
            appSST26Data.state = APP_SST26_STATE_READ_STATUS;
            appSST26Data.nextState = APP_SST26_STATE_MEMORY_READ;
            break;

        case APP_SST26_STATE_MEMORY_READ:
            APP_SST26_MemoryRead(APP_SST26_MEM_ADDR, readDataBuffer, APP_SST26_PAGE_PROGRAM_SIZE_BYTES, false);
            //printf("SPI1_SST26s: Verify\r\n");
            appSST26Data.state = APP_SST26_STATE_VERIFY;
            break;

        case APP_SST26_STATE_VERIFY:
            if (memcmp(writeDataBuffer, readDataBuffer, APP_SST26_PAGE_PROGRAM_SIZE_BYTES) == 0) {
                PRINTF_CRIT_TRACE("SPI1_SST26: Verify"TERM_GREEN" Success\r\n"TERM_RESET);
                app_controlData.moduleError.eSST26=false;
                appSST26Data.state = APP_SST26_STATE_XFER_SUCCESSFUL;
            } else {
                PRINTF_CRIT_INFO("SPI1_SST26: Verify"TERM_RED" Fail\r\n"TERM_RESET);
                app_controlData.moduleError.eSST26=true;
                appSST26Data.state = APP_SST26_STATE_XFER_ERROR;
            }
#ifdef SPI1_APP_LOOP
            appSST26Data.state = APP_SST26_STATE_MEMORY_READ;
#endif
            break;

        case APP_SST26_STATE_XFER_SUCCESSFUL:
            appSST26Data.state = APP_SST26_STATE_IDLE;
            break;

        case APP_SST26_STATE_XFER_ERROR:
            app_controlData.moduleError.eSST26=true;
            appSST26Data.state = APP_SST26_STATE_IDLE;
            break;

        case APP_SST26_STATE_IDLE:
            break;

        default:
            break;
    }
#endif //SPI1_APP_ENABLE
}