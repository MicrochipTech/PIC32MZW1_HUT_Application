/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_wifi.c

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

#include <string.h>
#include "app_wifi.h"
#include "app_control.h"
#include "app_flash_otp.h"

#define HUT_MAJOR_VERSION           2
#define HUT_MINOR_VERSION           5
#define HUT_PATCH_VERSION           3
static SYS_TMR_HANDLE       wifi_timer_handle = SYS_TMR_HANDLE_INVALID;      // tick handle
#define WIFI_TIMER_TICK_RATE        1
extern unsigned int rftm0_handler_counter;
extern unsigned int hut_timer_handler_counter;
unsigned int wifi_timer_val;
unsigned int wifi_timer_start;

static void _WIFI_TickHandler(uintptr_t context)
{
    rftm0_handler_counter++;
    if(wifi_timer_start)
    {
        if (rftm0_handler_counter - wifi_timer_start > wifi_timer_val)
        {
            hut_timer_handler_counter++;
            //RealTimeTempVoltCal();
            wifi_timer_start = rftm0_handler_counter;
        }
    }
}

void APP_WIFI_Initialize_1(void) {
    SYS_CONSOLE_PRINT("APP_WIFI_Initialize_1\n");
    app_flash_otp_init();
    user_main();
    MAC_Interrupt_enable();
}

void APP_WIFI_Initialize(void) {
    APP_WIFI_Initialize_1();
}

#define SHELL_BUFFER_SIZE 500
#define PMU_SPI_READ_MASK 0xFFFF0000

static volatile unsigned char g_process_cmd = 0;
static int g_counter=0;
static char g_string[SHELL_BUFFER_SIZE] = {0};
char *cmd[8];
extern unsigned int *spi_status_reg;
extern unsigned int *spi_cntrl_reg;
extern unsigned int *pmu_cmode_reg;
static char mac_addr[8];

void startTimer(unsigned long timer)
{
    wifi_timer_start = rftm0_handler_counter;
    wifi_timer_val = timer;
}

void stopTimer(void)
{
    wifi_timer_val = 0;
    wifi_timer_start = 0;
    hut_timer_handler_counter = 0;
}

void hut_application_print(unsigned char *msg)
{
    SYS_CONSOLE_PRINT("%s ", msg);
}

void hut_application_print_2(unsigned char *msg, int param1, int param2)
{
    SYS_CONSOLE_PRINT("%s: %d %d ", msg, param1, param2);
}

void hut_application_print_1(unsigned char *msg, int param)
{
    SYS_CONSOLE_PRINT("%s: %d ", msg, param);
}

void hut_application_input_cmd(unsigned char *cmd, unsigned short length)
{
    memcpy(&g_string[g_counter], cmd, length);
    g_counter += length;
    g_process_cmd = true;
}

void sys_reset(void)
{
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    RSWRSTSET = _RSWRST_SWRST_MASK;
    RSWRST;
    Nop();
    Nop();
    Nop();
}

void process_cmd(void)
{
    static char buf[10];
    static int i;
    for(i= 0; i < 6; i++)
    buf[i] = g_string[i];
    buf[i] = '\0';

    if (shell_cmd_parser(g_string, g_counter + 1))
    {
      	hut_application_msg("\nINVALID_CMD_FORMAT");
    }
    memset(g_string, 0, sizeof(g_string));
    g_counter = 0;
}

void APP_WIFI_Tasks(void) {
    static int once = 0;
    
    if(once == 0)
    {
        if(0xFFFFFFFF == (*((unsigned int *)(DUMMY_FLASH_START))))
        {
            // read_cal_from_flash();
            write_cal_from_flash();  
            write_gain_to_flash();
        }
        SYS_CONSOLE_PRINT("\n---------------------------------------------\r\n");
        SYS_CONSOLE_PRINT("RIO-2 HUT - Version = %d.%d.%d       \n",
          HUT_MAJOR_VERSION, HUT_MINOR_VERSION,HUT_PATCH_VERSION);
        SYS_CONSOLE_PRINT("\r\n----HUT code init done------------\r\n");

        SYS_CONSOLE_PRINT("\r\n            PMU in %s mode                      \n",
            ((PMUCMODE & 0x80000000) >> 31) ? "BUCK-PWM" : "MLDO" );
        SYS_CONSOLE_PRINT("\r            Sys Clk - 200MHz                    \n");
        SYS_CONSOLE_PRINT("\r            Part is %s                          \r\n",
          ((((DEVID & 0x0FF00000) >> 20) == 0x8C) ? \
          (((DEVIDbits.VER) == 1) ? "SG402-A1":"SG402-A0") : "SG407-A0(B0)"));
        SYS_CONSOLE_PRINT("\n---------------------------------------------\r\n");
        SYS_CONSOLE_PRINT("Part is now in %s mode\r\n", ((PMUCMODE & 0x80000000) >> 31) ? "BUCK" : "MLDO" );
        wifi_timer_handle = SYS_TIME_CallbackRegisterMS(_WIFI_TickHandler, 0, 
                                WIFI_TIMER_TICK_RATE, SYS_TIME_PERIODIC);
        
        once = 1;
    }
    {
        if(g_process_cmd)
        {
            g_process_cmd = 0;
            process_cmd();
        }
    }        
}

void MAC_Timer0_enable(){
    SYS_CONSOLE_PRINT("MAC_TIMER0_ENABLE CHECK ************\n\n");
    IEC2bits.RFTM0IE = 1; //Enable Timer0 ISR
    //0xBF8100E0 
}
/****************************************************************************************
 * Function:        void MAC_Timer0_disable()
 *
 * Overview:        The function Disable WLAN Timer 0 ISR
****************************************************************************************/
void MAC_Timer0_disable(){
    SYS_CONSOLE_PRINT("MAC_TIMER0_DISABLE CHECK ************\n\n");
    IEC2bits.RFTM0IE = 0; //Disable Timer0 ISR
}
/****************************************************************************************
 * Function:        void MAC_Timer1_enable()
 *
 * Overview:        The function Enable WLAN Timer 1 ISR
****************************************************************************************/

void MAC_Timer1_enable(){
    SYS_CONSOLE_PRINT("MAC_TIMER1_ENABLE CHECK ************\n\n");
    //IEC2bits.RFTM1IE = 1; //Enable Timer1 ISR
}
/****************************************************************************************
 * Function:        void MAC_Timer1_enable()
 *
 * Overview:        The function Disable WLAN Timer 1 ISR
****************************************************************************************/
void MAC_Timer1_disable(){
    SYS_CONSOLE_PRINT("MAC_TIMER1_DISABLE CHECK ************\n\n");
//    IEC2bits.RFTM1IE = 0; //Disable Timer1 ISR
}

void MAC_Interrupt_enable(){
    IEC2bits.RFMACIE = 1; //Enable MAC ISR
}

void MAC_Interrupt_disable(){
    IEC2bits.RFMACIE = 0; //Enable MAC ISR
}

HUT_MODE hut_peripheral = 0;

void hut_application_msg(unsigned char *msg)
{
//    SYS_CONSOLE_PRINT("hut_application_msg: %d for peripheral %d\n", strlen(msg), hut_peripheral);
    if(hut_peripheral == HUT_MODE_UART)
    {
        SYS_CONSOLE_PRINT(msg);
    }
    else if (hut_peripheral == HUT_MODE_USB)
    {
        peripheral_application_msg_usb(msg);        
    }
    else if (hut_peripheral == HUT_MODE_ETH)
    {
        peripheral_application_msg_eth(msg);        
    }
    else
    {
        SYS_CONSOLE_PRINT(msg);        
    }
}

void wdrv_pic32mzw_mac_isr(unsigned int vector)
{
    mac_isr_hut(0);    
}

void wdrv_pic32mzw_timer_tick_isr(unsigned int param)
{
//    RealTimeTempVoltCal();
}

/* Placeholder dummy functions. */

void readFlash(uint32_t addressReadWrite, uint32_t bytesToRead, uint8_t *readBuffer){;}

void writeFlash(uint32_t addressReadWrite, uint32_t bytesToRead, uint8_t *readBuffer){
    
; 
}
void eraseFlash(uint32_t addressReadWrite){
    
;
}

void wdrv_pic32mzw_init(void)
{
    ;
}

uint8_t wdrv_pic32mzw_qmu_get_tx_count(void)
{
    ;
}

void wdrv_pic32mzw_user_stop(void)
{
    ;
}

void wdrv_pic32mzw_process_cfg_message(uint8_t* cfgmsg)
{
    ;
}

void wdrv_pic32mzw_wlan_send_packet(uint8_t* pBuf, uint16_t pktlen, uint32_t tos, uint8_t offset)
{
    ;
}

void wdrv_pic32mzw_user_main(void)
{
    ;
}

void wdrv_pic32mzw_mac_controller_task(void)
{
    ;
}

void wdrv_pic32mzw_smc_isr(unsigned int param)
{
    ;
}


/*******************************************************************************
 End of File
 */
