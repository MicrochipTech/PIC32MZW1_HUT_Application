
#include "app_touch.h"
#include "system/console/sys_console.h"
#include "touch/touch_api_ptc.h"
#include "peripheral/gpio/plib_gpio.h"

extern volatile uint8_t measurement_done_touch;
static uint16_t position = 0u;

void APP_TOUCH_Initialize(void) {
    SYS_CONSOLE_PRINT("APP_TOUCH: Starting CVD task\r\n");
}

void APP_TOUCH_Tasks(void) {
    touch_process();
    if (measurement_done_touch == 1u) {
        measurement_done_touch = 0u;

        if ((get_sensor_state(0) & 0x80)||(get_sensor_state(1) & 0x80)) {
            GPIO_PinClear(GPIO_PIN_RB6); //GPIO_PIN_RB6);
        } else {
            GPIO_PinSet(GPIO_PIN_RB6); //GPIO_PIN_RB6);
        }

        if (get_scroller_state(0) == 0u) {
            GPIO_PinSet(GPIO_PIN_RA13); //GPIO_PIN_RA13);
            GPIO_PinSet(GPIO_PIN_RA15);
            GPIO_PinSet(GPIO_PIN_RB14);
            GPIO_PinSet(GPIO_PIN_RB10);
            GPIO_PinSet(GPIO_PIN_RA4);
            GPIO_PinSet(GPIO_PIN_RB9);
        } else {
            position = get_scroller_position(0);
            //printf("slider position = %u\n",position);

            if (position < 42) {
                GPIO_PinClear(GPIO_PIN_RA13); //GPIO_PIN_RA13);
                GPIO_PinSet(GPIO_PIN_RA15);
                GPIO_PinSet(GPIO_PIN_RB14);
                GPIO_PinSet(GPIO_PIN_RB10);
                GPIO_PinSet(GPIO_PIN_RA4);
                GPIO_PinSet(GPIO_PIN_RB9);
            } else if (position < 85) {
                GPIO_PinClear(GPIO_PIN_RA13); //GPIO_PIN_RA13);
                GPIO_PinClear(GPIO_PIN_RA15);
                GPIO_PinSet(GPIO_PIN_RB14);
                GPIO_PinSet(GPIO_PIN_RB10);
                GPIO_PinSet(GPIO_PIN_RA4);
                GPIO_PinSet(GPIO_PIN_RB9);
            } else if (position < 128) {
                GPIO_PinClear(GPIO_PIN_RA13); //GPIO_PIN_RA13);
                GPIO_PinClear(GPIO_PIN_RA15);
                GPIO_PinClear(GPIO_PIN_RB14);
                GPIO_PinSet(GPIO_PIN_RB10);
                GPIO_PinSet(GPIO_PIN_RA4);
                GPIO_PinSet(GPIO_PIN_RB9);
            } else if (position < 171) {
                GPIO_PinClear(GPIO_PIN_RA13); //GPIO_PIN_RA13);
                GPIO_PinClear(GPIO_PIN_RA15);
                GPIO_PinClear(GPIO_PIN_RB14);
                GPIO_PinClear(GPIO_PIN_RB10);
                GPIO_PinSet(GPIO_PIN_RA4);
                GPIO_PinSet(GPIO_PIN_RB9);
            } else if (position < 214) {
                GPIO_PinClear(GPIO_PIN_RA13); //GPIO_PIN_RA13);
                GPIO_PinClear(GPIO_PIN_RA15);
                GPIO_PinClear(GPIO_PIN_RB14);
                GPIO_PinClear(GPIO_PIN_RB10);
                GPIO_PinClear(GPIO_PIN_RA4);
                GPIO_PinSet(GPIO_PIN_RB9);
            } else if (position < 256) {
                GPIO_PinClear(GPIO_PIN_RA13); //GPIO_PIN_RA13);
                GPIO_PinClear(GPIO_PIN_RA15);
                GPIO_PinClear(GPIO_PIN_RB14);
                GPIO_PinClear(GPIO_PIN_RB10);
                GPIO_PinClear(GPIO_PIN_RA4);
                GPIO_PinClear(GPIO_PIN_RB9);
            }
        }
    }
}