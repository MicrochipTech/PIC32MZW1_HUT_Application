/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_rtcc.c

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

#include "app_rtcc.h"
#include "app_config.h"
#include "wdrv_pic32mzw_common.h"
#include "wdrv_pic32mzw_assoc.h"

APP_RTCC_DATA app_rtccData;

static volatile bool rtcc_alarm = false;

void RTCC_Callback(uintptr_t context) {
    rtcc_alarm = true;
}

#ifdef RTCC_APP_ENABLE  
static void _AssociationRSSICallback(DRV_HANDLE handle, WDRV_PIC32MZW_ASSOC_HANDLE assocHandle, int8_t rssi) {
    PRINTF_CRIT_INFO(TERM_YELLOW "Connected RSSI: %d \r\n" TERM_RESET, rssi);
    app_controlData.rssiData.rssi=rssi;
}
#endif

void APP_RTCC_Initialize(void) {
    app_rtccData.state = APP_RTCC_STATE_INIT;
    app_controlData.moduleError.eRTCC = false;
}

void APP_RTCC_Tasks(void) {
#ifndef RTCC_APP_ENABLE
    return;
#else
    switch (app_rtccData.state) {
            /* Application's initial state. */
        case APP_RTCC_STATE_INIT:
            app_controlData.moduleError.eRTCC = false;
#ifdef RTCC_APP_LOOP
            PRINTF_CRIT_INFO(TERM_YELLOW"RTCC"TERM_RESET": Alarm registered once in a 1s with Sosc\r\n");
#else
            PRINTF_CRIT_TRACE(TERM_YELLOW"RTCC"TERM_RESET": Alarm triggered once after 5 Seconds with  Sosc\r\n");
#endif
            struct tm sys_time;
            struct tm alarm_time;

            // Time setting 31-12-2019 23:59:58 Monday
            sys_time.tm_hour = 23;
            sys_time.tm_min = 59;
            sys_time.tm_sec = 58;

            sys_time.tm_year = 19;
            sys_time.tm_mon = 12;
            sys_time.tm_mday = 31;
            sys_time.tm_wday = 1;

            // Alarm setting 01-01-2020 00:00:05 Tuesday
            alarm_time.tm_hour = 00;
            alarm_time.tm_min = 00;
            alarm_time.tm_sec = 05;

            alarm_time.tm_year = 20;
            alarm_time.tm_mon = 01;
            alarm_time.tm_mday = 01;
            alarm_time.tm_wday = 2;

            RTCC_CallbackRegister(RTCC_Callback, (uintptr_t) NULL);

            if (RTCC_TimeSet(&sys_time) == false) {
                /* Error setting up time */
                PRINTF_CRIT_INFO("RTCC: "TERM_RED"Error setting time\r\n"TERM_RESET);
                app_rtccData.state = APP_RTCC_STATE_ERROR;
            }
            RTCC_ALARM_MASK mask;
#ifdef RTCC_APP_LOOP
            mask = RTCC_ALARM_MASK_SECOND;
#else
            mask = RTCC_ALARM_MASK_YEAR;
#endif
            if (RTCC_AlarmSet(&alarm_time, mask) == false) {
                /* Error setting up alarm */
                PRINTF_CRIT_INFO("RTCC: "TERM_RED"Error setting alarm\r\n"TERM_RESET);
                app_rtccData.state = APP_RTCC_STATE_ERROR;
            }
            PRINTF_CRIT_INFO("RTCC: Waiting for alarm\r\n");
            app_rtccData.state = APP_RTCC_STATE_WAIT_ALARM;
            break;
        case APP_RTCC_STATE_WAIT_ALARM:
            if (rtcc_alarm) {
                rtcc_alarm = false;
                RTCC_TimeGet(&sys_time);
                PRINTF_CRIT_INFO("RTCC: "TERM_GREEN"Alarm"TERM_YELLOW" %d-%d-%d %d:%d:%d\r\n"TERM_RESET, sys_time.tm_mday, sys_time.tm_mon, sys_time.tm_year, sys_time.tm_hour, sys_time.tm_min, sys_time.tm_sec);
                /*Get RSSI if connected*/
                if (app_controlData.isWifiConnected) {
//                    if (WDRV_PIC32MZW_STATUS_RETRY_REQUEST != WDRV_PIC32MZW_AssocRSSIGet(app_controlData.rssiData.assocHandle, NULL, _AssociationRSSICallback)) {
                        SYS_CONSOLE_PRINT("APP_RTCC: " TERM_RED "Failed getting RSSI\r\n" TERM_RESET);
                   // }
                }
            }
            break;
        case APP_RTCC_STATE_ERROR:
            app_controlData.moduleError.eRTCC = true;
            break;
        default:
            /* TODO: Handle error in application's state machine. */
            break;
    }
#endif //RTCC_APP_ENABLE
}
/*******************************************************************************
 End of File
 */
