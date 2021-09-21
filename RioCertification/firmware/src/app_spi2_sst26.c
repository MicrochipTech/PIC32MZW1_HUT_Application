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

#define SPI_MANUFACTURER_ID 191
#define SPI_DEVICE_ID 9794

APP_SST26_DATA appSPI2SST26Data;

#ifdef SPI2_APP_ENABLE
static uint8_t SPI2_writeDataBuffer[APP_SST26_PAGE_PROGRAM_SIZE_BYTES];
static uint8_t SPI2_readDataBuffer[APP_SST26_PAGE_PROGRAM_SIZE_BYTES];

static void APP_SST26_Reset(void) {
    appSPI2SST26Data.isTransferDone = false;

    appSPI2SST26Data.transmitBuffer[0] = APP_SST26_CMD_ENABLE_RESET;

    APP_SPI2_SST26_CS_ENABLE();
    appSPI2SST26Data.isCSDeAssert = true;
    SPI2_Write(appSPI2SST26Data.transmitBuffer, 1);
    while (appSPI2SST26Data.isTransferDone == false);

    appSPI2SST26Data.isTransferDone = false;

    appSPI2SST26Data.transmitBuffer[0] = APP_SST26_CMD_MEMORY_RESET;

    APP_SPI2_SST26_CS_ENABLE();
    appSPI2SST26Data.isCSDeAssert = true;
    SPI2_Write(appSPI2SST26Data.transmitBuffer, 1);

    while (appSPI2SST26Data.isTransferDone == false);
}

static void APP_SST26_WriteEnable(void) {
    appSPI2SST26Data.isTransferDone = false;

    appSPI2SST26Data.transmitBuffer[0] = APP_SST26_CMD_ENABLE_WRITE;

    APP_SPI2_SST26_CS_ENABLE();
    appSPI2SST26Data.isCSDeAssert = true;
    SPI2_Write(appSPI2SST26Data.transmitBuffer, 1);

    while (appSPI2SST26Data.isTransferDone == false);
}
#if 0

static void APP_SST26_WriteDisable(void) {
    appSPI2SST26Data.isTransferDone = false;

    appSPI2SST26Data.transmitBuffer[0] = APP_SST26_CMD_DISABLE_WRITE;

    APP_SPI2_SST26_CS_ENABLE();
    appSPI2SST26Data.isCSDeAssert = true;
    SPI2_Write(appSPI2SST26Data.transmitBuffer, 1);

    while (appSPI2SST26Data.isTransferDone == false);
}
#endif 

static void APP_SST26_SectorErase(uint32_t address) {
    APP_SST26_WriteEnable();

    appSPI2SST26Data.isTransferDone = false;

    /* The address bits from A11:A0 are don't care and must be Vih or Vil */
    address = address & 0xFFFFF000;

    appSPI2SST26Data.transmitBuffer[0] = APP_SST26_CMD_4KB_SECTOR_ERASE;
    appSPI2SST26Data.transmitBuffer[1] = (address >> 16);
    appSPI2SST26Data.transmitBuffer[2] = (address >> 8);
    appSPI2SST26Data.transmitBuffer[3] = address;

    APP_SPI2_SST26_CS_ENABLE();
    appSPI2SST26Data.isCSDeAssert = true;
    SPI2_Write(appSPI2SST26Data.transmitBuffer, 4);

    while (appSPI2SST26Data.isTransferDone == false);
}
#if 0

static void APP_SST26_ChipErase(void) {
    APP_SST26_WriteEnable();

    appSPI2SST26Data.isTransferDone = false;

    appSPI2SST26Data.transmitBuffer[0] = APP_SST26_CMD_CHIP_ERASE;

    APP_SPI2_SST26_CS_ENABLE();
    appSPI2SST26Data.isCSDeAssert = true;
    SPI2_Write(appSPI2SST26Data.transmitBuffer, 1);

    while (appSPI2SST26Data.isTransferDone == false);
}
#endif

static void APP_SST26_PageProgram(uint32_t address, uint8_t* pPageData) {
    uint32_t i;

    APP_SST26_WriteEnable();

    appSPI2SST26Data.isTransferDone = false;

    appSPI2SST26Data.transmitBuffer[0] = APP_SST26_CMD_PAGE_PROGRAM;
    appSPI2SST26Data.transmitBuffer[1] = (address >> 16);
    appSPI2SST26Data.transmitBuffer[2] = (address >> 8);
    appSPI2SST26Data.transmitBuffer[3] = address;

    for (i = 0; i < APP_SST26_PAGE_PROGRAM_SIZE_BYTES; i++) {
        appSPI2SST26Data.transmitBuffer[4 + i] = pPageData[i];
    }

    APP_SPI2_SST26_CS_ENABLE();
    appSPI2SST26Data.isCSDeAssert = true;
    SPI2_Write(appSPI2SST26Data.transmitBuffer, (4 + APP_SST26_PAGE_PROGRAM_SIZE_BYTES));

    while (appSPI2SST26Data.isTransferDone == false);
}

static void APP_SST26_MemoryRead(uint32_t address, uint8_t* pReadBuffer, uint32_t nBytes, bool isHighSpeedRead) {
    uint8_t nTxBytes;

    appSPI2SST26Data.isTransferDone = false;

    appSPI2SST26Data.transmitBuffer[1] = (address >> 16);
    appSPI2SST26Data.transmitBuffer[2] = (address >> 8);
    appSPI2SST26Data.transmitBuffer[3] = address;

    if (isHighSpeedRead == true) {
        appSPI2SST26Data.transmitBuffer[0] = APP_SST26_CMD_MEMORY_HIGH_SPEED_READ;
        /* For high speed read, perform a dummy write */
        appSPI2SST26Data.transmitBuffer[4] = 0xFF;
        nTxBytes = 5;
    } else {
        appSPI2SST26Data.transmitBuffer[0] = APP_SST26_CMD_MEMORY_READ;
        nTxBytes = 4;
    }

    APP_SPI2_SST26_CS_ENABLE();
    appSPI2SST26Data.isCSDeAssert = false;
    SPI2_Write(appSPI2SST26Data.transmitBuffer, nTxBytes);

    while (appSPI2SST26Data.isTransferDone == false);

    appSPI2SST26Data.isTransferDone = false;
    appSPI2SST26Data.isCSDeAssert = true;
    SPI2_Read(pReadBuffer, nBytes);

    while (appSPI2SST26Data.isTransferDone == false);
}

static uint8_t APP_SST26_StatusRead(void) {
    uint8_t status;
    appSPI2SST26Data.isTransferDone = false;

    appSPI2SST26Data.transmitBuffer[0] = APP_SST26_CMD_STATUS_REG_READ;

    APP_SPI2_SST26_CS_ENABLE();
    appSPI2SST26Data.isCSDeAssert = true;
    SPI2_WriteRead(appSPI2SST26Data.transmitBuffer, 1, appSPI2SST26Data.transmitBuffer, (1 + 1));

    while (appSPI2SST26Data.isTransferDone == false);

    status = appSPI2SST26Data.transmitBuffer[1];

    return status;
}
#if 0

static uint8_t APP_SST26_ConfigRegisterRead(void) {
    uint8_t config_reg;
    appSPI2SST26Data.isTransferDone = false;

    appSPI2SST26Data.transmitBuffer[0] = APP_SST26_CMD_CONFIG_REG_READ;

    APP_SPI2_SST26_CS_ENABLE();
    appSPI2SST26Data.isCSDeAssert = true;
    SPI2_WriteRead(appSPI2SST26Data.transmitBuffer, 1, appSPI2SST26Data.transmitBuffer, (1 + 1));

    while (appSPI2SST26Data.isTransferDone == false);

    config_reg = appSPI2SST26Data.transmitBuffer[1];

    return config_reg;
}
#endif 

static void APP_SST26_JEDEC_ID_Read(uint8_t* manufacturerID, uint16_t* deviceID) {
    appSPI2SST26Data.isTransferDone = false;

    appSPI2SST26Data.transmitBuffer[0] = APP_SST26_CMD_JEDEC_ID_READ;

    APP_SPI2_SST26_CS_ENABLE();
    appSPI2SST26Data.isCSDeAssert = true;
    SPI2_WriteRead(appSPI2SST26Data.transmitBuffer, 1, appSPI2SST26Data.transmitBuffer, (1 + 3));

    while (appSPI2SST26Data.isTransferDone == false);

    *manufacturerID = appSPI2SST26Data.transmitBuffer[1];
    *deviceID = (appSPI2SST26Data.transmitBuffer[2] << 8UL) | appSPI2SST26Data.transmitBuffer[3];
}

static void APP_SST26_GlobalWriteProtectionUnlock(void) {
    APP_SST26_WriteEnable();

    appSPI2SST26Data.isTransferDone = false;
    appSPI2SST26Data.transmitBuffer[0] = APP_SST26_CMD_GLOBAL_BLOCK_PROTECTION_UNLOCK;

    APP_SPI2_SST26_CS_ENABLE();
    appSPI2SST26Data.isCSDeAssert = true;
    SPI2_Write(appSPI2SST26Data.transmitBuffer, 1);

    while (appSPI2SST26Data.isTransferDone == false);
}

static void APP_SST26_MinPowerOnDelay(void) {
    uint32_t i;

    /* Cheap delay. 
     * Based on the CPU frequency, ensure the delay is at-least 100 microseconds. 
     */
    for (i = 0; i < 100000; i++) {
        asm("NOP");
    }
}

/* This function will be called by SPI PLIB when transfer is completed */
static void APP_SST26_SPIEventHandler(uintptr_t context) {
    uint8_t* isCSDeAssert = (uint8_t*) context;

    if (*isCSDeAssert == true) {
        /* De-assert the chip select */
        APP_SPI2_SST26_CS_DISABLE();
    }

    appSPI2SST26Data.isTransferDone = true;
}

#endif  //SPI2_APP_ENABLE

void APP_SPI2_SST26_Initialize(void) {
    APP_SPI2_SST26_CS_DISABLE();

    appSPI2SST26Data.state = APP_SPI2_SST26_STATE_INITIALIZE;
    app_controlData.moduleError.eSPI2FLASH=true;
}
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

void APP_SPI2_SST26_Tasks(void) {
#ifndef SPI2_APP_ENABLE
    return;
#else
    uint8_t status;
    /* Check the application's current state. */
    switch (appSPI2SST26Data.state) {
        case APP_SPI2_SST26_STATE_INITIALIZE:
            APP_SST26_Initialize();
            app_controlData.moduleError.eSPI2FLASH=true;
            /* Fill up the test data */
            uint32_t i;
            for (i = 0; i < APP_SST26_PAGE_PROGRAM_SIZE_BYTES; i++) {
                SPI2_writeDataBuffer[i] = i;
            }

            /* Register a callback with the SPI PLIB and pass a pointer to the isCSDeAssert variable as the context */
            SPI2_CallbackRegister(APP_SST26_SPIEventHandler, (uintptr_t) & appSPI2SST26Data.isCSDeAssert);
            PRINTF_CRIT_INFO(TERM_YELLOW"APP_SST26_Tasks(SPI2)"TERM_RESET": Flash init start\r\n");
            appSPI2SST26Data.state = APP_SPI2_SST26_STATE_WAIT_MIN_POWER_UP_TIME;
            break;

        case APP_SPI2_SST26_STATE_WAIT_MIN_POWER_UP_TIME:
            APP_SST26_MinPowerOnDelay();
            appSPI2SST26Data.state = APP_SPI2_SST26_STATE_RESET;
            break;

        case APP_SPI2_SST26_STATE_RESET:
            APP_SST26_Reset();
//            PRINTF_CRIT_NORM("SPI2_SST26: reset Done\r\n");
            appSPI2SST26Data.state = APP_SPI2_SST26_STATE_GLOBAL_BLK_PROTECTION_UNLOCK;
            break;

        case APP_SPI2_SST26_STATE_GLOBAL_BLK_PROTECTION_UNLOCK:
            APP_SST26_GlobalWriteProtectionUnlock();
//            PRINTF_CRIT_NORM("SPI2_SST26: ID Read\r\n");
            appSPI2SST26Data.state = APP_SPI2_SST26_STATE_JEDEC_ID_READ;
            break;

        case APP_SPI2_SST26_STATE_JEDEC_ID_READ:
            APP_SST26_JEDEC_ID_Read(&appSPI2SST26Data.manufacturerID, &appSPI2SST26Data.deviceID);
            if((SPI_MANUFACTURER_ID == appSPI2SST26Data.manufacturerID)&&(SPI_DEVICE_ID==appSPI2SST26Data.deviceID)){
                app_controlData.moduleError.eSPI2FLASH=false;
                PRINTF_CRIT_INFO("SPI2_SST26: Erase\r\n");
                appSPI2SST26Data.state = APP_SPI2_SST26_STATE_SECTOR_ERASE;
            }
            else{
                app_controlData.moduleError.eSPI2FLASH=true;
                PRINTF_CRIT_INFO("SPI2_SST26: "TERM_YELLOW"Device not found\r\n"TERM_RESET);
#ifdef SPI2_APP_LOOP
                appSPI2SST26Data.state = APP_SPI2_SST26_STATE_WAIT_MIN_POWER_UP_TIME;
#else
                appSPI2SST26Data.state = APP_SPI2_SST26_STATE_XFER_ERROR;
#endif
            }
            break;

        case APP_SPI2_SST26_STATE_SECTOR_ERASE:
            APP_SST26_SectorErase(APP_SST26_MEM_ADDR);
            appSPI2SST26Data.state = APP_SPI2_SST26_STATE_READ_STATUS;
            appSPI2SST26Data.nextState = APP_SPI2_SST26_STATE_PAGE_PROGRAM;
            PRINTF_CRIT_INFO("SPI2_SST26: Write\r\n");
            break;

        case APP_SPI2_SST26_STATE_READ_STATUS:
            status = APP_SST26_StatusRead();
            if ((status & APP_SST26_STATUS_BIT_BUSY) == 0) {
                appSPI2SST26Data.state = appSPI2SST26Data.nextState;
            }
            break;

        case APP_SPI2_SST26_STATE_PAGE_PROGRAM:
            APP_SST26_PageProgram(APP_SST26_MEM_ADDR, &SPI2_writeDataBuffer[0]);
            //printf("SPI2_SST26: Read\r\n");
            appSPI2SST26Data.state = APP_SPI2_SST26_STATE_READ_STATUS;
            appSPI2SST26Data.nextState = APP_SPI2_SST26_STATE_MEMORY_READ;
            break;

        case APP_SPI2_SST26_STATE_MEMORY_READ:
            APP_SST26_MemoryRead(APP_SST26_MEM_ADDR, SPI2_readDataBuffer, APP_SST26_PAGE_PROGRAM_SIZE_BYTES, false);
            //printf("SPI2_SST26s: Verify\r\n");
            appSPI2SST26Data.state = APP_SPI2_SST26_STATE_VERIFY;
            break;

        case APP_SPI2_SST26_STATE_VERIFY:
            if (memcmp(SPI2_writeDataBuffer, SPI2_readDataBuffer, APP_SST26_PAGE_PROGRAM_SIZE_BYTES) == 0) {
                PRINTF_CRIT_TRACE("SPI2_SST26: Verify"TERM_GREEN" Success\r\n"TERM_RESET);
                app_controlData.moduleError.eSPI2FLASH=false;
                appSPI2SST26Data.state = APP_SPI2_SST26_STATE_XFER_SUCCESSFUL;
            } else {
                PRINTF_CRIT_INFO("SPI2_SST26: Verify"TERM_RED" Fail\r\n"TERM_RESET);
                app_controlData.moduleError.eSPI2FLASH=true;
                appSPI2SST26Data.state = APP_SPI2_SST26_STATE_XFER_ERROR;
            }
#ifdef SPI2_APP_LOOP
            appSPI2SST26Data.state = APP_SPI2_SST26_STATE_MEMORY_READ;
#endif
            break;

        case APP_SPI2_SST26_STATE_XFER_SUCCESSFUL:
            appSPI2SST26Data.state = APP_SPI2_SST26_STATE_IDLE;
            break;

        case APP_SPI2_SST26_STATE_XFER_ERROR:
            app_controlData.moduleError.eSPI2FLASH=true;
            break;

        case APP_SPI2_SST26_STATE_IDLE:
            break;

        default:
            break;
    }
#endif //SPI2_APP_ENABLE
}