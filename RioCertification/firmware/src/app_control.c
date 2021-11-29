/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_control.c

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
#include "app_control.h"
#include "app_config.h"
#include "app_wifi.h"
#include "system/command/sys_command.h"
#include "system/debug/sys_debug.h"
#include <string.h>
#include <stdlib.h>

#define READ_WRITE_SIZE         (NVM_FLASH_PAGESIZE/2)
#define BUFFER_SIZE             (READ_WRITE_SIZE / sizeof(uint32_t))

//#define APP_FLASH_ADDRESS       (NVM_FLASH_START_ADDRESS + NVM_FLASH_SIZE)-(2*NVM_FLASH_PAGESIZE) //use last page
#define APP_FLASH_ADDRESS       0x900fE000 //2nd last page

#define CTRL_WLAN_SSID_MAX_LEN   256
#define CTRL_WLAN_PASS_MAX_LEN   256

#define CTRL_NVM_CONFIG_SIGNATURE   0xDEAFBEEF

#define APP_BOOLSTRING(X) X?TERM_RED"Fail"TERM_RESET:TERM_GREEN"Pass"TERM_RESET

APP_CONTROL_DATA app_controlData;
extern HUT_MODE hut_peripheral;

static void populate_buffer(uint32_t* writeData, const char* ssid, const char* pass, uint32_t authMode, char* regDomain) {
    CONFIG_DATA data;
    data.signature = CTRL_NVM_CONFIG_SIGNATURE;
    data.authMode = authMode;
    memcpy((void*) data.ssid, ssid, strlen(ssid) + 1);
    memcpy((void*) data.pass, pass, strlen(pass) + 1);
    memcpy((void*) data.regDom, regDomain, strlen(regDomain) + 1);

    memset(writeData, 0, BUFFER_SIZE); //Clear teh buffer first
    memcpy((void*) writeData, (const void*) &data, sizeof (data)); //copy data into the template buffer.
}

static volatile bool xferDone = false;

static void nvmEventHandler(uintptr_t context) {
    xferDone = true;
}

static void config_nvm_init(void) {
    NVM_CallbackRegister(nvmEventHandler, (uintptr_t) NULL);
    return;
}

static uint32_t gDataBuff[BUFFER_SIZE] CACHE_ALIGN;

static int nvmWriteConfig(char* ssid, char* pass, uint32_t authMode, char* regDomain) {

    uint32_t address = APP_FLASH_ADDRESS;
    uint8_t *writePtr = (uint8_t *) gDataBuff;
    uint32_t i = 0;

    while (NVM_IsBusy() == true);

    if (!NVM_PageErase(address)) {
        SYS_CONSOLE_PRINT("CTRL: "TERM_RED"Failed NVM erase @ %x \r\n"TERM_RESET, address);
    }
    while (xferDone == false);
    xferDone = false;
    populate_buffer(gDataBuff, ssid, pass, authMode, regDomain);

    for (i = 0; i < READ_WRITE_SIZE; i += NVM_FLASH_ROWSIZE) {
        /* Program a row of data */
        if (!NVM_RowWrite((uint32_t *) writePtr, address)) {
            SYS_CONSOLE_PRINT("CTRL: "TERM_RED"Failed NVM ROW write @ %x \r\n"TERM_RESET, address);
        }

        while (xferDone == false);

        xferDone = false;

        writePtr += NVM_FLASH_ROWSIZE;
        address += NVM_FLASH_ROWSIZE;
    }
    return 0;
}

static bool nvmReadConfig(CONFIG_DATA* configData) {
    NVM_Read(gDataBuff, sizeof (CONFIG_DATA), APP_FLASH_ADDRESS);

    CONFIG_DATA* cfg = (CONFIG_DATA*) gDataBuff;
    if (CTRL_NVM_CONFIG_SIGNATURE != cfg->signature) {
        return false;
    } else {
        memcpy((void*) configData, (void*) gDataBuff, sizeof (CONFIG_DATA));
        return true;
    }
}

static void getmacCmd(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    unsigned char count;
    char *param[16];
    unsigned char command[512];
    char temp = 0;
    
    memset(command, 0, sizeof(command));
    
    for (temp = 0; temp < argc; temp++)
    {
        strcpy(command + strlen(command), argv[temp]);
        strcpy(command + strlen(command), " ");
    }
	
    hut_application_input_cmd(command, strlen(command) + 1);
}

static void dumppktCmd(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    unsigned char count;
    char *param[16];
    unsigned char command[512];
    char temp = 0;
    
    memset(command, 0, sizeof(command));
    
    for (temp = 0; temp < argc; temp++)
    {
        strcpy(command + strlen(command), argv[temp]);
        strcpy(command + strlen(command), " ");
    }
	
    hut_application_input_cmd(command, strlen(command) - 1);
}

static void setmacCmd(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    unsigned char count;
    char *param[16];
    unsigned char command[512];
    char temp = 0;

    if (argc < 2) {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "usage: setmac <xx xx xx xx xx xx> <x>");
        return;
    }
    
    memset(command, 0, sizeof(command));
    
    for (temp = 0; temp < argc; temp++)
    {
        strcpy(command + strlen(command), argv[temp]);
        strcpy(command + strlen(command), " ");
    }
	
    hut_application_input_cmd(command, strlen(command) - 1);
}

#if 0
/*
* \brief Converts the input string consisting of hexadecimal digits into an integer value
*/ 
int xtoi(char *c)
{
  size_t szlen = strlen(c);
  int idx, ptr, factor,result =0;

  if(szlen > 0){
    if(szlen > 8) return 0;
    result = 0;
    factor = 1;

    for(idx = szlen-1; idx >= 0; --idx){
    if(isxdigit( *(c+idx))){
                if( *(c + idx) >= 97){
                  ptr = ( *(c + idx) - 97) + 10;
                }else if( *(c + idx) >= 65){
                  ptr = ( *(c + idx) - 65) + 10;
                }else{
                  ptr = *(c + idx) - 48;
                }
                result += (ptr * factor);
                factor *= 16;
    }else{
                                return 4;
    }
    }
  }

  return result;
}
#endif

uint8_t Parser_HexAsciiToInt(uint16_t hexAsciiLen, char* pInHexAscii, uint8_t* pOutInt)
{
    uint16_t rxHexAsciiLen = strlen(pInHexAscii);
    uint16_t iCtr = 0;
    uint16_t jCtr = rxHexAsciiLen >> 1;
    uint16_t index = 0;
    uint8_t retValue = 0;
    char tempBuff[3];

    if(rxHexAsciiLen % 2 == 0)
    {
        jCtr --;
    }

    if(hexAsciiLen == rxHexAsciiLen)
    {
        while(rxHexAsciiLen > 0)
        {
            if(rxHexAsciiLen >= 2U)
            {
                tempBuff[iCtr] = *(((char*)pInHexAscii) + (rxHexAsciiLen - 2));
                iCtr ++;
                tempBuff[iCtr] = *(((char*)pInHexAscii) + (rxHexAsciiLen - 1));

                rxHexAsciiLen -= 2U;
            }
            else
            {
                tempBuff[iCtr] = '0';
                iCtr ++;
                tempBuff[iCtr] = *(((char*)pInHexAscii) + (rxHexAsciiLen - 1));

                rxHexAsciiLen --;
            }

            iCtr ++;
            tempBuff[iCtr] = '\0';
//            *(pOutInt + jCtr) = xtoi(tempBuff); 
            *(pOutInt + index) = xtoi(tempBuff); 
            index++;
            iCtr = 0;
            jCtr --;
        }

        retValue = 1;
    }

    return retValue;
}

unsigned int iconfig_command_counter;
extern unsigned int rftm0_handler_counter;
extern unsigned int rfmac_handler_counter;
extern unsigned int hut_timer_handler_counter;
extern unsigned int print_start;

static void dumpmemCmd(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    unsigned char count;
    unsigned char command[512];
    unsigned int addr;
    unsigned int size;
    unsigned int temp_index = 0;
    
    if (argc < 2) {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "usage: dumpmem <addr> <size>");
        return;
    }
    
    size = atoi(argv[2]);
    
    if(size == 0)
    {
        SYS_CONSOLE_PRINT("%d %d %d\n", rfmac_handler_counter, 
                        rftm0_handler_counter, hut_timer_handler_counter);
        SYS_CONSOLE_PRINT("%d\n", iconfig_command_counter);
        print_start = 1;
        return;
    }
    else
        print_start = 0;
    
    addr = 0xBFC56000;
    SYS_CONSOLE_PRINT("%x\n", (*((unsigned int *)(addr))));
    SYS_CONSOLE_PRINT("%x\n", (*((unsigned int *)(0xBFC56008))));
    
    addr = 0;
    
    Parser_HexAsciiToInt(strlen(argv[1]), argv[1], (uint8_t *)&addr);
    
    for(temp_index = 0; temp_index < size; temp_index += 4)
    {
        SYS_CONSOLE_PRINT("addr: %x value: %x\n", addr + (temp_index), 
                            *((unsigned int *)(addr + temp_index)));
    }
}

static void iconfigCommands(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    unsigned char count;
    char *param[16];
    unsigned char command[512];
    char temp = 0;

    if (argc < 2) {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "usage: iconfig testname <param1> <param2> .... ");
        return;
    }
    
    memset(command, 0, sizeof(command));
    
    for (temp = 0; temp < argc; temp++)
    {
        strcpy(command + strlen(command), argv[temp]);
        strcpy(command + strlen(command), " ");
    }

    iconfig_command_counter++;    
    
	hut_peripheral = HUT_MODE_UART;
	
    hut_application_input_cmd(command, strlen(command) + 1);
}

static void appstatCommands(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    if (argc != 1) {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "usage: appstat>");
        return;
    }
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "Task Status: \r\n");
#ifdef ADCHS_APP_ENABLE
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tADCHS       : %s\r\n", APP_BOOLSTRING(app_controlData.moduleError.eADCHS));
#else
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tADCHS       : "TERM_CYAN"Skip"TERM_RESET"\r\n");
#endif
#ifdef ADCHS_APP_ENABLE
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tControl     : %s\r\n", APP_BOOLSTRING(app_controlData.moduleError.eCONTROL));
#else
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tControl       : "TERM_CYAN"Skip"TERM_RESET"\r\n");
#endif
#ifdef  ECC608_APP_ENABLE
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tI2C2 ECC608 : %s\r\n", APP_BOOLSTRING(app_controlData.moduleError.eECC608));
#else
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tI2C2 ECC608 : "TERM_CYAN"Skip"TERM_RESET"\r\n");
#endif    
#if defined(XPRJ_pic32mz_w1_freertos) || defined (XPRJ_pic32mz_w1_23_freertos)/*This is defined in project properties of the noETH project config.*/
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tETH         : %s\r\n", APP_BOOLSTRING(app_controlData.moduleError.eETH));
#endif
#ifdef I2C_APP_ENABLE    
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tI2C1 EEPROM : %s\r\n", APP_BOOLSTRING(app_controlData.moduleError.eI2CEEPROM));
#else
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tI2C1 EEPROM : "TERM_CYAN"Skip"TERM_RESET"\r\n");
#endif
#ifdef XPRJ_pic32mz_w1_wfiiot_freertos
    #ifdef I2C_SENSOR_APP_ENABLE    
        (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tI2C1 Sensors: %s\r\n", APP_BOOLSTRING(app_controlData.moduleError.eI2CEEPROM));
    #else
        (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tI2C1 Sensors: "TERM_CYAN"Skip"TERM_RESET"\r\n");
    #endif
#endif
#ifdef PWM_APP_ENABLE     
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tPWM         : %s\r\n", APP_BOOLSTRING(app_controlData.moduleError.ePWM));
#else
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tPWM         : "TERM_CYAN"Skip"TERM_RESET"\r\n");
#endif 
#ifdef RTCC_APP_ENABLE       
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tRTCC        : %s\r\n", APP_BOOLSTRING(app_controlData.moduleError.eRTCC));
#else
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tRTCC        : "TERM_CYAN"Skip"TERM_RESET"\r\n");
#endif    
#ifdef SPI2_APP_ENABLE       
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tSPI2 Flash  : %s\r\n", APP_BOOLSTRING(app_controlData.moduleError.eSPI2FLASH));
#else
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tSPI2 Flash  : "TERM_CYAN"Skip"TERM_RESET"\r\n"));
#endif        
#ifdef SPI1_APP_ENABLE     
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tSPI1 Flash  : %s\r\n", APP_BOOLSTRING(app_controlData.moduleError.eSST26));
#else
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tSPI1 Flash  : "TERM_CYAN"Skip"TERM_RESET"\r\n"));
#endif       
#ifdef USB_APP_ENABLE       
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tUSB         : %s\r\n", APP_BOOLSTRING(app_controlData.moduleError.eUSB));
#else
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tUSB         : "TERM_CYAN"Skip"TERM_RESET"\r\n"));
#endif       
    if (true == app_controlData.connectWiFi) {
        (*pCmdIO->pCmdApi->print)(cmdIoParam, "\tWiFi        : %s\r\n\r\n", APP_BOOLSTRING(app_controlData.moduleError.eWIFI));
    }

    return;
}

static void softReset(void) {
    bool int_flag = false;

    /*disable interrupts since we are going to do a sysKey unlock*/
    int_flag = (bool) __builtin_disable_interrupts();

    /* unlock system for clock configuration */
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    if (int_flag) {
        __builtin_mtc0(12, 0, (__builtin_mfc0(12, 0) | 0x0001)); /* enable interrupts */
    }

    RSWRSTbits.SWRST = 1;
    /*This read is what actually causes the reset*/
    RSWRST = RSWRSTbits.SWRST;

    /*Reference code. We will not hit this due to reset. This is here for reference.*/
    int_flag = (bool) __builtin_disable_interrupts();

    SYSKEY = 0x33333333;

    if (int_flag) /* if interrupts originally were enabled, re-enable them */ {
        __builtin_mtc0(12, 0, (__builtin_mfc0(12, 0) | 0x0001));
    }

}

static void resetCommands(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    if (argc != 1) {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "usage: softreset");
        return;
    }

    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "RESETTING!!!");
    softReset();
}

static const SYS_CMD_DESCRIPTOR ctrlCmdTbl[] = {
    {"appstat", appstatCommands, ": appstat"},
    {"iconfig", iconfigCommands, ": iconfig"},
    {"dumppkt", dumppktCmd, ": dumppkt"},
    {"setmac", setmacCmd, ": setmac"},
    {"getmac", getmacCmd, ": getmac"},
    {"dumpmem", dumpmemCmd, ": dumpmem"},
    {"sreset", resetCommands, ": sreset"},
};

void APP_CONTROL_Initialize(void) {
    if (!SYS_CMD_ADDGRP(ctrlCmdTbl, sizeof (ctrlCmdTbl) / sizeof (*ctrlCmdTbl), "app", ": application commands")) {
        SYS_CONSOLE_MESSAGE("CTRL: "TERM_RED"Failed adding commands\r\n"TERM_RESET);
    }
    app_controlData.wlanREGChange = false;
    app_controlData.wlanConfigChange = false;
    app_controlData.wlanConfigvalid = false;
    app_controlData.isWifiConnected = false;
    app_controlData.rssiData.assocHandle = (uintptr_t)NULL;
    app_controlData.rssiData.rssi = 0;
    if (/*SWITCH1_STATE_PRESSED*/0 == SWITCH1_Get()) {
        app_controlData.connectWiFi = false;
    } else {
        app_controlData.connectWiFi = true;
    }

    app_controlData.wlanConfig.signature = 0;

    /*this will be over written if a default is read from flash.*/
    memcpy(app_controlData.wlanConfig.regDom, WIFI_DEFAULT_REG_DOMAIN, strlen(WIFI_DEFAULT_REG_DOMAIN) + 1);

    app_controlData.state = APP_CONTROL_STATE_INIT;
    app_controlData.moduleError.eCONTROL = false;
}

void APP_CONTROL_Tasks(void) {
    switch (app_controlData.state) {
        case APP_CONTROL_STATE_INIT:
            config_nvm_init();

            if (nvmReadConfig(&app_controlData.wlanConfig)) {
                app_controlData.wlanConfigvalid = true;
                SYS_CONSOLE_PRINT(TERM_YELLOW"CTRL: WLAN Config read from NVM:\r\n"TERM_RESET);
            } else {
                SYS_CONSOLE_MESSAGE("CTRL: "TERM_YELLOW"No WLAN config in flash. Using default.\r\n"TERM_RESET);
            }
            app_controlData.state = APP_CONTROL_STATE_SERVICE_TASKS;
            break;
        case APP_CONTROL_STATE_SERVICE_TASKS:

            /*Feel free to puke*/
            if ((true == app_controlData.moduleError.eADCHS) ||
                    (true == app_controlData.moduleError.eCONTROL) ||
                    (true == app_controlData.moduleError.eECC608) ||
                    (true == app_controlData.moduleError.eETH) ||
                    (true == app_controlData.moduleError.eI2CEEPROM) ||
                    (true == app_controlData.moduleError.ePWM) ||
                    (true == app_controlData.moduleError.eRTCC) ||
                    (true == app_controlData.moduleError.eSPI2FLASH) ||
                    (true == app_controlData.moduleError.eSST26) ||
                    (true == app_controlData.moduleError.eUSB) ||
                    (true == app_controlData.moduleError.eWIFI)) {
                app_controlData.moduleErrorFlag = true;
            } else {
                app_controlData.moduleErrorFlag = false;
            }
            WDT_Clear();
            break;
        case APP_CONTROL_STATE_ERROR:
            app_controlData.moduleError.eCONTROL = true;
            break;
        default:
            break;
    }
}
