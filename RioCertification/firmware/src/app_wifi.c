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
unsigned int print_start = 0;
unsigned char packet_dump[4000];
unsigned int packet_index = 0;
unsigned int temp_done = 0;

void dumppkt()
{
    int temp;
    
    SYS_CONSOLE_PRINT("%d %d ------\n", temp_done, packet_index);
    
    if (temp_done == 4000)
        return;
    
    for (temp = 0; temp < 100; temp++)
    {
        SYS_CONSOLE_PRINT("%x ", packet_dump[temp_done + temp]);
    }
    
    temp_done += 100;
}

void hut_application_packet(unsigned char *packet, int length)
{
    if ((packet_index + length) > 4000)
        return;
    
    memcpy(&packet_dump[packet_index], packet, length);
    packet_index += length;
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

int get_macaddress()
{
    static int otp_mac[2];
    otp_rec_t rec_mac_data, *rec_mac;
    rec_mac = otp_read_rec(OTP_REC_DEVICE_MAC_ID_TAG);
    if(rec_mac->status == OTP_ERROR_NOT_FOUND)
    {
        SYS_CONSOLE_PRINT("\n Device mac not programmed\n");    
    }
    else 
    {

        memcpy(otp_mac,rec_mac->dataptr,rec_mac->length);
        SYS_CONSOLE_PRINT("\n\rDevice mac Address:%02x:%02x:%02x:%02x:%02x:%02x\r\n",(otp_mac[0] & 0xFF), ((otp_mac[0] & 0x00FF00) >> 8), ((otp_mac[0] & 0x00FF0000) >> 16),
            ((otp_mac[0] & 0xFF000000) >> 24),(otp_mac[1] & 0xFF) , ((otp_mac[1] & 0x00FF00) >> 8)
            );
    }
    rec_mac = otp_read_rec(OTP_REC_ETHERNET_MAC_ID_TAG);
    if(rec_mac->status == OTP_ERROR_NOT_FOUND)
    {
        SYS_CONSOLE_PRINT("\n ethernet mac not programmed \n");
    }
    else 
    {
    memcpy(otp_mac,rec_mac->dataptr,rec_mac->length);
    //printf("otp mac[0]= %x, otp mac[1] = %x",otp_mac[0],otp_mac[1] );
        SYS_CONSOLE_PRINT("\n\rethernet mac Address:%02x:%02x:%02x:%02x:%02x:%02x\r\n",(otp_mac[0] & 0xFF), ((otp_mac[0] & 0x00FF00) >> 8), ((otp_mac[0] & 0x00FF0000) >> 16),
            ((otp_mac[0] & 0xFF000000) >> 24),(otp_mac[1] & 0xFF) , ((otp_mac[1] & 0x00FF00) >> 8)
              );
    }
    return 0;
}
void program_mac(char *mac_addr)
{
    otp_rec_t rec;
    SYS_CONSOLE_PRINT("  Writing device MAC :\n");
    rec.tag     = OTP_REC_DEVICE_MAC_ID_TAG;
    rec.length  = 8;
    rec.dataptr = mac_addr;
    otp_write_rec(&rec);
}
void program_mac_eth(char *mac_addr)
{
    otp_rec_t rec;
    SYS_CONSOLE_PRINT("  Writing ethernet MAC :\n");
    rec.tag     = OTP_REC_ETHERNET_MAC_ID_TAG;
    rec.length  = 8;
    rec.dataptr = mac_addr;
    otp_write_rec(&rec);
}

int set_macaddress(char *buf, unsigned int len)
{
    unsigned int spi_addr, spi_val , reg_val, delay;
    char *previous = buf;
    char **argvptr = cmd;
    int cmd_index = 0, argc, status;
    static int zero_counter;
    char mac_type = 0;
    char index;
    
    for (index = 0; index < 8; index++)
        cmd[index] = NULL;
    
    putstring("");
    SYS_CONSOLE_PRINT("\n set **MAC Address** %d \n", len);
    if(buf)
    {
        *argvptr = buf;
        argvptr++;
        argc = 1;
    }
    SYS_CONSOLE_PRINT("Len %d ", len);
    
    for(cmd_index =0; cmd_index < len; cmd_index++)
    {
        if((buf[cmd_index] == ' ')||(buf[cmd_index] == '\t')||(buf[cmd_index] == '\r'))
        {
            SYS_CONSOLE_PRINT("%x %d ", buf[cmd_index], cmd_index);
            buf[cmd_index] = '\0';
            previous = buf + (cmd_index + 1);
            *argvptr = previous;
            argvptr++;
        }
        while(*previous == ' ' || *previous == '\t'|| *previous == '\r')
        {
            SYS_CONSOLE_PRINT("Does it enter here\n");
            argvptr--; 
            previous++;
            cmd_index++;
            *argvptr = previous;
            if(*previous != ' ' || *previous != '\t')
                argvptr++;
        }
        if( *previous && argc != 30)
        {
            argc++;
        }
    }
    SYS_CONSOLE_PRINT("Reach here\n");
    {
        int counter=0;
        for(counter = 0; counter < 8; counter++)
            SYS_CONSOLE_PRINT("%d: %s %d\n", counter, cmd[counter], cmd[counter]);
    }
    
   if(argc < 2 )
   {
   	    SYS_CONSOLE_PRINT(" Wrong command \n");
        SYS_CONSOLE_PRINT("command is \"setmac <xx xx xx xx xx xx> <x>\" \n");
        return 0xFF;
   }
   else
        cmd_index = 0;
    if((cmd[1] == NULL) || (cmd[2] == NULL)){
        SYS_CONSOLE_PRINT(" Bad values for MAC address - Stop programming \n");
        SYS_CONSOLE_PRINT("command is \"setmac <xx xx xx xx xx xx> <x>\" \n");
        return 0xFF;
    }
    static int i;
    if(cmd[6] == NULL)
    {
        SYS_CONSOLE_PRINT("\n Incomplete MAC provided - Stop Programming\n");
        SYS_CONSOLE_PRINT("command is \"setmac <xx xx xx xx xx xx> <x>\" \n");
        return 0xFF;
    }
    
    if(cmd[7] != NULL)
    {
        mac_type = strtoul( cmd[7], 0, 16 );
        SYS_CONSOLE_PRINT("mac_type  %d ", mac_type);
        if(mac_type == 0)
        {
            SYS_CONSOLE_PRINT(" MAC  address for device \n");
        }
        else if(mac_type == 1)
        {
            SYS_CONSOLE_PRINT(" MAC  address for Ethernet \n");
        }
        else
        {
            printf("Wrong MAC type! \n");
            SYS_CONSOLE_PRINT("command is \"setmac <xx xx xx xx xx xx> <0 or 1 >\" \n");
            memset(buf, 0, len);
            return 0xFF;            
        }
#if 0        
        if(cmd[8] != NULL)
        {
            if(strtoul( cmd[8], 0, 16 ) != 0)
        {
            SYS_CONSOLE_PRINT(" Extra Arguments, Wrong command! \n");
            SYS_CONSOLE_PRINT("command is \"setmac <xx xx xx xx xx xx> <x>\" \n");
            memset(buf, 0, len);
            return 0xFF;
        }
    }
#endif        
    }    
    //mac_addr[0] = 77; /*ASCII - M*/
    //mac_addr[1] = 65; /*ASCII - A*/
    for (i = 0 ; i < 6; i++)
    {
        mac_addr[i] = strtoul( cmd[i + 1], 0, 16 );
        SYS_CONSOLE_PRINT("%x ", mac_addr[i]);
        if(mac_addr[i] == 0x0)
            zero_counter ++;
    }

    if(zero_counter > 5)
    {
        SYS_CONSOLE_PRINT("MAC value has multiple zeros - re-enter\n");
        zero_counter = 0;
        return 0xFF;
    }
    SYS_CONSOLE_PRINT("\n\rMAC Address:%02x:%02x:%02x:%02x:%02x:%02x\r\n", mac_addr[0]&0xFF,
            mac_addr[1]&0xFF, mac_addr[2]&0xFF, mac_addr[3]&0xFF, mac_addr[4]&0xFF, mac_addr[5]&0xFF );
    if(mac_type == 0)
    {
        SYS_CONSOLE_PRINT("Writing WiFi MAC");
        program_mac(mac_addr);
    }
    else if(mac_type == 1)
    {
        SYS_CONSOLE_PRINT("Writing Ethernet MAC");
        program_mac_eth(mac_addr);
    }
    mac_type = 0;
    memset(buf, 0, len);
    SYS_CONSOLE_PRINT("\n\rMAC Address programmed\r\n");
    return 0;
 }

void process_pmu_config(char *buf, unsigned int len)
{
    unsigned int spi_addr, spi_val , reg_val, delay;
    char *previous = buf;
    char **argvptr = cmd;
    int cmd_index = 0, argc, status;

    printf("\n **PMU Register Access** \n");
    
    if(buf)
    {
        *argvptr = buf;
        argvptr++;
        argc = 1;
    }
    
    for(cmd_index =0; cmd_index < len; cmd_index++)
    {
        if((buf[cmd_index] == ' ')||(buf[cmd_index] == '\t')||(buf[cmd_index] == '\r'))
        {
            buf[cmd_index] = '\0';
            previous = buf + (cmd_index + 1);
            /*Only store if we hit a delimiter*/
            *argvptr = previous;
            argvptr++;
        }
        while(*previous == ' ' || *previous == '\t'|| *previous == '\r')
        {
            /*Another delimiter, overwrite stored value*/
            argvptr--; 
            previous++;
            cmd_index++;
            *argvptr = previous;
            if(*previous != ' ' || *previous != '\t')
                argvptr++;
        }
        if( *previous && argc != 4 )
        {
            argc++;
        }
    }
   if(argc < 2 )
   {
   	    printf(" Wrong command \n");
        printf("command is \"pmu r/w addr value\" \n");
        return;
   }
   else
        cmd_index = 0;
    
    if((cmd[1] == NULL) || (cmd[2] == NULL)){
        printf(" Wrong command \n");
        printf("command is \"pmu r/w addr value\" \n");
        return;
    }
    spi_addr = strtoul( cmd[2], 0, 16 );
    
    if((*cmd[1]) == 'w')
    {
    if((cmd[3]))
        spi_val = strtoul( cmd[3], 0, 16 );
    else
    {
        printf(" Wrong command \n");
        printf("command is \"pmu r/w addr value\" \n");
        return;
    }
    printf("Writing PMU register addr = 0x%x , value = 0x%x\n", spi_addr, spi_val);  
    reg_val = (spi_addr << 16) | spi_val;
    *spi_cntrl_reg = reg_val;    
    while (1)
    {
        status = *spi_status_reg;
        if (status & 0x80)
            break;
    }  
    delay = 0x10000;
    while (delay--) ;
    }

    else if((*cmd[1]) == 'r')
    {
        printf("Reading PMU register addr = 0x%x \n", spi_addr); 
        reg_val = (1 << 24) | (spi_addr << 16) ;
        *spi_cntrl_reg = reg_val;
        
        while (1)
        {
            status = *spi_status_reg;
            if (status & 0x80)
            break;
        }
        
        spi_val = ((status & PMU_SPI_READ_MASK) >> 16) ; 
        printf("Value of PMU register @ addr = 0x%x is 0x%x\n", spi_addr, spi_val); 
    }
    else
        printf("command is \"pmu r/w addr value\" \n");
}

void process_cmd(void)
{
#if 1
    static char buf[10];
    static int i;
    for(i= 0; i < 6; i++)
    buf[i] = g_string[i];
    buf[i] = '\0';
    if(g_string[0] ==  'p' && g_string[1] == 'm' && g_string[2] == 'u')
        process_pmu_config(g_string, g_counter + 1);
    else if(strncmp(g_string,"dumppkt",7) == 0)
    {
        dumppkt();
    }
    else if(strncmp(buf,"setmac",6) == 0)
    {
        SYS_CONSOLE_PRINT( "\n \r Set mac address %d\n", g_counter );
        set_macaddress(g_string, g_counter);
    }
    else if(strncmp(buf,"getmac",6) == 0)
    {
        printf( "\n \rGet mac address\n" );
        get_macaddress();
    }
#ifdef MCHP_HUT_OTP_SIGNATURE    
    else if(strncmp(buf,"signOTP",6) == 0)
    {
        printf( "\n \rsignOTP\n" );
        signOTPRecords();
        printf("\ndone\n");
        
    } 
    else if(strncmp(buf,"verifyOTPSignature",6) == 0)
    {
        printf( "\n \rverifyOTPSignature\n" );
        verifyOTPRecords();
        printf("\ndone\n");
       
    }
#endif    
    else
#endif        
    {
        if (shell_cmd_parser(g_string, g_counter + 1))
        {
        	hut_application_msg("\nINVALID_CMD_FORMAT");
            printf( "%s: Wrong command\n", g_string );
        }
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
//        SYS_CONSOLE_PRINT("RIO-2 HUT - Version 230400 baud = 2.5.2A\n");
  //      SYS_CONSOLE_PRINT("\n----HUT code init done------------\n");
  //      SYS_CONSOLE_PRINT("\r\nRIO-2 HUT - Version 230400 baud = 2.5.2A");
        SYS_CONSOLE_PRINT("\r\n----HUT code init done------------\r\n");

        SYS_CONSOLE_PRINT("\r\n            PMU in %s mode                      \n",
            ((PMUCMODE & 0x80000000) >> 31) ? "BUCK-PWM" : "MLDO" );
        SYS_CONSOLE_PRINT("\r            Sys Clk - 200MHz                    \n");
        SYS_CONSOLE_PRINT("\r            Part is %s                          \r\n",
          ((((DEVID & 0x0FF00000) >> 20) == 0x8C) ? \
          (((DEVIDbits.VER) == 1) ? "SG402-A1":"SG402-A0") : "SG407-A0(B0)"));
        SYS_CONSOLE_PRINT("\n---------------------------------------------\r\n");
//        SYS_CONSOLE_PRINT("\r----HUT code init done------------\n");
        //SYS_CONSOLE_PRINT("[DEBUG] xtal LDO Bypass check reg 0x82=0x%x\n", wifi_spi_read(0x82));
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
