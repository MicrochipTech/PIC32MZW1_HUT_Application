/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_pwm.c

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

#include "app_pwm.h"
#include "app_config.h"
#include "app_control.h"

APP_PWM_DATA app_pwmData;

#define TIMER_PERIOD_10ms 3900
#define OCMP_CMP_VALUE TIMER_PERIOD_10ms
#define RAMP_INCREMENT 50

static volatile uint16_t gRValue = 0;
static volatile bool rampFlip = true;

void TMR_Callback_Fn(uint32_t status, uintptr_t context) {
    if (OCMP_CMP_VALUE == gRValue) rampFlip = false;
    else if (0 == gRValue) rampFlip = true;

    if (true == rampFlip) gRValue += RAMP_INCREMENT;
    else if (false == rampFlip) gRValue -= RAMP_INCREMENT;

    /*Switch off red LED if any module has reported an error.*/
    if(true==app_controlData.moduleErrorFlag){
#if defined(XPRJ_pic32mz_w1_wfiiot_freertos)
        OCMP1_CompareSecondaryValueSet(UINT16_MAX); //Active low
#else
        OCMP2_CompareSecondaryValueSet(0);
#endif
    }
    else{
#if defined(XPRJ_pic32mz_w1_wfiiot_freertos)
        OCMP1_CompareSecondaryValueSet(gRValue);
#else
        OCMP2_CompareSecondaryValueSet(gRValue);
#endif
    }
    
    /*Switch off Green LED if WiFi is off or disconnected*/
    if((false==app_controlData.connectWiFi)||(false==app_controlData.isWifiConnected)){
#if defined(XPRJ_pic32mz_w1_wfiiot_freertos)
        OCMP3_CompareSecondaryValueSet(UINT16_MAX);  
#else
        OCMP4_CompareSecondaryValueSet(0);  
#endif
    }
    else{
#if defined(XPRJ_pic32mz_w1_wfiiot_freertos)
        OCMP3_CompareSecondaryValueSet(gRValue);
#else
        OCMP4_CompareSecondaryValueSet(OCMP_CMP_VALUE - gRValue);
#endif
    }
}

void APP_PWM_Initialize(void) {
    app_pwmData.state = APP_PWM_STATE_INIT;
    app_controlData.moduleError.ePWM=false;
}

void APP_PWM_Tasks(void) {
#ifndef PWM_APP_ENABLE
    return ;
#else
    /* Check the application's current state. */
    switch (app_pwmData.state) {
            /* Application's initial state. */
        case APP_PWM_STATE_INIT:
        {
            PRINTF_CRIT_INFO(TERM_YELLOW"PWM app"TERM_RESET": Start\r\n");
            app_controlData.moduleError.eRTCC=false;
#if defined(XPRJ_pic32mz_w1_wfiiot_freertos)
            OCMP1_Enable();
            OCMP3_Enable();
#else
            OCMP2_Enable();
            OCMP4_Enable();
#endif
            
            
            TMR3_CallbackRegister(TMR_Callback_Fn, (uintptr_t)NULL);
            TMR3_Start();
            
            app_pwmData.state = APP_PWM_STATE_IDLE_TASKS;
            break;
        }
        case APP_PWM_STATE_IDLE_TASKS:
        {
            break;
        }
        default:
        {
            break;
        }
    }
#endif //PWM_APP_ENABLE
}


/*******************************************************************************
 End of File
 */
