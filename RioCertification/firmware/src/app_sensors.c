/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_sensors.c

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

#include "app_sensors.h"
#include "driver/i2c/drv_i2c.h"
#include "system/console/sys_console.h"
#include "system/time/sys_time.h"
#include "app_config.h"
#include <math.h>
#include <string.h>


APP_SENSORS_DATA app_sensorsData;

#ifdef I2C_SENSOR_APP_ENABLE

/* I2C transfer callback */
static void i2cTransferCallback(DRV_I2C_TRANSFER_EVENT event,
        DRV_I2C_TRANSFER_HANDLE transferHandle,
        uintptr_t context) {
    switch (event) {
        case DRV_I2C_TRANSFER_EVENT_COMPLETE:
            app_sensorsData.i2c.transferStatus = I2C_TRANSFER_STATUS_SUCCESS;
            break;
        case DRV_I2C_TRANSFER_EVENT_ERROR:
            app_sensorsData.i2c.transferStatus = I2C_TRANSFER_STATUS_ERROR;
            break;
        default:
            break;
    }
}

/* I2C read */
static bool i2cReadReg(uint8_t addr, uint16_t reg, uint8_t size) {
    bool ret = false;
    app_sensorsData.i2c.transferHandle = DRV_I2C_TRANSFER_HANDLE_INVALID;
    app_sensorsData.i2c.txBuffer[0] = (uint8_t) reg;

    DRV_I2C_WriteReadTransferAdd(app_sensorsData.i2c.i2cHandle,
            addr,
            (void*) app_sensorsData.i2c.txBuffer, 1,
            (void*) &app_sensorsData.i2c.rxBuffer, size,
            &app_sensorsData.i2c.transferHandle);
    if (app_sensorsData.i2c.transferHandle == DRV_I2C_TRANSFER_HANDLE_INVALID) {
        PRINTF_CRIT_INFO(TERM_RED"APP_SENSORS: I2C read reg %x error \r\n"TERM_RESET, reg);
        app_controlData.moduleError.eI2CEEPROM = true;
        ret = false;
    } else
        ret = true;
    return ret;
}

/* I2C read complete */
static void i2cReadRegComp(uint8_t addr, uint8_t reg) {
    app_sensorsData.i2c.rxBuffer = (app_sensorsData.i2c.rxBuffer << 8) | (app_sensorsData.i2c.rxBuffer >> 8);
    PRINTF_CRIT_TRACE(TERM_YELLOW"I2C read complete - periph addr %x val %x\r\n"TERM_RESET, addr, app_sensorsData.i2c.rxBuffer);
    switch (addr) {
            /* MCP9808 */
        case MCP9808_I2C_ADDRESS:
            if (reg == MCP9808_REG_TAMBIENT) {
                uint8_t upperByte = (uint8_t) (app_sensorsData.i2c.rxBuffer >> 8);
                uint8_t lowerByte = ((uint8_t) (app_sensorsData.i2c.rxBuffer & 0x00FF));
                upperByte = upperByte & 0x1F;
                if ((upperByte & 0x10) == 0x10) { // Ta < 0 degC
                    upperByte = upperByte & 0x0F; // Clear sign bit
                    app_sensorsData.mcp9808.temperature = 256 - ((upperByte * 16) + lowerByte / 16);
                } else {
                    app_sensorsData.mcp9808.temperature = ((upperByte * 16) + lowerByte / 16);
                }
                if (true == app_sensorsData.printTemp) {
                    PRINTF_CRIT_INFO(TERM_GREEN"APP_SENSORS: MCP9808 Temperature %d (C)\r\n"TERM_RESET, app_sensorsData.mcp9808.temperature);
                    app_sensorsData.printTemp = false;
                }
                app_controlData.moduleError.eI2CEEPROM = false;
            }
            break;

            /* OPT3001 */
        case OPT3001_I2C_ADDRESS:
            if (reg == OPT3001_REG_RESULT) {
                uint16_t m = app_sensorsData.i2c.rxBuffer & 0x0FFF;
                uint16_t e = (app_sensorsData.i2c.rxBuffer & 0xF000) >> 12;
                app_sensorsData.opt3001.light = (m * pow(2, e)) / 100;
                if (true == app_sensorsData.printLux) {
                    PRINTF_CRIT_INFO(TERM_GREEN"APP_SENSORS: OPT3001 Light %d (lux)\r\n"TERM_RESET, app_sensorsData.opt3001.light);
                    app_sensorsData.printLux = false;
                }
                app_controlData.moduleError.eI2CEEPROM = false;
            }
            break;

        default:
            break;
    }
}

/* I2C write */
static bool i2cWriteReg(uint8_t addr, uint16_t reg, uint16_t val) {
    bool ret = false;
    app_sensorsData.i2c.transferHandle = DRV_I2C_TRANSFER_HANDLE_INVALID;
    app_sensorsData.i2c.txBuffer[0] = (uint8_t) reg;
    app_sensorsData.i2c.txBuffer[1] = (uint8_t) (val >> 8);
    app_sensorsData.i2c.txBuffer[2] = (uint8_t) (val & 0x00FF);

    DRV_I2C_WriteTransferAdd(app_sensorsData.i2c.i2cHandle,
            addr,
            (void*) app_sensorsData.i2c.txBuffer, 3,
            &app_sensorsData.i2c.transferHandle);
    if (app_sensorsData.i2c.transferHandle == DRV_I2C_TRANSFER_HANDLE_INVALID) {
        PRINTF_CRIT_INFO(TERM_RED"APP_SENSORS: I2C write reg %x error \r\n"TERM_RESET, reg);
        app_controlData.moduleError.eI2CEEPROM = true;
        ret = false;
    } else
        ret = true;

    return ret;
}

/* I2C write complete */
static void i2cWriteRegComp(uint8_t addr, uint8_t reg) {
    PRINTF_CRIT_TRACE(TERM_YELLOW"I2C write complete - periph addr %x\r\n"TERM_RESET, addr);
}

/* Sensors sub-module init */
static void sensorsInit() {
    /* Issue I2C read operation to get sensors readings*/
    app_sensorsData.readSensor = false;

    /*sensors structures*/
    memset(&app_sensorsData.mcp9808, 0, sizeof (app_sensorsData.mcp9808));
    memset(&app_sensorsData.opt3001, 0, sizeof (app_sensorsData.opt3001));

    /*I2C structure*/
    memset(&app_sensorsData.i2c, 0, sizeof (app_sensorsData.i2c));
    app_sensorsData.i2c.i2cHandle = DRV_I2C_TRANSFER_HANDLE_INVALID;
    app_sensorsData.i2c.transferHandle = DRV_I2C_TRANSFER_HANDLE_INVALID;
}

/* Read MCP9808 Temperature */
int16_t APP_SENSORS_readTemp(void) {
    return app_sensorsData.mcp9808.temperature;
}

/* Read OPT3001 Light */
uint32_t APP_SENSORS_readLight(void) {
    return app_sensorsData.opt3001.light;
}

static void timeCallback(uintptr_t param) {
    app_sensorsData.readSensor = true;
    //rate limit prints while in full throttle.
    app_sensorsData.printLux = true;
    app_sensorsData.printTemp = true;
}

#endif //I2C_SENSOR_APP_ENABLE

void APP_SENSORS_Initialize(void) {
#ifdef I2C_SENSOR_APP_ENABLE
    app_controlData.moduleError.eI2CEEPROM = true;
    sensorsInit();
    app_sensorsData.printLux = false;
    app_sensorsData.printTemp = false;
    /* Start a periodic timer to handle periodic events*/
    app_sensorsData.timeHandle = SYS_TIME_CallbackRegisterMS(timeCallback,
            (uintptr_t) 0,
            APP_SENSORS_READ_PERIOD_MS,
            SYS_TIME_PERIODIC);
    if (app_sensorsData.timeHandle == SYS_TIME_HANDLE_INVALID) {
        PRINTF_CRIT_INFO(TERM_RED"APP_SENSORS: Failed creating a periodic timer for sensors\r\n"TERM_RESET);
        app_controlData.moduleError.eI2CEEPROM = true;
        return;
    }

    /* Open I2C driver client */
    app_sensorsData.i2c.i2cHandle = DRV_I2C_Open(DRV_I2C_INDEX_0, DRV_IO_INTENT_READWRITE);
    if (app_sensorsData.i2c.i2cHandle == DRV_HANDLE_INVALID) {
        PRINTF_CRIT_INFO(TERM_RED"APP_SENSORS: Failed to open I2C driver for sensors reading\r\n"TERM_RESET);
        app_controlData.moduleError.eI2CEEPROM = true;
    } else {
        DRV_I2C_TransferEventHandlerSet(app_sensorsData.i2c.i2cHandle, i2cTransferCallback, 0);
    }
#endif
    app_sensorsData.state = APP_SENSORS_WAIT_TURN;
}

void APP_SENSORS_Tasks(void) {
#ifndef I2C_SENSOR_APP_ENABLE
    return;
#else
    switch (app_sensorsData.state) {
        case APP_SENSORS_WAIT_TURN:
#ifdef I2C_SENSOR_FULL_THROTTLE
            app_sensorsData.state = APP_SENSORS_TURN_ON_MCP9808;
            break;
#else
            if (true == app_sensorsData.readSensor) {
                app_sensorsData.readSensor = false;
                app_sensorsData.state = APP_SENSORS_TURN_ON_MCP9808;
            }
            break;
#endif
        case APP_SENSORS_TURN_ON_MCP9808:
        {
            app_sensorsData.i2c.transferStatus = I2C_TRANSFER_STATUS_IN_PROGRESS;
            if (i2cWriteReg(MCP9808_I2C_ADDRESS, MCP9808_REG_CONFIG, MCP9808_CONFIG_DEFAULT))
                app_sensorsData.state = APP_SENSORS_WAIT_TURN_ON_MCP9808;
            break;
        }
        case APP_SENSORS_WAIT_TURN_ON_MCP9808:
        {
            if (app_sensorsData.i2c.transferStatus == I2C_TRANSFER_STATUS_SUCCESS) {
                i2cWriteRegComp(MCP9808_I2C_ADDRESS, MCP9808_REG_CONFIG);
                app_sensorsData.state = APP_SENSORS_TURN_ON_OPT3001;
            } else if (app_sensorsData.i2c.transferStatus == I2C_TRANSFER_STATUS_ERROR) {
                PRINTF_CRIT_INFO(TERM_RED "APP_SENSORS: I2C write MCP9808_REG_CONFIG error \r\n"TERM_RESET);
                app_controlData.moduleError.eI2CEEPROM = true;
                app_sensorsData.state = APP_SENSORS_TURN_ON_OPT3001;
            }
        }
            break;
        case APP_SENSORS_TURN_ON_OPT3001:
        {
            app_sensorsData.i2c.transferStatus = I2C_TRANSFER_STATUS_IN_PROGRESS;
            if (i2cWriteReg(OPT3001_I2C_ADDRESS, OPT3001_REG_CONFIG, OPT3001_CONFIG_CONT_CONVERSION))
                app_sensorsData.state = APP_SENSORS_WAIT_TURN_ON_OPT3001;
            else
                app_sensorsData.state = APP_SENSORS_READ_TEMP;
            break;
        }
        case APP_SENSORS_WAIT_TURN_ON_OPT3001:
        {
            if (app_sensorsData.i2c.transferStatus == I2C_TRANSFER_STATUS_SUCCESS) {
                i2cWriteRegComp(OPT3001_I2C_ADDRESS, OPT3001_REG_CONFIG);
                app_sensorsData.state = APP_SENSORS_READ_TEMP;
            } else if (app_sensorsData.i2c.transferStatus == I2C_TRANSFER_STATUS_ERROR) {
                PRINTF_CRIT_INFO(TERM_RED "APP_SENSORS: I2C write OPT3001_REG_CONFIG error \r\n"TERM_RESET);
                app_controlData.moduleError.eI2CEEPROM = true;
                app_sensorsData.state = APP_SENSORS_READ_TEMP;
            }
            break;
        }
            /* MCP9808 read ambient temperature */
        case APP_SENSORS_READ_TEMP:
        {
            /* Schedule MCP9808 temperature reading */
            app_sensorsData.i2c.transferStatus = I2C_TRANSFER_STATUS_IN_PROGRESS;
            if (i2cReadReg(MCP9808_I2C_ADDRESS, MCP9808_REG_TAMBIENT, 2))
                app_sensorsData.state = APP_SENSORS_WAIT_READ_TEMP;
            else
                app_sensorsData.state = APP_SENSORS_READ_LIGHT;
            break;
        }
            /* MCP9808 wait for read ambient temperature */
        case APP_SENSORS_WAIT_READ_TEMP:
        {
            /* MCP9808 Temperature reading operation done */
            if (app_sensorsData.i2c.transferStatus == I2C_TRANSFER_STATUS_SUCCESS) {
                i2cReadRegComp(MCP9808_I2C_ADDRESS, MCP9808_REG_TAMBIENT);
                app_sensorsData.state = APP_SENSORS_READ_LIGHT;
            } else if (app_sensorsData.i2c.transferStatus == I2C_TRANSFER_STATUS_ERROR) {
                PRINTF_CRIT_INFO(TERM_RED"APP_SENSORS: I2C read temperature error \r\n"TERM_RESET);
                app_controlData.moduleError.eI2CEEPROM = true;
                app_sensorsData.state = APP_SENSORS_READ_LIGHT;
            }
            break;
        }
            /* OPT3001 read ambient light */
        case APP_SENSORS_READ_LIGHT:
        {
            /* Schedule OPT3001 light reading */
            app_sensorsData.i2c.transferStatus = I2C_TRANSFER_STATUS_IN_PROGRESS;
            if (i2cReadReg(OPT3001_I2C_ADDRESS, OPT3001_REG_RESULT, 2))
                app_sensorsData.state = APP_SENSORS_WAIT_READ_LIGHT;
            else
                app_sensorsData.state = APP_SENSORS_WAIT_TURN;
            break;
        }
            /* OPT3001 wait for read ambient light */
        case APP_SENSORS_WAIT_READ_LIGHT:
        {
            /* OPT3001 Light reading operation done */
            if (app_sensorsData.i2c.transferStatus == I2C_TRANSFER_STATUS_SUCCESS) {
                i2cReadRegComp(OPT3001_I2C_ADDRESS, OPT3001_REG_RESULT);
#ifdef I2C_SENSOR_APP_LOOP
                app_sensorsData.state = APP_SENSORS_WAIT_TURN;
#else
                app_sensorsData.state = APP_SENSORS_IDLE;
#endif             
            } else if (app_sensorsData.i2c.transferStatus == I2C_TRANSFER_STATUS_ERROR) {
                PRINTF_CRIT_INFO(TERM_RED"APP_SENSORS: I2C read light error \r\n"TERM_RESET);
                app_controlData.moduleError.eI2CEEPROM = true;
#ifdef I2C_SENSOR_APP_LOOP
                app_sensorsData.state = APP_SENSORS_WAIT_TURN;
#else
                app_sensorsData.state = APP_SENSORS_IDLE;
#endif             
            }
            break;
        }
#if 0
            /* MCP9808 shutdown (to save power)*/
        case APP_SENSORS_SHUTDOWN_MCP9808:
        {
            app_sensorsData.i2c.transferStatus = I2C_TRANSFER_STATUS_IN_PROGRESS;
            if (i2cWriteReg(MCP9808_I2C_ADDRESS, MCP9808_REG_CONFIG, MCP9808_CONFIG_SHUTDOWN))
                app_sensorsData.state = APP_SENSORS_WAIT_SHUTDOWN_MCP9808;
            else
                app_sensorsData.state = APP_SENSORS_SHUTDOWN_OPT3001;
            break;
        }
            /* MCP9808 wait for shutdown */
        case APP_SENSORS_WAIT_SHUTDOWN_MCP9808:
        {
            if (app_sensorsData.i2c.transferStatus == I2C_TRANSFER_STATUS_SUCCESS) {
                i2cWriteRegComp(MCP9808_I2C_ADDRESS, MCP9808_REG_CONFIG);
                app_sensorsData.state = APP_SENSORS_SHUTDOWN_OPT3001;
            } else if (app_sensorsData.i2c.transferStatus == I2C_TRANSFER_STATUS_ERROR) {
                PRINTF_CRIT_INFO(TERM_RED"APP_SENSORS: I2C write MCP9808_REG_CONFIG error \r\n"TERM_RESET);
                app_sensorsData.state = APP_SENSORS_SHUTDOWN_OPT3001;
            }
            break;
        }
            /* OPT3001 shutdown (to save power)*/
        case APP_SENSORS_SHUTDOWN_OPT3001:
        {
            app_sensorsData.i2c.transferStatus = I2C_TRANSFER_STATUS_IN_PROGRESS;
            if (i2cWriteReg(OPT3001_I2C_ADDRESS, OPT3001_REG_CONFIG, OPT3001_CONFIG_SHUTDOWN))
                app_sensorsData.state = APP_SENSORS_WAIT_SHUTDOWN_OPT3001;
            else
                app_sensorsData.state = APP_SENSORS_WAIT_TURN;
            break;
        }
            /* OPT3001 wait for shutdown */
        case APP_SENSORS_WAIT_SHUTDOWN_OPT3001:
        {
            if (app_sensorsData.i2c.transferStatus == I2C_TRANSFER_STATUS_SUCCESS) {
                i2cWriteRegComp(OPT3001_I2C_ADDRESS, OPT3001_REG_CONFIG);
#ifdef I2C_SENSOR_APP_LOOP
                app_sensorsData.state = APP_SENSORS_WAIT_TURN;
#else
                app_sensorsData.state = APP_SENSORS_IDLE;
#endif            
            } else if (app_sensorsData.i2c.transferStatus == I2C_TRANSFER_STATUS_ERROR) {
                PRINTF_CRIT_INFO(TERM_RED"I2C write OPT3001_REG_CONFIG error \r\n"TERM_RESET);
#ifdef I2C_SENSOR_APP_LOOP
                app_sensorsData.state = APP_SENSORS_WAIT_TURN;
#else
                app_sensorsData.state = APP_SENSORS_IDLE;
#endif            
            }
        }
#endif
            break;
        case APP_SENSORS_IDLE:
            break;
        default:
        {
            break;
        }

#endif //I2C_SENSOR_APP_ENABLE
    }
}