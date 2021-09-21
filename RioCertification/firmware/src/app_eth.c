/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_eth.c

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

#include "app_eth.h"
#include "definitions.h"

extern unsigned int hut_peripheral;

APP_ETH_DATA app_ethData;

void APP_ETH_Initialize(void) {
    app_ethData.state = APP_ETH_STATE_INIT;
    app_controlData.moduleError.eETH=false;    
}

void peripheral_application_msg_eth(unsigned char *msg)
{
    SYS_CONSOLE_PRINT("State %d Msg %s\n", app_ethData.state, msg);
    
    memset(app_ethData.cmdResponse, 0, sizeof(app_ethData.cmdResponse));
    if (strlen(msg) < sizeof(app_ethData.cmdResponse))
    {
        memcpy(app_ethData.cmdResponse, msg, strlen(msg));
    }
    else
        memcpy(app_ethData.cmdResponse, msg, sizeof(app_ethData.cmdResponse));

    if(TCPIP_UDP_PutIsReady(app_ethData.tSkt))
    {
        TCPIP_UDP_ArrayPut(app_ethData.tSkt, app_ethData.cmdResponse, strlen(msg));
        TCPIP_UDP_Flush(app_ethData.tSkt);
    }
    app_ethData.state = ETH_APP_PORT_OPEN;
}

void APP_ETH_OpenSockets(void) {
    IP_MULTI_ADDRESS		serverIpAddr;
    
    TCPIP_Helper_StringToIPAddress(TCPIP_HUT_REMOTE_IP_ADDR, &serverIpAddr.v4Add);
    app_ethData.tSkt = TCPIP_UDP_ClientOpen(IP_ADDRESS_TYPE_IPV4, TCPIP_HUT_REMOTE_SERVER_PORT, 0);
    TCPIP_UDP_Bind(app_ethData.tSkt, IP_ADDRESS_TYPE_ANY, 0, 0);
    TCPIP_UDP_DestinationIPAddressSet(app_ethData.tSkt, IP_ADDRESS_TYPE_IPV4, &serverIpAddr.v4Add);
    TCPIP_UDP_DestinationPortSet(app_ethData.tSkt, TCPIP_HUT_REMOTE_SERVER_PORT);

    SYS_CONSOLE_PRINT("Opening UDP Socket on %d for receiving HUT command\n", TCPIP_HUT_LOCAL_SERVER_PORT);
        
    app_ethData.uSkt = TCPIP_UDP_ServerOpen(IP_ADDRESS_TYPE_IPV4, TCPIP_HUT_LOCAL_SERVER_PORT,  0);
}

void APP_ETH_Tasks(void) {

    SYS_STATUS tcpipStat;
    const char *netName, *netBiosName;
    static IPV4_ADDR dwLastIP[2] = {
        {-1},
        {-1}
    };
    IPV4_ADDR ipAddr;
    TCPIP_NET_HANDLE netH;
    int i, nNets;
    unsigned int buffsize = 0;

    /* Check the application's current state. */
    switch (app_ethData.state) {
            /* Application's initial state. */
        case APP_ETH_STATE_INIT:
            tcpipStat = TCPIP_STACK_Status(sysObj.tcpip);
            if (tcpipStat < 0) { // some error occurred
                SYS_CONSOLE_MESSAGE(TERM_YELLOW"ETH_APP"TERM_RESET": TCP/IP stack initialization failed!\r\n");
                app_ethData.state = ETH_APP_TCPIP_ERROR;
            } else if (tcpipStat == SYS_STATUS_READY) {
                // now that the stack is ready we can check the
                // available interfaces
                nNets = TCPIP_STACK_NumberOfNetworksGet();
                for (i = 0; i < nNets; i++) {
                    netH = TCPIP_STACK_IndexToNet(i);
                    netName = TCPIP_STACK_NetNameGet(netH);
                    netBiosName = TCPIP_STACK_NetBIOSName(netH);

#if defined(TCPIP_STACK_USE_NBNS)
                    SYS_CONSOLE_PRINT("    Interface %s on host %s - NBNS enabled\r\n", netName, netBiosName);
#else
                    SYS_CONSOLE_PRINT("    Interface %s on host %s - NBNS disabled\r\n", netName, netBiosName);
#endif  // defined(TCPIP_STACK_USE_NBNS)
                    (void) netName; // avoid compiler warning 
                    (void) netBiosName; // if SYS_CONSOLE_PRINT is null macro

                }
                app_ethData.state = ETH_APP_TCPIP_WAIT_FOR_IP;
            }
            break;

        case ETH_APP_TCPIP_WAIT_FOR_IP:

            // if the IP address of an interface has changed
            // display the new value on the system console
            nNets = TCPIP_STACK_NumberOfNetworksGet();

            for (i = 0; i < nNets; i++) {
                netH = TCPIP_STACK_IndexToNet(i);
                if (!TCPIP_STACK_NetIsReady(netH)) {
                    continue; // interface not ready yet! , 
                    //looking for another interface, that can be used for communication.
                }
                // Now. there is a ready interface that we can use
                ipAddr.Val = TCPIP_STACK_NetAddress(netH);
                // display the changed IP address
                if (dwLastIP[i].Val != ipAddr.Val) {
                    dwLastIP[i].Val = ipAddr.Val;
                    if (0 != ipAddr.v[3]) {
                        SYS_CONSOLE_PRINT(TERM_GREEN"%s"TERM_RESET,TCPIP_STACK_NetNameGet(netH));
                        SYS_CONSOLE_MESSAGE(" IP Address: ");
                        SYS_CONSOLE_PRINT("%d.%d.%d.%d \r\n", ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]);
                    }
                }
                app_ethData.state = ETH_APP_TCPIP_DONE;
            }
            break;
        case ETH_APP_TCPIP_DONE:
            APP_ETH_OpenSockets();
            app_ethData.state = ETH_APP_PORT_OPEN;
            break;
		case ETH_APP_PORT_OPEN:
			buffsize = TCPIP_UDP_GetIsReady(app_ethData.uSkt);
			if (buffsize == 0)
			{
				break;
			}
			else
			{
		        TCPIP_UDP_ArrayGet(app_ethData.uSkt,app_ethData.cmdBuffer,buffsize);
				SYS_CONSOLE_PRINT("Cmd %s received\n", app_ethData.cmdBuffer);
                hut_peripheral = 3;
    			app_ethData.state = ETH_APP_WAIT_FOR_CMD_RSP;
                hut_application_input_cmd(app_ethData.cmdBuffer, buffsize);
                memset(app_ethData.cmdBuffer, 0, sizeof(app_ethData.cmdBuffer));
			}
			break;
        case ETH_APP_TCPIP_ERROR:
            app_controlData.moduleError.eETH=true;
            break;
        default:
        {
            break;
        }
    }
}
/*******************************************************************************
 End of File
 */
