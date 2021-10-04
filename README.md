# PIC32MZW1_HUT_Application


PIC32MZW1 HUT Application project is used to enable customers to run the HUT code with any peripheral of their choice. The default project provided shows how to run HUT commands via UART, USB and Ethernet. Using that as a reference code, the customer can make changes to other peripheral code state machines to send the commands via that peripheral. Appropriate changes would be required in the state machine of that peripheral to enable it to run the HUT test. This developer guide along with reference code provides all details required to enable customer to implement the changes.

void hut_application_input_cmd(unsigned char *cmd, unsigned short length)

The above API needs to be invoked from the peripheral code to execute a HUT command. 
The command to be executed and the length of the command needs to be passed as parameters to the API. 
The above API gets executed in the context of the customer application. 
The actual command will be executed in the context of the Wi-Fi task.
The above function will store the command provided in global array (g_string) and set a flag (g_process_cmd) to ensure the command gets executed whenever the Wi-Fi task is run.
Each command given to the HUT library will be responded with a response. The customer application should not give another command to the HUT library till the response for the previous command has been received. The response to the command would be provided via the hut_application_msg function.
It is the customer application responsibility to provide the whole command in one shot. For ex. The customer application might receive the command a single byte at a time over UART. The application would need to keep reading over UART till full command is received and then provide the command in one shot via the hut_application_input_cmd API.

void hut_application_msg(unsigned char *msg)

The above API will be invoked by the HUT library to send a response to the customer application for a command sent by the customer application.
By default the above function invokes SYS_CONSOLE_PRINT and passes the msg received to that function.
There is a global variable (hut_peripheral) provided to set the peripheral type being used. Value of 1 indicates UART, value of 2 indicates USB and value of 3 indicates Ethernet. Hut_peripheral can be automatically set to the peripheral type depending on which interface the command is received from. For ex. app_usb.c sets hut_peripheral to 2 just before passing the command to the HUT library via hut_application_input_cmd. Similarly iconfigCommands sets it to 1 whenever a command is received over UART and hut_peripheral is set to 3 in app_eth.c just before passing the command to the HUT library via hut_application_input_cmd.
Below is a list of responses that will be sent by HUT library for a command received by it i.e. msg passed in hut_application_msg API will point to one of the below strings:
* TX_TEST_STARTED
* TX_TEST_DONE
* TX_TONE_STARTED
* TX_TONE_STOPED
* RX_TEST_STARTED
* RX_PKT_CLEARED
* RX_STOP_DONE
* INVALID_CMD_FORMAT
