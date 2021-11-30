# PIC32MZW1_HUT_Application


PIC32MZW1 HUT Application project is used to enable customers to run the HUT code with any peripheral of their choice. The default project provided shows how to run HUT commands via UART, USB and Ethernet. Using that as a reference code, the customer can make changes to other peripheral code state machines to send the commands via that peripheral. Appropriate changes would be required in the state machine of that peripheral to enable it to run the HUT test. This developer guide along with reference code provides all details required to enable customer to implement the changes.

# API List

## void hut_application_input_cmd(unsigned char *cmd, unsigned short length)

The above API needs to be invoked from the peripheral code to execute a HUT command. 
The command to be executed and the length of the command needs to be passed as parameters to the API. 
The above API gets executed in the context of the customer application. 
The actual command will be executed in the context of the Wi-Fi task.
The above function will store the command provided in global array (g_string) and set a flag (g_process_cmd) to ensure the command gets executed whenever the Wi-Fi task is run.
Each command given to the HUT library will be responded with a response. The customer application should not give another command to the HUT library till the response for the previous command has been received. The response to the command would be provided via the hut_application_msg function.
It is the customer application responsibility to provide the whole command in one shot. For ex. The customer application might receive the command a single byte at a time over UART. The application would need to keep reading over UART till full command is received and then provide the command in one shot via the hut_application_input_cmd API.

## void hut_application_msg(unsigned char *msg)

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

# List of commands required for Certification

## Command to start modulated transmission (iconfig --starttx)

iconfig --starttx "Channel" "Rate" "Duration" "Rate_type" "Antenna" "Power" "SecChOffset" "SecChPos" "IsshortGI/Preamble" "FrameLen" "ResetPhyRf" "EnableCCA"
        :- Starts modulated Transmission where:-
  * Channel : Transmission channel number
  * Rate : Data rates (1, 2, 5, 6, 9, 11, 12, 18, 24, 36, 48, 54 Mbps or 0 to 7 if its MCS rates)
  * Duration : Duration of transmission in Seconds. Valid values are from 1 to 9999.  9999 option will be used for continuous transmission for up to 9999 seconds. This will work based on TSF timestamps to check the time upper limit and duration values are met with an close approximation to stop the transmission when the target time duration is met and all the transmission variables reset and RF mode standby.
  * Rate_type : 1 for MCS rates; Default->0 for legacy
  * Antenna : 1 - CONN1 and 2 - CONN2 (Not used in Rio2 but need to enter to succeed the command)
  * Power: Check the section on how "Power Calculation" is done.
  * SecChOffset : 0 -> 20MHz; 1 -> Secondary channel will be above primary channel; 3-> Secondary channel will be below primary channel
  * SecChPos : 2 -> 20MHz operation (no offset) and 0 -> full 40MHz band; 1 -> upper 20MHz 3 -> lower 20MHz
  * IsshortGI/Preamble : 0 -> long; 1-> short. GI: GI option 0 -> Long guard interval, GI option 1 -> Short guard interval (SGI). Preamble: 0 -> Long preamble, 1 -> short preamble
  * FrameLen : Frame length in bytes
  * ResetPhyRf : Flags for resetting PHY, RF and triggering RF Calibration before Tx or Rx
  * EnableCCA: If set, this will enable CCA while the transmissions. This option is mainly used for Adaptivity tests. If clear, this will disable CCA.

### Command Usage Example:

To start Tx on channel 9 with MCS 7 rate:
  * iconfig --starttx 9 7 9999 1 2 0 0 2 0 500 0 0

TX_TEST_STARTED is the last response seen on the console

## Command to stop the ongoing transmission (iconfig --stoptx)

iconfig --stoptx
        :-Stops Transmission

### Command Usage Example:
  
  * iconfig --stoptx

TX_STOP_DONE is the last response seenon the console.

## Command to start an unmodulated tone transmission (iconfig --starttone)
  
iconfig --starttone "channel" "incr_ptr" "GainIdx"
  :- Starts un-modulated Carrier Transmission
  
  * incr_ptr: -28 ~28     default 0 . ie for center frequency
  * GainIdx:   Range 0-255, Default   66 
  
  incr_ptr and GainIdx are optional but needs to be given together as mentioned below.

### Command Usage Example:
  
  * iconfig --starttone 11 4 66
  OR 
  * iconfig --starttone 11

TX_TONE_STARTED is the last console message

## Command to stop the transmission of unmodulated tone (iconfig --stoptone)

iconfig --stoptone
  :- Stops un-modulated Carrier Transmission

### Command Usage Example:
* iconfig --stoptone

TX_TONE_STOPED is the last console message

## Command to start receiving on given channel and start collecting Rx statistics (iconfig --startrx)

iconfig --startrx "Channel" "SecChOffset" "Antenna" "ResetPhyRf"
  :-Puts device in Rx mode in the given channel and starts collecting Rx statistics.

### Command Usage Example:
* iconfig --startrx 6 0 2 0

RX_PKT_CLEARED
RX_TEST_STARTED are the last console messages

## Command to stop reception and clear the Rx statistics (iconfig --stoprx)

iconfig --stoprx
  :-Stops Reception

### Command Usage Example:
* iconfig --stoprx

RX_STOP_DONE is the last console messsage

## Command to get Rx statistics (iconfig --rxpktcount)

iconfig --rxpktcount "Rate" "Rate_type"
  :- Displays the TOTAL and CRC SUCCESS packets

### Command Usage Example:

To print Rx statistics for all the data rates:
* iconfig --rxpktcount 0 0

## Command to clear the Rx statistics (iconfig --rxpktcountclear)

iconfig --rxpktcountclear
   :- Clears Rx statistics

### Command Usage Example:

* iconfig --rxpktcountclear

RX_PKT_CLEARED is the last console message

## Command to reset the system and fresh boot (iconfig --reset)

### Command Usage Example:

* iconfig --reset

## Command to get the Base Tx Gain Index (iconfig --printBaseTxGainIndex)

This command is used to get the Base Tx Gain Index value from the memory. It is required for power calculation purposes.

### Command Usage Example:

* iconfig --printBaseTxGainIndex

baseTxGainIndex "number" is the response. "number" printed will be used in transmit power calculations. Check the "Power Calculation" section to see how it is used.

# Power Calculation

Formula for calculating TX_Gain_index is “TX_Gain_index = BaseGainIndex + RateOffset - (BaseTXP – desired_txp) * 4 + channel_offset”

The BaseGainIndex is used to eliminate the device to device power variation. It is a value stored in memory and it is specific to the device. Hence this value needs to be retrieved from memory for the device on which the test is going to be done.

The TX_Gain_index value can be calculated using the above formula and the values provided in the below tables.

After the TX_Gain_index has been calculated, it needs to be provided as an input to the “iconfig –starttx" command.

![image](https://user-images.githubusercontent.com/47098770/143814718-ac9038ef-d2c9-496a-9c88-95fd79100a7d.png)

![image](https://user-images.githubusercontent.com/47098770/143814584-46edb3e7-97a8-47d8-bc01-09977e870623.png)

![image](https://user-images.githubusercontent.com/47098770/143814618-4ce19b29-e1c2-4e09-87c5-9c14ab0b7f0b.png)

A typical test flow during certification will be as follows:

* Customer (End product Certification requirements) decides on the desired transmit power.
* Based on that, they calculate the gain index value using the formula provided above.
* Now they use this gain index with starttx to do the test and test for certification requirements (The calculated value is provided as the value for the "power" parameter in the iconfig --starttx command).

# Interface Specific Notes

## Ethernet

The firmware has DHCP Server enabled by default. The IP address assigned to the DUT is 192.168.5.1. When the test device (for ex. laptop) is connected via Ethernet cable to the DUT, it will be assigned an IP address by the DHCP Server running on the DUT. Connectivity to the DUT can be tested by running a ping to 192.168.5.1 from the laptop.

Once the IP has been assigned to the laptop, scripts can be written on the laptop to trigger the RF tests. The validation was done using a utility like "Packet Sender" to send the command to the DUT from the laptop. A sample screenshot is given below for "iconfig --startrx" command:


![image](https://user-images.githubusercontent.com/47098770/144002467-de96c185-0622-4fbb-87be-9954707eaa26.png)

The DUT response to any command received from the host by sending a packet to the broadcast IP address "255.255.255.255" and port number 8000. If the host needs to process any response from the DUT (for ex. check the receive statistics), it would need to wait for a packet on port 8000 on any IP address of the laptop. The validation was done using a python script that was waiting for a packet to be received on port 8000 on any of the laptop interfaces and printing any packet received.

![image](https://user-images.githubusercontent.com/47098770/144002847-8d2e941a-3f5f-41e8-b667-f3985f59eec0.png)

Below is the sample output from the receive.py script when the command "iconfig --rxpktcount 0 0" is given:

![image](https://user-images.githubusercontent.com/47098770/144012015-fff0e550-e4d7-4100-ae62-0285cf003167.png)

