/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_adchs.c

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

#include "app_adchs.h"
#include "app_config.h"

APP_ADCHS_DATA app_adchsData;

#define ADC_VREF                (3.3f)
#define ADC_MAX_COUNT           (4095)

void APP_ADCHS_Initialize(void) {
    app_adchsData.state = APP_ADCHS_STATE_INIT;
    app_controlData.moduleError.eADCHS=false;
}

void APP_ADCHS_Tasks(void) {
#ifndef ADCHS_APP_ENABLE
    return;
#else
    static uint16_t adc_count;
    static float input_voltage;
    /* Check the application's current state. */
    switch (app_adchsData.state) {
        case APP_ADCHS_STATE_INIT:
            TMR3_Start();
            PRINTF_CRIT_INFO(TERM_YELLOW"ADC"TERM_RESET": Starting APP_ADCHS_Tasks\r\n");
            app_adchsData.state = APP_ADCHS_STATE_WAIT_READ;
            break;
            /* Application's initial state. */
        case APP_ADCHS_STATE_WAIT_READ:
            /* Wait till ADC conversion result is available */
            if (!ADCHS_ChannelResultIsReady(ADCHS_CH15))
                app_adchsData.state = APP_ADCHS_STATE_WAIT_READ;
            else
                app_adchsData.state = APP_ADCHS_STATE_READ;
            break;
        case APP_ADCHS_STATE_READ:
            adc_count = ADCHS_ChannelResultGet(ADCHS_CH15);
            input_voltage = (float) adc_count * ADC_VREF / ADC_MAX_COUNT;
            
            //avoid compilation issue when print is not defined. 
            (void)adc_count;
            (void)input_voltage;
            
            PRINTF_CRIT_TRACE("ADC: Count = "TERM_YELLOW"0x%03x"TERM_RESET", ADC Input Voltage = "TERM_YELLOW"%d.%02d V"TERM_RESET" \r\n", adc_count, (int) input_voltage, (int) ((input_voltage - (int) input_voltage)*100.0));

#ifdef ADCHS_APP_LOOP                
            app_adchsData.state = APP_ADCHS_STATE_WAIT_READ;
#else 
            app_adchsData.state = APP_ADCHS_STATE_DONE;
#endif
            break;
        case APP_ADCHS_STATE_DONE:
                /*One read done. */
            break;
        default:
            break;
    }
#endif //ADCHS_APP_ENABLE
}


/*******************************************************************************
 End of File
 */
