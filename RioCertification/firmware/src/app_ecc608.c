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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdio.h>
#include <stddef.h>      // Defines NULL
#include <stdbool.h>     // Defines true
#include <stdlib.h>      // Defines EXIT_FAILURE
#include "definitions.h" // SYS function prototypes
#include "cryptoauthlib.h"
#include "app_ecc608.h"
#include "app_config.h"

#ifdef ECC608_APP_ENABLE
extern ATCAIfaceCfg atecc608_0_init_data;
uint8_t sernum[9];
char displayStr[ATCA_SERIAL_NUM_SIZE * 3];
size_t displen = sizeof(displayStr);
#endif
APP_ECC608_DATA app_ecc608Data;

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

void APP_ECC608_Initialize(void)
{
    app_ecc608Data.state = APP_ECC608_STATE_INIT;
    app_controlData.moduleError.eECC608 = false;
}

void APP_ECC608_Tasks(void)
{
#ifndef ECC608_APP_ENABLE
    return;
#else
    ATCA_STATUS status;
    switch (app_ecc608Data.state)
    {
    case APP_ECC608_STATE_INIT:
        app_controlData.moduleError.eECC608 = false;
        PRINTF_CRIT_INFO(TERM_YELLOW "ECC608" TERM_YELLOW ": Start ECC608 Application \r\n");
        app_ecc608Data.state = APP_ECC608_STATE_ATCA_INIT;
        break;
    case APP_ECC608_STATE_ATCA_INIT:
        status = atcab_init(&atecc608_0_init_data);
        if (ATCA_SUCCESS == status)
        {
            app_controlData.moduleError.eECC608 = false;
            PRINTF_CRIT_TRACE(TERM_GREEN"ECC608: ATCA_INIT Done!\r\n"TERM_RESET);
            app_ecc608Data.state = APP_ECC608_STATE_READ_SERIAL;
        }
        else
        {
            atcab_release();
            app_controlData.moduleError.eECC608 = true;
            PRINTF_CRIT_INFO(TERM_RED"ECC608: ATCA_INIT fail\r\n"TERM_RESET);
            app_ecc608Data.state = APP_ECC608_STATE_ERROR;
        }
        break;
    case APP_ECC608_STATE_READ_SERIAL:
        status = atcab_read_serial_number(sernum);

        if (ATCA_SUCCESS == status)
        {
            app_controlData.moduleError.eECC608 = false;
            atcab_bin2hex(sernum, 9, displayStr, &displen);
            PRINTF_CRIT_TRACE("ECC608: "TERM_GREEN"Serial Number - %s\r\n\n"TERM_RESET, displayStr);
        }
        else
        {
            atcab_release();
            app_controlData.moduleError.eECC608 = true;
            PRINTF_CRIT_INFO(TERM_RED"ECC608: Serial read fail\r\n"TERM_RESET);
            app_ecc608Data.state = APP_ECC608_STATE_ERROR;
        }
#ifndef ECC608_APP_LOOP
            PRINTF_CRIT_INFO("ECC608:" TERM_GREEN " Task Done\r\n" TERM_RESET);
            app_ecc608Data.state = APP_ECC608_STATE_DONE;
#endif
        break;
    case APP_ECC608_STATE_DONE:
        break;
    case APP_ECC608_STATE_ERROR:
#ifdef ECC608_APP_LOOP
        app_ecc608Data.state = APP_ECC608_STATE_ATCA_INIT;
#endif
        break;
    default:
        break;
    }
#endif //ECC608_APP_ENABLE
}

/*******************************************************************************
 End of File
*/
