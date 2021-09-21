/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_sensors.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_SENSORS_Initialize" and "APP_SENSORS_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_SENSORS_STATES" definition).  Both
    are defined here for convenience.
 *******************************************************************************/

#ifndef _APP_SENSORS_H
#define _APP_SENSORS_H


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "config/pic32mz_w1_wfiiot_freertos/driver/driver_common.h"
#include "config/pic32mz_w1_wfiiot_freertos/driver/i2c/drv_i2c.h"
#include "config/pic32mz_w1_wfiiot_freertos/system/time/sys_time.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif

#define APP_SENSORS_READ_PERIOD_MS (1000)

    /* MCP9808 registers */
#define MCP9808_I2C_ADDRESS         0x18 
#define MCP9808_REG_CONFIG          0x01
#define MCP9808_REG_TAMBIENT		0x05
#define MCP9808_REG_MANUF_ID		0x06
#define MCP9808_REG_DEVICE_ID		0x07
#define MCP9808_REG_RESOLUTION		0x08

    /* MCP9808 other settings */
#define MCP9808_CONFIG_DEFAULT		0x00
#define MCP9808_CONFIG_SHUTDOWN		0x0100
#define MCP9808_RES_DEFAULT         62500
#define MCP9808_MANUF_ID            0x54
#define MCP9808_DEVICE_ID           0x0400
#define MCP9808_DEVICE_ID_MASK		0xff00

    /* OPT3001 registers */
#define OPT3001_I2C_ADDRESS             0x44
#define OPT3001_REG_RESULT              0x00
#define OPT3001_REG_CONFIG              0x01
#define OPT3001_REG_LOW_LIMIT           0x02
#define OPT3001_REG_HIGH_LIMIT          0x03
#define OPT3001_REG_MANUFACTURER_ID     0x7E
#define OPT3001_REG_DEVICE_ID           0x7F

    /* MCP9808 other settings */
#define OPT3001_CONFIG_SHUTDOWN             0x00
#define OPT3001_CONFIG_CONT_CONVERSION		0xCE10        //continuous convesrion
#define OPT3001_MANUF_ID                    0x5449
#define OPT3001_DEVICE_ID                   0x3001

    typedef enum {
        APP_SENSORS_TURN_ON_MCP9808 = 0,
        APP_SENSORS_WAIT_TURN_ON_MCP9808,
        APP_SENSORS_TURN_ON_OPT3001,
        APP_SENSORS_WAIT_TURN_ON_OPT3001,
        APP_SENSORS_READ_TEMP,
        APP_SENSORS_WAIT_READ_TEMP,
        APP_SENSORS_READ_LIGHT,
        APP_SENSORS_WAIT_READ_LIGHT,
        APP_SENSORS_WAIT_TURN,
        APP_SENSORS_IDLE,
#if 0
        APP_SENSORS_SHUTDOWN_MCP9808,
        APP_SENSORS_WAIT_SHUTDOWN_MCP9808,
        APP_SENSORS_SHUTDOWN_OPT3001,
        APP_SENSORS_WAIT_SHUTDOWN_OPT3001,
#endif
    } APP_SENSORS_STATES;

    /* I2C tranfser status */
    typedef enum {
        I2C_TRANSFER_STATUS_IN_PROGRESS,
        I2C_TRANSFER_STATUS_SUCCESS,
        I2C_TRANSFER_STATUS_ERROR,
        I2C_TRANSFER_STATUS_IDLE,
    } APP_CTRL_I2C_TRANSFER_STATUS;

    /* I2C */
    typedef struct {
        DRV_HANDLE i2cHandle;
        DRV_I2C_TRANSFER_HANDLE transferHandle;
        APP_CTRL_I2C_TRANSFER_STATUS transferStatus;
        uint8_t txBuffer[4];
        uint16_t rxBuffer;
    } APP_CTRL_I2C;

    /* MCP9808 structure */
    typedef struct {
        bool IsShutdown;
        int16_t temperature;
    } APP_CTRL_MCP9808;

    /* OPT3001 structure */
    typedef struct {
        bool IsShutdown;
        uint32_t light;
    } APP_CTRL_OPT3001;

    typedef struct {
        /* The application's current state */
        APP_SENSORS_STATES state;
        SYS_TIME_HANDLE timeHandle;
        bool readSensor;
        bool printLux;
        bool printTemp;
        APP_CTRL_I2C i2c;
        APP_CTRL_MCP9808 mcp9808;
        APP_CTRL_OPT3001 opt3001;
    } APP_SENSORS_DATA;

    void APP_SENSORS_Initialize(void);
    void APP_SENSORS_Tasks(void);

#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_SENSORS_H */

/*******************************************************************************
 End of File
 */

