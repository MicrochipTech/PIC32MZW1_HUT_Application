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

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "app_config.h"

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

#if !defined(MODE_DEV)
#define MODE_RELEASE    
#else
TODO( "!!!This is a dev build. Disable MODE_DEV for release!!!")
#endif

#define VERSION_NUMBER "vH3-2.3"
#define noETH_VERSION_TAG   "a"
#define FreeRTOS_23_VERSION_TAG   "b"
#define noETH23_VERSION_TAG   "c"
#define noETH_IoT_VERSION_TAG   "d"
#define noETH_refDesign_VERSION_TAG   "e"
#if defined(XPRJ_pic32mz_w1_noETH)
#define APP_VERSION VERSION_NUMBER noETH_VERSION_TAG
#elif defined(XPRJ_pic32mz_w1_23_freertos)
#define APP_VERSION VERSION_NUMBER FreeRTOS_23_VERSION_TAG
#elif defined(XPRJ_pic32mz_w1_23_noETH)
#define APP_VERSION VERSION_NUMBER noETH23_VERSION_TAG
#elif defined(XPRJ_pic32mz_w1_wfiiot_freertos)
#define APP_VERSION VERSION_NUMBER noETH_IoT_VERSION_TAG
#elif defined(XPRJ_pic32mz_w1_refDesign_freertos)
#define APP_VERSION VERSION_NUMBER noETH_refDesign_VERSION_TAG
#else
#define APP_VERSION VERSION_NUMBER
#endif

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );

/*Guarding me against my own stupidity!!*/
#if defined(MODE_DEV)
    /*This is to clear the programmer garbage*/
    printf(TERM_CTRL_RST TERM_CTRL_CLRSCR);
    SYS_CONSOLE_PRINT(TERM_CTRL_CLRSCR);
    SYS_CONSOLE_PRINT(TERM_BOLD TERM_RED "!!!If you are seeing this, ask developer for a new build\r\n"TERM_RESET);
#else
    SYS_CONSOLE_PRINT(TERM_CTRL_CLRSCR);
#endif
    
    SYS_CONSOLE_PRINT("\r\nStarting Certification Application ("TERM_CYAN"%s %s"TERM_RESET"): " \
                    TERM_BOLD TERM_BG_RED APP_VERSION TERM_RESET" \r\n",__DATE__, __TIME__);
    
    //print build properties related info. 
    /*This is defined in project properties of the noETH project config.*/
#if defined(XPRJ_pic32mz_w1_noETH) || defined (XPRJ_pic32mz_w1_23_noETH) ||\
    defined(XPRJ_pic32mz_w1_wfiiot_freertos) || defined(XPRJ_pic32mz_w1_refDesign_freertos)
    SYS_CONSOLE_PRINT(TERM_RED"Ethernet is not enabled\r\n"TERM_RESET);
/*
#else
    if(TCPIP_NETWORK_DEFAULT_MAC_ADDR_IDX0 == 0){
        SYS_CONSOLE_PRINT(TERM_RED"ETH has an invalid MAC address. No IP for you.\r\n"TERM_RESET);
    }
*/
#endif
    //SYS_CONSOLE_PRINT(TERM_RED"TPC is disabled\r\n"TERM_RESET);
    
    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

