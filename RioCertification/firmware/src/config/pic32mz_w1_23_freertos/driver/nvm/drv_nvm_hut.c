/* ************************************************************************** */
/** MPLAB NVM Driver

  @Company
     Microchip Technology Inc.

  @File Name
    drv_nvm_hut.c

  @Summary
    Driver offering Boot Flash and PFM read/write capability

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

//#include "system_config.h"
#include "xc.h"
//#include "system_definitions.h"
#include "definitions.h"

#define FLASH_PROGRAM_CMD        1
#define FLASH_BLOCKERASE_CMD     2
#define FLASH_CHIPERASE_CMD      5
#define FLASH_LOCK_CMD           8
#define FLASH_UNLOCK_CMD         9
#define FLASH_PAGE_ERASE         3   
#define FLASH_PROGRAM_MAC        6

#define SUCCESS            0
#define PROGRAM_ERROR       100
#define SECTOR_ERASE_ERROR  101
#define CHIP_ERASE_ERROR    102
#define LOCK_ERROR          103
#define UNLOCK_ERROR        104
#define PAGE_ERASE_ERROR    105
#define CMD_MOT_IMPLEMENTED 141

//Macro definitions
#define UNLOCK_KEY1 0x00000000
#define UNLOCK_KEY2 0xAA996655
#define UNLOCK_KEY3 0x556699AA

#define ENABLE_FLASH_OPERATION 0x00008000
#define FLASH_OPERATION_ERROR  0x00002000

#define NOP             0x0
#define SINGLE_WRITE    0x1
#define QUAD_WRITE      0x2
#define ROW_WRITE       0x3
#define PAGE_ERASE      0x4
#define LOWER_PFM_ERASE 0x5
#define UPPER_PFM_ERASE 0x6
#define PFM_ERASE       0x7

#define BOOTFLASH_START_ADDRESS 0x1FC00000
#define CPU_FLASH_ADDRESS       0xB0000000
#define CPU_BOOTFLASH_ADDRESS   BOOTFLASH_START_ADDRESS | CPU_FLASH_ADDRESS
#define BOOT_FLASH_LENGTH       0x10000
#define FLASH_PAGE_LENGTH       0x1000
#define PFM_START_ADDRESS       0x10000000
#define PFM_LENGTH              0x00100000
#define ERASE_VALUE             0xFFFFFFFF
#define INIT_VALUE              ERASE_VALUE
#define NVMADDR_BASE_MASK       0x0000001C
#define FLASH_BASE_ADRESS_MASK  0x1FCFFFFF
#define DISABLE                 0
#define ENABLE                  0x007FFFFF

extern unsigned char g_process_uart_cmd;
extern int g_counter;
extern char g_string[];
extern unsigned int nvm_data[];
#define ALIGN(x)                __attribute__((coherent, aligned(x)));
#define virt2phy(x) ((uint32_t)x & 0x0000FFFFF);
unsigned char *flash_data_ptr = NULL;
unsigned char erase_buffer[4096] ALIGN(16); 
unsigned int *mac_datap = NULL;
unsigned int nvm_data[8];

void NVMOperation(unsigned int command)
{
    /* allow writes to NVMCON.NVMWR and set NVMCON.NVMOP to desired operation using a single write */
    NVMCON = 0x4000 | command;
    __builtin_disable_interrupts();
    /* Perform unlock sequence */
    NVMKEY = UNLOCK_KEY1;
    NVMKEY = UNLOCK_KEY2;
    NVMKEY = UNLOCK_KEY3;
    
    /* Enable operation */
    NVMCON = ENABLE_FLASH_OPERATION;
    while(NVMCONbits.WR);
    __builtin_enable_interrupts();
}

void ReadInitNVMData(unsigned int *baseAddr)
{
    unsigned int nvmDataAdr = (unsigned int )&NVMDATA0, i;
    for(i=0; i<8; i++)
    {
       *(unsigned int *)(nvmDataAdr+(i*0x10)) = *baseAddr++ & INIT_VALUE;
    }
}

void BootFlashProtection(unsigned int endis)
{
    /* Perform unlock sequence */
    NVMKEY = UNLOCK_KEY1;
    NVMKEY = UNLOCK_KEY2;
    NVMKEY = UNLOCK_KEY3;
    //*(unsigned int *)NVMLBWP = 0x80000000 | endis;
    NVMLBWP = 0x80000000 | endis;
}

void flash_main(void)
{
    unsigned int i=0, pgm_addr=0, pgm_len=0, j=0, pages_to_erase =0, offset =0;
    unsigned int *nvmDataAddr = (unsigned int *)&NVMDATA0;
    unsigned int erasePageAddress , bytes_to_write;
    unsigned char* mem_pointer;
    unsigned int numofrows = 0, k = 0, row_offset;
    
    i = 0;
    
    if((nvm_data[7] > 0) && (nvm_data[7] <= 9))
    {
        switch(nvm_data[7])
        {
            case FLASH_PROGRAM_CMD:
               if((nvm_data[0] == CPU_FLASH_ADDRESS) || (nvm_data[0] == CPU_BOOTFLASH_ADDRESS))
               {
                pgm_addr = nvm_data[4] & FLASH_BASE_ADRESS_MASK;
                pgm_len = nvm_data[5];
                if((nvm_data[0] & FLASH_BASE_ADRESS_MASK) == BOOTFLASH_START_ADDRESS)
                    BootFlashProtection((unsigned int)DISABLE);

                nvmDataAddr += (nvm_data[4] & NVMADDR_BASE_MASK);
                while (i < (pgm_len/4))
                {
                    NVMADDR = pgm_addr;
                    ReadInitNVMData((unsigned int *)((pgm_addr & ~NVMADDR_BASE_MASK) | nvm_data[0]));
                    for(;((unsigned int)nvmDataAddr & 0xFF) < 0xC0 ; pgm_addr += 0x4, nvmDataAddr += 0x4)
                    {
                        if(i < (pgm_len/4))
                        {
                            *(nvmDataAddr) = flash_data_ptr[i++];
                        }
                        else
                            break;
                    }
                    NVMOperation(ROW_WRITE);
                    if (NVMCON & FLASH_OPERATION_ERROR)
                    {
                       nvm_data[4] = NVMADDR | nvm_data[0];
                       nvm_data[7] = PROGRAM_ERROR;
                       NVMOperation(NOP);
                       return;
                    }
                    nvmDataAddr = (unsigned int *)&NVMDATA0;
                }
                nvm_data[7] = SUCCESS;
               }
               else
                  {
                    nvm_data[4] = nvm_data[0];
                    nvm_data[7] = PROGRAM_ERROR;
                  }

               break;
            case FLASH_PROGRAM_MAC:
                 if((nvm_data[0] == CPU_FLASH_ADDRESS) || (nvm_data[0] == CPU_BOOTFLASH_ADDRESS))
               {
                pgm_addr = nvm_data[4] & FLASH_BASE_ADRESS_MASK;
                pgm_len = nvm_data[5];
                if((nvm_data[0] & FLASH_BASE_ADRESS_MASK) == BOOTFLASH_START_ADDRESS)
                    BootFlashProtection((unsigned int)DISABLE);

                nvmDataAddr += (nvm_data[4] & NVMADDR_BASE_MASK);
                while (i < (pgm_len/4))
                {
                    NVMADDR = pgm_addr;
                    ReadInitNVMData((unsigned int *)((pgm_addr & ~NVMADDR_BASE_MASK) | nvm_data[0]));
                    for(;((unsigned int)nvmDataAddr & 0xFF) < 0xC0 ; pgm_addr += 0x4, nvmDataAddr += 0x4)
                    {
                        if(i < 2)
                        {
                            *(nvmDataAddr) = mac_datap[i++];
                        }
                        else
                            break;
                    }
                    NVMOperation(SINGLE_WRITE);
                    if (NVMCON & FLASH_OPERATION_ERROR)
                            {
                                nvm_data[4] = NVMADDR | nvm_data[0];
                                nvm_data[7] = PROGRAM_ERROR;
                                NVMOperation(NOP);
                                return;
                    }
                    nvmDataAddr = (unsigned int *)&NVMDATA0;
                }
                nvm_data[7] = SUCCESS;
               }
               else
                  {
                    nvm_data[4] = nvm_data[0];
                    nvm_data[7] = PROGRAM_ERROR;
                  }
                break;
            case FLASH_BLOCKERASE_CMD :               
               nvm_data[7] = CMD_MOT_IMPLEMENTED;
               break;

            case FLASH_PAGE_ERASE :
                if((nvm_data[0] == CPU_FLASH_ADDRESS) || (nvm_data[0] == CPU_BOOTFLASH_ADDRESS))
                {
                    if((nvm_data[0] & FLASH_BASE_ADRESS_MASK) == BOOTFLASH_START_ADDRESS)
                        BootFlashProtection((unsigned int)DISABLE);    
                                
                    numofrows = (nvm_data[5] / 1024) + 1;
                    erasePageAddress = ((uint32_t)nvm_data[4] / FLASH_PAGE_LENGTH) * FLASH_PAGE_LENGTH;
                    pages_to_erase = (nvm_data[5] / FLASH_PAGE_LENGTH) + 1;
                    k = 0;
                    memset(erase_buffer, 0xFF, sizeof(erase_buffer));
                    mem_pointer = (uint8_t *)erasePageAddress;
                    for(i = 0; i < pages_to_erase; i++)
                    {
                        for(j = 0; j < 4096; j++)
                            erase_buffer[j] = mem_pointer[j];
                        NVMADDR = (((uint32_t)(erasePageAddress) & FLASH_BASE_ADRESS_MASK)+ (i *FLASH_PAGE_LENGTH));
                        NVMOperation(PAGE_ERASE);
                        if (NVMCON & FLASH_OPERATION_ERROR)
                        {
                            nvm_data[4] = NVMADDR | nvm_data[0];
                            nvm_data[7] = PAGE_ERASE_ERROR;
                            NVMOperation(NOP);
                            return;
                        }
                        else
                        {
                            nvm_data[7] = SUCCESS;
                        }
                        if (flash_data_ptr != NULL)
                        {
                            bytes_to_write = nvm_data[5];
                            offset =  (nvm_data[4] + (k * 1024)) - erasePageAddress;
                            for(j = 0; j < bytes_to_write; j++)
                                erase_buffer[j + offset] = flash_data_ptr[j];
                            pgm_addr = nvm_data[4] & FLASH_BASE_ADRESS_MASK;
                            pgm_len = bytes_to_write;
                            if((nvm_data[0] & FLASH_BASE_ADRESS_MASK) == BOOTFLASH_START_ADDRESS)
                                BootFlashProtection((unsigned int)DISABLE);
                            nvmDataAddr += (nvm_data[4] & NVMADDR_BASE_MASK);
                            NVMADDR = pgm_addr;
                            int c ;    
                            for (c =0 ; c < 4 ; c ++)
                            {    
                                NVMSRCADDR = virt2phy(erase_buffer + (c * 1024));
                                NVMADDR = (erasePageAddress & 0x1FFFFFFF) + (c *1024);
                                NVMOperation(ROW_WRITE);
                                if (NVMCON & FLASH_OPERATION_ERROR)
                                {
                                    nvm_data[4] = NVMADDR | nvm_data[0];
                                    nvm_data[7] = PROGRAM_ERROR;
                                    NVMOperation(NOP);
                                    return;
                                }
                            }
                        }
                    }
                }
                    break;
                
            case FLASH_CHIPERASE_CMD :
               if((nvm_data[0] == CPU_FLASH_ADDRESS) || (nvm_data[0] == CPU_BOOTFLASH_ADDRESS))
               {
                BootFlashProtection((unsigned int)DISABLE);
                for(i = 0; i < (BOOT_FLASH_LENGTH/FLASH_PAGE_LENGTH); i++)
                {
                    NVMADDR = (BOOTFLASH_START_ADDRESS + (i*FLASH_PAGE_LENGTH));
                    NVMOperation(PAGE_ERASE);
                    if (NVMCON & FLASH_OPERATION_ERROR)
                    {
                        nvm_data[4] = NVMADDR | nvm_data[0];
                        nvm_data[7] = CHIP_ERASE_ERROR;
                        NVMOperation(NOP);
                        return;
                    }
                }
                NVMOperation(PFM_ERASE);
                if (NVMCON & FLASH_OPERATION_ERROR)
                {
                    nvm_data[4] = 0x10000000|nvm_data[0];
                    nvm_data[7] = CHIP_ERASE_ERROR;
                    NVMOperation(NOP);
                }
                else
                    nvm_data[7] = SUCCESS;
               }
               else
                  {
                    nvm_data[4] = nvm_data[0];
                    nvm_data[7] = CHIP_ERASE_ERROR;                    
                  }
               break;

            case FLASH_LOCK_CMD :
                nvm_data[7] = CMD_MOT_IMPLEMENTED;
                break;

            case FLASH_UNLOCK_CMD :
                nvm_data[7] = CMD_MOT_IMPLEMENTED;
                //
                break;
        }               
    }
    else
    {
        nvm_data[7] = CMD_MOT_IMPLEMENTED;        
    }
   
}
/** 
  @Function
    unsigned int program_flash_area(unsigned int flash_addr,
                                    unsigned int *flash_data, unsigned int words)

  @Summary
    API to write to internal Flash area

  @Description
    This function is used to write to both the Boot Flash & the PFM area
    of the Internal Flash on Rio2.
    The minimum write size the function supports is 1 word or 32 bits.
    Maximum size which can be written in one cycle is 8 words or 256bits.
  
    For data more than 256 bits, the function uses more than 1 write cycle
    to write data. This design follows the DOS and is in line with it.
  
  @Precondition
    "None."

  @Parameters
    @param flash_addr Address in flash to being programming.
    
    @param flash_data Data buffer where data to be written is placed.
 
    @param words No. of words to be written
  @Returns
    SUCCESS if write completes else ERROR code when write fails
 */
unsigned int program_flash_area(unsigned int flash_addr, unsigned char *flash_data, unsigned int length)
{   
    /* Erase required area before programming*/
    nvm_data[0] = flash_addr & 0xFFF00000;
    nvm_data[7] = FLASH_PAGE_ERASE;
    nvm_data[4] = flash_addr;
    nvm_data[5] = length;
    flash_data_ptr = flash_data;
    flash_main();
#if 0   
    if(nvm_data[7] == PAGE_ERASE_ERROR)
        return nvm_data[7];
    
    /*Start programming data */
    nvm_data[0] = flash_addr & 0xFFF00000;
    nvm_data[7] = FLASH_PROGRAM_CMD;
    nvm_data[4] = flash_addr;
    nvm_data[5] = length;  // Internal Magic for Quad write
    
    flash_data_ptr = flash_data;
    flash_main();
#endif    
    return nvm_data[7];
}
/** 
  @Function
    unsigned int erase_flash_pages(unsigned int flash_addr, unsigned int num_of_pages)

  @Summary
    API to erase internal Flash area

  @Description
    This function is used to erase both the Boot Flash & the PFM area
    of the Internal Flash on Rio2.
    The minimum erase size the NVM module supports is 1 Page = 4KB.
  
   @Precondition
    "None."

  @Parameters
    @param flash_addr Address in start erase operation.
    
    @param num_of_pages  No. of pages to be erased.
  @Returns
    SUCCESS if erase completes else ERROR code when erase fails
 */
unsigned int erase_flash_pages(unsigned int flash_addr, unsigned int num_of_pages)
{
    nvm_data[0] = flash_addr & 0xFFF00000;         
    nvm_data[7] = FLASH_PAGE_ERASE;
    nvm_data[4] = flash_addr;
    nvm_data[5] = num_of_pages;  // Total Size to erase ,
    flash_main();
    
    return nvm_data[7];
}

void program_mac_address(unsigned int flash_addr, char *mac_addr)
{
	printf("\n **MAC Address** will be written @ 0xBfC55F00\n");
	program_flash_area(flash_addr, mac_addr, 8);    
}
