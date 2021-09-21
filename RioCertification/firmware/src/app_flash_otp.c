#include "app_flash_otp.h"

powerCalState pCalState;
factoryCalState fCalState;

unsigned char active_slot_rec = 0;
unsigned char p_cal[100] = {0};
const unsigned char gGainTags[COUNTRY_CODE_LEN] =
{
    0x40, 0x50, 0x60, 0x70, 0x80, 0x90
};

#ifdef MCHP_HUT_OTP_SIGNATURE
    uint8_t message_otp[4096];
    uint16_t msg_length = 0;
    uint8_t sign[64];
 #endif

struct flash_map {
        float           gainCorrFact[6];
        float           pwr_17dBm;
        unsigned int    tssi_17dBm;
        unsigned int    SDADC_25;
        unsigned int    VDD0p8;
        float           switchAdjustTemp;
        float           switchAdjustFactory;
        float           dHot1;
        float           dHot2;                             
        float           dCold1;
        float           dCold2;
        unsigned char   baseTxGainIndex   ;
        unsigned char   highGainflag;
    }flash_param,*flash_v = NULL;

static unsigned short it_fletcher_csum(unsigned char * data, unsigned short length, unsigned short * context)
{
    uint8   index;
    uint8 * C0 = (uint8 *)context;
    uint8 * C1 = C0 + 1;
    
    for (index = 0; index < length; index++)
    {
        *C0 = (*C0 + data[index]);
        *C1 = (*C1 + *C0);
    }
    uint8 CB0 = 255 - (*C0 + *C1);
    uint8 CB1 = 255 - (*C0 + CB0);
    
    return CB0 << 8 | CB1;
}
static uint16 otp_insert_checksum(otp_rec_t * rec)
{
    otp_rec_t   drec = *rec;
    uint16      fc_ctxt = 0;
    
    drec.tag &= OTP_REC_TAG_MASK;  // mask out flags for checksum
    it_fletcher_csum((uint8 *)&drec + sizeof(drec.csum),
                     OTP_RECORD_HEADER_SIZE - sizeof(drec.csum),
                     &fc_ctxt);
    rec->csum = it_fletcher_csum(drec.dataptr, drec.length, &fc_ctxt);
    
    return rec->csum;
}
static uint8 otp_verify_checksum(otp_rec_t * rec)
{
    otp_rec_t   drec = *rec;
    uint16      fc_ctxt = 0;
    
    drec.tag &= OTP_REC_TAG_MASK;  // mask out flags for checksum
    it_fletcher_csum((uint8 *)&drec + sizeof(drec.csum),
                     OTP_RECORD_HEADER_SIZE - sizeof(drec.csum),
                     &fc_ctxt);
    drec.csum = it_fletcher_csum(drec.dataptr, drec.length, &fc_ctxt);
    
    return (drec.csum == rec->csum)?1:0;
}

unsigned int flash_write(unsigned int flash_addr, unsigned char *flash_data, unsigned int length)
{ 
    unsigned int r_value = 999;
    int i = 0;
    r_value = program_flash_area((unsigned int) flash_addr,(((unsigned char *)(flash_data))), 
                (length));
       if(0 == r_value)
       {
        printf("\n Writing to flash successful size written %d bytes\n",length);
       }
       else
       {
           printf("\n flash write failed");
       }
}

static otp_rec_t * otp_get_next(otp_rec_t *rec)
{
    static otp_rec_t ret_rec;
    uint8 * ptr;
    
    ret_rec.tag = 0xFF;
    ret_rec.rechdr[0] = 0xFF;
    ret_rec.rechdr[1] = 0xFF;
    ret_rec.rechdr[2]  = 0xFF;
    
    if (rec == 0)
    {
        ptr = (unsigned char *)DUMMY_FLASH_START;
        rec = &ret_rec;
    }
    else
    {
        if (rec->status >= OTP_ERROR)
        {
            printf("\n otp_get_next exit OTP_ERROR\n");
            return rec;
        }
        else if (rec->status == OTP_BAD_RECORD)
        {
            ptr = rec->dataptr + OTP_REC_BADREC_OFFSET;
        }
        else
        {
            ptr = rec->dataptr + rec->length;
        }
    }

    if (ptr > DUMMY_FLASH_END || (ptr + *(ptr + 2)) > DUMMY_FLASH_END)
    {
        rec->status = OTP_ERROR_EXHAUSTED;
        printf("\n OTP_ERROR_EXHAUSTED" );
        
        return rec;
    }
    
    memcpy(rec, ptr, OTP_RECORD_HEADER_SIZE);
    rec->hdrptr  = ptr;
    rec->dataptr = ptr + OTP_RECORD_HEADER_SIZE;
    
    if (*((uint16 *)ptr) == 0xffff)
    {
        rec->length = 0;
        rec->status = OTP_NEW_RECORD;
        //printf("\n new record FOUND ptr : %x value = %x",ptr, *ptr );
        
        return rec;
    }
      
    if (rec->tag & OTP_FLAG_UNCOMMITTED || rec->length > OTP_REC_MAX_LENGTH 
                                        || !otp_verify_checksum(rec))
    {
        rec->status = OTP_BAD_RECORD;
        printf("\n OTP_BAD_RECORD\n");
        
    }
    else
    {
        if ((rec->tag & OTP_FLAG_LIVE) == 0x00)
        {
            rec->status = OTP_DELETED_RECORD;
            
        }
        else
        {
            rec->status = OTP_SUCCESS;
            
        }
    }
    return rec;
}

unsigned int get_remaining_otp_memory(void)
{
    static otp_rec_t ret_rec;
    otp_rec_t * rec = 0;
    unsigned int hdrptr = 0;
    unsigned char no_of_records = 0;
    ret_rec.tag     = ret_rec.length = 0;
    ret_rec.dataptr = (uint8 *)0;
    ret_rec.status  = OTP_ERROR_NOT_FOUND;
    do
    {
        rec = otp_get_next(rec);
    }while((rec->status != OTP_NEW_RECORD) && (rec->status <= OTP_ERROR));
    hdrptr = (unsigned int *)rec->hdrptr;
    return ((DUMMY_FLASH_END - 0xFF) - (hdrptr));
}

unsigned int otp_scan(void)
{
    static otp_rec_t ret_rec;
    otp_rec_t * rec = 0;
    unsigned int hdrptr = 0;
    unsigned char no_of_records = 0;
    ret_rec.tag     = ret_rec.length = 0;
    ret_rec.dataptr = (uint8 *)0;
    ret_rec.status  = OTP_ERROR_NOT_FOUND;
    do
    {
        printf("\n\n***************\n\n");
        rec = otp_get_next(rec);
        no_of_records++;
        printf("\n record no. %d", no_of_records);
        printf("\n start of rec: %x\n",rec->hdrptr + offsetof(otp_rec_t,csum ));
        printf("\n checksum: %x\n",rec->csum);
        printf("\n length: %x\n",rec->length);
        printf("\n rec->tag = %x   @   %x", rec->tag,(rec->hdrptr + offsetof(otp_rec_t, tag)));
        printf("\n reserved header: %x  %x  %x\n",rec->rechdr[0],rec->rechdr[1],rec->rechdr[2]);
        printf("\n rec->hdrptr = %x", rec->hdrptr);
        printf("\n rec->dataptr = %x", rec->dataptr);
        printf("\n rec->dataptr at %x = %x ",rec->dataptr, *(unsigned int *)(rec->dataptr));
        printf("\n rec->dataptr + %x at %x = %x ", rec->length, rec->dataptr + rec->length,*(unsigned int *)(rec->dataptr + rec->length));
        printf("\n rec->status %d\n",rec->status);
        printf("\n\n***************\n\n");
    }while((rec->status != OTP_NEW_RECORD) && (rec->status <= OTP_ERROR));
    printf("\n next available hdrptr : %x",((unsigned int *)rec->hdrptr));
    printf("\n rec->length : %x",(rec->length));
    printf("\n length from hdrptr = %d\n",*(rec->hdrptr + offsetof(otp_rec_t, length)));
    printf("\n Last OTP address : %x",DUMMY_FLASH_END);
    hdrptr = (unsigned int *)rec->hdrptr;
    printf("\n Available OTP : %d",DUMMY_FLASH_END - (hdrptr+ 0xFF));
    return (DUMMY_FLASH_END - (hdrptr + 0xFF));
}
void print_all_record(void)
{
    static otp_rec_t ret_rec;
    otp_rec_t * rec = 0;
    unsigned int no_of_records = 0;
    unsigned int hdrptr = 0;
    ret_rec.tag     = ret_rec.length = 0;
    ret_rec.dataptr = (uint8 *)0;
    ret_rec.status  = OTP_ERROR_NOT_FOUND;
    SYS_CONSOLE_PRINT("\n\n\n **********ALL RECORDS *************\n\n\n");
    do
    {
    rec = otp_get_next(rec);
        no_of_records++;
        SYS_CONSOLE_PRINT("\n\n *******RECORD NO. %d*******\n", no_of_records);
    SYS_CONSOLE_PRINT("\n  RECORD STRUCTURE \n");
    SYS_CONSOLE_PRINT("\n rec->checksum = %x", rec->csum);
    SYS_CONSOLE_PRINT("\n rec->length = %x", rec->length);
    SYS_CONSOLE_PRINT("\n rec->tag = %x", rec->tag);
    SYS_CONSOLE_PRINT("\n rec->header0 = %x", rec->rechdr[0]);
    SYS_CONSOLE_PRINT("\n rec->header1 = %x", rec->rechdr[1]);
    SYS_CONSOLE_PRINT("\n rec->header2 = %x", rec->rechdr[2]);
    SYS_CONSOLE_PRINT("\n rec->hdrptr = %x", rec->hdrptr);
    
    SYS_CONSOLE_PRINT("\n rec->dataptr = %x", rec->dataptr);
    SYS_CONSOLE_PRINT("\n rec->status = %x", rec->status);

#if 0    
    SYS_CONSOLE_PRINT("\n MEMORY MAP\n");
    
    SYS_CONSOLE_PRINT("\n rec->checksum @ %x = %x", (rec->hdrptr + offsetof(otp_rec_t, csum)),
            *(rec->hdrptr + offsetof(otp_rec_t, csum)));
    SYS_CONSOLE_PRINT("\n rec->checksum @ %x = %x", (rec->hdrptr + offsetof(otp_rec_t, csum) + 1),
            *(rec->hdrptr + offsetof(otp_rec_t, csum) + 1));
    SYS_CONSOLE_PRINT("\n rec->length @ %x = %x", (rec->hdrptr + offsetof(otp_rec_t, length)),
            *(rec->hdrptr + offsetof(otp_rec_t, length)));
    SYS_CONSOLE_PRINT("\n rec->length @ %x = %x", (rec->hdrptr + offsetof(otp_rec_t, length) + 1),
            *(rec->hdrptr + offsetof(otp_rec_t, length) + 1));
    SYS_CONSOLE_PRINT("\n rec->tag @ %x = %x", (rec->hdrptr + offsetof(otp_rec_t, tag)),
            *(rec->hdrptr + offsetof(otp_rec_t, tag)));

    SYS_CONSOLE_PRINT("\n rec->header0 at %x = %x ", (rec->hdrptr + offsetof(otp_rec_t, rechdr[0])),
            *(rec->hdrptr + offsetof(otp_rec_t, rechdr[0])));
    SYS_CONSOLE_PRINT("\n rec->header1 at %x = %x ", (rec->hdrptr + offsetof(otp_rec_t, rechdr[1])),
            *(rec->hdrptr + offsetof(otp_rec_t, rechdr[1])));
    SYS_CONSOLE_PRINT("\n rec->header2 at %x = %x ", (rec->hdrptr + offsetof(otp_rec_t, rechdr[2])),
            *(rec->hdrptr + offsetof(otp_rec_t, rechdr[2])));
        SYS_CONSOLE_PRINT("\n rec->dataptr = %x", rec->dataptr);
        SYS_CONSOLE_PRINT("\n Data from %x (%x) to %x (%x)",rec->dataptr, *(unsigned int *)(rec->dataptr),
            rec->dataptr + (rec->length - 4), *(unsigned int *)(rec->dataptr + (rec->length - 4)));
        int temp  = 0;
        SYS_CONSOLE_PRINT("\n");
        for(temp = 0; temp< rec->length; temp=temp+4)
        {
            if(((temp)% 8) == 0)
                SYS_CONSOLE_PRINT("\n");
            SYS_CONSOLE_PRINT("@%x : %x \t",(unsigned int *)(rec->dataptr + temp),*(unsigned int *)(rec->dataptr + temp));
        }
#endif        
    }while((rec->status != OTP_NEW_RECORD) && (rec->status <= OTP_ERROR));
    
    SYS_CONSOLE_PRINT("\n\n\n **********ALL RECORDS ************* \n\n\n");
    SYS_CONSOLE_PRINT("\n next available hdrptr : %x",((unsigned int *)rec->hdrptr));

    SYS_CONSOLE_PRINT("\n Last OTP address : %x",DUMMY_FLASH_END);
    hdrptr = (unsigned int *)rec->hdrptr;
    SYS_CONSOLE_PRINT("\n Available OTP : %d",DUMMY_FLASH_END - (hdrptr+ 0xFF));
    SYS_CONSOLE_PRINT("\n");
    
}
otp_rec_t * otp_read_rec(uint8 tag)
{
    static otp_rec_t ret_rec;
    otp_rec_t * rec = 0;
    ret_rec.tag     = ret_rec.length = 0;
    ret_rec.dataptr = (uint8 *)0;
    ret_rec.status  = OTP_ERROR_NOT_FOUND;
    do
    {
        rec = otp_get_next(rec);
        
        if (OTP_REC_GET_TAG(rec->tag) == OTP_REC_GET_TAG(tag))
        {
            //printf("\n Got record TAG matching this record hdrptr :%x  dataptr: %x\n",
            //        rec->hdrptr, rec->dataptr);
            if ((rec->tag & OTP_FLAG_UNCOMMITTED) == 0) // must be committed
            {
                memcpy(&ret_rec, rec, sizeof(otp_rec_t));   
            }
        }
    }
    while((rec->status != OTP_NEW_RECORD) && (rec->status < OTP_ERROR));
    //print_all_record();
    return &ret_rec;
}
unsigned char otp_write_rec(otp_rec_t *rec)
{
    otp_rec_t * ins_rec = 0;
        
    do
    {
      ins_rec = otp_get_next(ins_rec);
    } while(ins_rec->status != OTP_NEW_RECORD && ins_rec->status < OTP_ERROR);
    if (ins_rec->status >= OTP_ERROR)
    {
        return OTP_ERROR;
    }
    if (ins_rec->hdrptr + OTP_RECORD_HEADER_SIZE + rec->length  >= DUMMY_FLASH_END)
    {
      return OTP_ERROR_EXHAUSTED;
    }
    else if (rec->length > OTP_REC_MAX_LENGTH || OTP_REC_GET_TAG(rec->tag) >= OTP_REC_MAX_TAG_VAL)
    {
       return OTP_BAD_RECORD;
    }
    else // all checks passed, write the data
    {
        rec->hdrptr = ins_rec->hdrptr;
        rec->rechdr[0] = ins_rec->rechdr[0];
        rec->rechdr[1] = ins_rec->rechdr[1];
        rec->rechdr[2] = ins_rec->rechdr[2];

        otp_insert_checksum(rec);  // NB: this shouldn't be needed!!
        rec->tag |= OTP_FLAG_LIVE | OTP_FLAG_UNCOMMITTED;

        flash_write((unsigned int)ins_rec->hdrptr, (uint8 *)rec, OTP_RECORD_HEADER_SIZE);
        //printf("\n Update the Data");
        flash_write((unsigned int)ins_rec->dataptr, (uint8 *)rec->dataptr, rec->length);
        rec->tag ^= OTP_FLAG_UNCOMMITTED;
        //printf("\n Update the commit flag");
        flash_write((unsigned int)ins_rec->hdrptr + offsetof(otp_rec_t, tag),(unsigned char *) &rec->tag, 1);
        //printf("\n ins_rec->hdrptr : %x", (ins_rec->hdrptr));
        //printf("DELETE THE PRESENT REC\n");
        otp_rec_t * ins_rec2 = 0;
        do
        {
            ins_rec2 = otp_get_next(ins_rec2);
            if ((OTP_REC_GET_TAG(ins_rec2->tag) == OTP_REC_GET_TAG(rec->tag)))
            {
                if (ins_rec2->tag & OTP_FLAG_LIVE)
                {
                    if (ins_rec2->hdrptr != rec->hdrptr)
                    {
                        ins_rec2->tag ^= OTP_FLAG_LIVE;
                        flash_write(ins_rec2->hdrptr + offsetof(otp_rec_t, tag), 
                                         (unsigned char *)   &ins_rec2->tag, 1);
                    }
                }
            }
        } while(ins_rec2->status != OTP_NEW_RECORD && ins_rec2->status < OTP_ERROR);
    }
        //printf("\n otp_write_rec exit\n");
    return OTP_SUCCESS;
}
int  write_fact_cal( void *cal_ptr)
{
    unsigned int r_value = 999;
    otp_rec_t rec;
    PRECONbits.PFMWS = 0x4;
    if (cal_ptr != NULL)
    {
        //printf("  Writing record one (length 2):\n");
        rec.tag     = OTP_REC_FACT_CAL_TAG_V2;
        rec.length  = sizeof((flash_param));
        rec.dataptr = cal_ptr;
        otp_write_rec(&rec);

    }
    else
    {
        printf("\n cal_ptr is null \n");
    }
}

int  write_flash_fact_cal( void *cal_ptr)
{
    unsigned int r_value = 999;
    otp_rec_t rec;
    PRECONbits.PFMWS = 0x4;
    if (cal_ptr != NULL)
    {
        rec.tag  = OTP_REC_FACT_CAL_TAG_V2;
        rec.tag |= 1;
        rec.rechdr[0] = 0xFF;
        rec.rechdr[1] = 0xFF;
        rec.rechdr[2] = 0xFF;
        
        rec.length  = sizeof((flash_param));
        rec.dataptr = cal_ptr;
        printf("\n Write Cal to flash\n");
        otp_insert_checksum(&rec);
        program_flash_area(((unsigned int) FLASH_BASE_MEM_FACTORY_CAL),
                            (((unsigned char *)(&rec))), 8);
        program_flash_area(((unsigned int) FLASH_BASE_MEM_FACTORY_CAL + 8),
                            (((unsigned char *)(rec.dataptr))),sizeof((flash_param)));         
    }
    else
    {
        printf("\n cal_ptr is null \n");
    }
}

int  write_txlut( void *gain_ptr, char * c_code, void *r_info, unsigned short rf_ver, unsigned char slot,unsigned char otp)
{
    otp_rec_t regulatory_rec;
    P32MZW1_GAIN_TABLE_ENTRY entry;
    REG_INFO * reg_info = (REG_INFO *)r_info;
    unsigned int address;
    
    if ((slot < 1) || (slot > COUNTRY_CODE_LEN))
       {
        SYS_CONSOLE_PRINT("\n Invalid record slot");
        return -1;
        
        //printf("\n written from %x",((unsigned int) FLASH_BASE_MEM_GAIN + c_code * 72));
    }

    if (strlen(c_code) > COUNTRY_CODE_LEN - 1)
    {
        SYS_CONSOLE_PRINT("\n Country code too long");
        return -1;
    }
    SYS_CONSOLE_PRINT("\n otp_flag in main: %d", otp);
    if(gain_ptr != NULL) 
    {
        slot -= 1;
        if(otp == 0)
        {
        regulatory_rec.tag = gGainTags[slot];
        regulatory_rec.tag |= 1;
        regulatory_rec.rechdr[0] = 0xFF;
        regulatory_rec.rechdr[1] = 0xFF;
        regulatory_rec.rechdr[2] = 0xFF;
        regulatory_rec.length    = sizeof(P32MZW1_GAIN_TABLE_ENTRY);
        memcpy (entry.gainParams, (unsigned char *)gain_ptr, GAIN_TABLE_SIZE);
        entry.channels = reg_info->max_chan;
        entry.channelsBE = reg_info->be_chan;
        entry.version = rf_ver;
        memset(entry.countryCode, 0x00, COUNTRY_CODE_LEN);
        strcpy(entry.countryCode, c_code);
        regulatory_rec.dataptr = (unsigned char *)&entry;
        SYS_CONSOLE_PRINT("\n Write gain to flash\n");
        otp_insert_checksum(&regulatory_rec);

        address = FLASH_BASE_MEM_GAIN + (slot * (OTP_RECORD_HEADER_SIZE + sizeof(P32MZW1_GAIN_TABLE_ENTRY)));
        program_flash_area(
                address,
                (((unsigned char *)(&regulatory_rec))), 
                OTP_RECORD_HEADER_SIZE);
        
        address += OTP_RECORD_HEADER_SIZE;
        program_flash_area(
                address,
                (((unsigned char *)(regulatory_rec.dataptr))),
                regulatory_rec.length);
        }
        else if(otp == 1)
        {
            regulatory_rec.tag = gGainTags[slot];
            regulatory_rec.length    = sizeof(P32MZW1_GAIN_TABLE_ENTRY);
            memcpy (entry.gainParams, (unsigned char *)gain_ptr, GAIN_TABLE_SIZE);
            entry.channels = reg_info->max_chan;
            entry.channelsBE = reg_info->be_chan;
            entry.version = rf_ver;
            memset(entry.countryCode, 0x00, COUNTRY_CODE_LEN);
            strcpy(entry.countryCode, c_code);
            regulatory_rec.dataptr = (unsigned char *)&entry;
            SYS_CONSOLE_PRINT("\n Write gain to OTP length : %d tag = %x\n",
                    regulatory_rec.length,gGainTags[slot]);
            otp_write_rec(&regulatory_rec);
        
        }
#ifdef GAIN_RECORD_DEBUG
        printf("\n FLASH RECORDS \n");
    
        printf("\n csum: %x", *((unsigned short *)(FLASH_BASE_MEM_GAIN + 
                slot * (OTP_RECORD_HEADER_SIZE + sizeof(P32MZW1_GAIN_TABLE_ENTRY)))));
        printf("\n length: %x", *((unsigned short *)(FLASH_BASE_MEM_GAIN + 
                (slot * (OTP_RECORD_HEADER_SIZE + sizeof(P32MZW1_GAIN_TABLE_ENTRY)))+2)));
        printf("\n tag: %x", *((unsigned char *)(FLASH_BASE_MEM_GAIN + 
                (slot * (OTP_RECORD_HEADER_SIZE + sizeof(P32MZW1_GAIN_TABLE_ENTRY)))+4)));
 #endif         
    }
    else
    {
        SYS_CONSOLE_PRINT("\n Gain values not programmed in Firmware \n");
    }        
}

unsigned int  get_factory_cal_from_flash()
{
    if((OTP_REC_FACT_CAL_TAG_V2 | 0x01) !=
            (*((unsigned char *)FLASH_BASE_MEM_FACTORY_CAL 
            + offsetof(otp_rec_t,tag))))
    {
        return 0;
    }
    return (FLASH_BASE_MEM_FACTORY_CAL + OTP_RECORD_HEADER_SIZE);
}
#ifdef MCHP_HUT_OTP_SIGNATURE
void verifyOTPRecords(void)
{
    otp_rec_t *rec = 0, *s_rec = 0;
    uint8_t temp  = 0, slot_no = 1;
    uint8_t slot_tag = 0x40;
    uint32_t sign_rec_hdrptr = 0;
    
    memset(sign, 0, sizeof(sign)/sizeof(sign[0]));
    s_rec = otp_read_rec(OTP_REC_SIGNATURE);
    if(s_rec->status == OTP_ERROR_NOT_FOUND)
    {
        printf("\n Signature record not found\n"); 
        return;
    }
    printf("\n Signature record found!\n");
    memcpy(sign, s_rec->dataptr,s_rec->length);
    
    sign_rec_hdrptr =(uint32_t) s_rec->hdrptr;
    
    memset(message_otp, 0, sizeof(message_otp)/sizeof(message_otp[0]));
    msg_length = 0;
    rec = otp_get_next(rec);
    while((rec->status != OTP_NEW_RECORD) && (rec->status <= OTP_ERROR)
            && (rec->tag != (OTP_REC_SIGNATURE | 0x01)))
    {
        memcpy(message_otp + msg_length, rec->dataptr, rec->length);
        msg_length += rec->length;
        sign_rec_hdrptr =(uint32_t) rec->hdrptr;
         rec = otp_get_next(rec);
    }

#ifdef HUT_CRYPTO_DEBUG_MSG    
   printf("\n data buffer for tamper record Read from OTP:\n\n");
    for(temp = 0; temp < 64; temp++)
    {
       printf("0x%X\t",sign[temp]); 
    }
    
   printf("\n msg_length (verifyOTPSignature) = %d\n\n\n",msg_length);
#endif   
    if(!verifyOTPSignature(message_otp,msg_length,sign))
    {
        printf("\nSignature Verification successful\n");
    }
    else
    {
        printf("\n ERROR: Signature verification failure\n");
    }
    rec = otp_read_rec(OTP_REC_FACT_CAL_TAG_V2);
    if(rec->status == OTP_ERROR_NOT_FOUND)
    {
        printf("\n Factory cal not programmed\n");    
    }
    else
    {
        if(((uint32_t)(rec->hdrptr)) < sign_rec_hdrptr)
        {
            printf("\nFactory cal TAG = 0x%X written by MCHP\n",rec->tag);
        }
        else
        {
            printf("\nFactory cal TAG = 0x%X overwritten \n",rec->tag);
        }
    }
    rec = otp_read_rec(OTP_REC_PON_CAL_TAG);
    if(rec->status == OTP_ERROR_NOT_FOUND)
    {
        printf("\n Power on cal not programmed\n");    
    }
    else
    {
        if(((uint32_t)(rec->hdrptr)) < sign_rec_hdrptr)
        {
            printf("\nPower on cal TAG = 0x%X written by MCHP\n",rec->tag);
        }
        else
        {
            printf("\nPower on cal TAG = 0x%X overwritten \n",rec->tag);
        }
    }
    rec = otp_read_rec(OTP_REC_DEVICE_MAC_ID_TAG);
    if(rec->status == OTP_ERROR_NOT_FOUND)
    {
        printf("\n WiFi MAC not programmed\n");    
    }
    else
    {
        if(((uint32_t)(rec->hdrptr)) < sign_rec_hdrptr)
        {
            printf("\nWiFi MAC TAG = 0x%X written by MCHP\n",rec->tag);
        }
        else
        {
            printf("\nWiFi MAC TAG = 0x%X overwritten \n",rec->tag);
        }
    } 
    rec = otp_read_rec(OTP_REC_ETHERNET_MAC_ID_TAG);
    if(rec->status == OTP_ERROR_NOT_FOUND)
    {
        printf("\n ETH MAC not programmed\n");    
    }
    else
    {
        if(((uint32_t)(rec->hdrptr)) < sign_rec_hdrptr)
        {
            printf("\nETH MAC TAG = 0x%X written by MCHP\n",rec->tag);
        }
        else
        {
            printf("\nETH MAC TAG = 0x%X overwritten \n",rec->tag);
        }
    }
    while(slot_no <= COUNTRY_CODE_LEN)
    {
        rec = otp_read_rec(slot_tag);
        if(rec->status == OTP_ERROR_NOT_FOUND)
        {
            printf("\n slot number %d not programmed\n",slot_no);
            break;
        }
        else
        {
            if(((uint32_t)(rec->hdrptr)) < sign_rec_hdrptr)
            {
                printf("\n Slot %d TAG = 0x%X written by MCHP\n",slot_no, rec->tag);
            }
            else
            {
                printf("\n Slot %d TAG = 0x%X Overwritten \n",slot_no, rec->tag);
            }
        }    

        slot_no++;
        slot_tag = (((slot_tag >> 4) + 1) << 4);
    }    
}
void signOTPRecords(void)
{
    otp_rec_t sign_rec,*rec = 0;
    uint8_t *signptr = NULL;
    uint8_t temp  = 0, slot_no = 1;
    uint8_t slot_tag = 0x40;
    uint32_t sign_rec_hdrptr =0;
    
    memset(message_otp, 0, sizeof(message_otp)/sizeof(message_otp[0]));
    memset(sign, 0, sizeof(sign)/sizeof(sign[0]));
    msg_length = 0;
    rec = otp_get_next(rec);
    while((rec->status != OTP_NEW_RECORD) && (rec->status <= OTP_ERROR))
    {

        memcpy(message_otp + msg_length, rec->dataptr, rec->length);
        msg_length += rec->length;
        sign_rec_hdrptr =(uint32_t) rec->hdrptr;
               
        rec = otp_get_next(rec);
    }

    signptr = (uint8_t *)getOTPSignature(message_otp,msg_length );
    if(NULL == signptr)
    {
        return;
    }
    memcpy(sign,signptr, 64);
    sign_rec.tag     = OTP_REC_SIGNATURE;
    sign_rec.length  = 64;
    sign_rec.dataptr = sign;
#ifdef HUT_CRYPTO_DEBUG_MSG
    printf("\n signature record:\n");
    printf("\n TAG: 0x%X\n",sign_rec.tag );
    printf("\n length(bytes): %d\n",sign_rec.length );
    printf("\n data buffer for tamper record written to OTP:\n\n");
    for(temp = 0; temp < 64; temp++)
    {
       printf("0x%X\t",sign_rec.dataptr[temp]); 
    }
#endif    
    printf("\n Signature record update to OTP!\n");
    otp_write_rec(&sign_rec);
}
#endif
unsigned int  Get_reg_by_slot(unsigned char slot)
{
    otp_rec_t rec_reg, *rec_gain;
    slot -= 1;
    rec_gain = otp_read_rec(gGainTags[slot]);
    if(rec_gain->status == OTP_ERROR_NOT_FOUND)
    {
        SYS_CONSOLE_PRINT("\n slot %x not found in OTP\n",slot+1);
        return 0;
    }
    SYS_CONSOLE_PRINT("\n slot %x found in OTP\n",slot+1);
    return rec_gain->dataptr;
}
int  write_tankcal( void *tank1_cal,void *tank2_cal, unsigned short b_gap)
{
    unsigned int r_value = 999;
    int index  = 0;
    
    otp_rec_t rec;
    memcpy(p_cal,tank1_cal,28);
    memcpy((p_cal + 28),tank2_cal,28);
    memcpy((p_cal + 56), &b_gap,(sizeof(b_gap)));
    rec.tag     = OTP_REC_PON_CAL_TAG;
    rec.length  = (sizeof(tank1_cal) * 7) + (sizeof(tank2_cal) * 7)
                   + sizeof(b_gap) + 2  ;
    SYS_CONSOLE_PRINT("  \n Writing power on cal length = %d\n",rec.length);
    rec.dataptr = p_cal;
    otp_write_rec(&rec);
}
int  write_slot( unsigned char slot_no)
{
        unsigned int r_value = 999;
        otp_rec_t slot_rec;
        slot_rec.tag = 0xB0;
        slot_rec.tag |= 1;
        slot_rec.rechdr[0] = 0xFF;
        slot_rec.rechdr[1] = 0xFF;
        slot_rec.rechdr[2] = 0xFF;
        slot_rec.length    = 4;
        active_slot_rec = slot_no;
        slot_rec.dataptr = &active_slot_rec;
        otp_insert_checksum(&slot_rec);
       program_flash_area(((unsigned int) FLASH_BASE_MEM_ACTIVE_SLOT),
                (((unsigned char *)(&slot_rec))), 8);
       program_flash_area(((unsigned int) FLASH_BASE_MEM_ACTIVE_SLOT + 8),
                (((unsigned char *)(slot_rec.dataptr))), 4);

    //printf("\n Writing slot number\n");
}
int  print_cal(void)
{
    otp_rec_t *rec_factory_ptr;
    
    rec_factory_ptr = otp_read_rec(OTP_REC_FACT_CAL_TAG_V2);
    if((rec_factory_ptr->status == OTP_ERROR_NOT_FOUND) ||
                (rec_factory_ptr->status == OTP_BAD_RECORD))
    {
    rec_factory_ptr = otp_read_rec(OTP_REC_FACT_CAL_TAG);
        if((rec_factory_ptr->status == OTP_ERROR_NOT_FOUND) ||
                (rec_factory_ptr->status == OTP_BAD_RECORD))
        {
            printf("\n NO VALID FACTROY CAL\n");
            return 0;
        }
        else
        {
    set_fact_cal_ptr(rec_factory_ptr->dataptr,0);
        }
    }
    else
    {
        set_fact_cal_ptr(rec_factory_ptr->dataptr,0);
    }
    return 1;    
}

void app_flash_otp_init(void)
{
    //register_flash_callback_tank(write_tankcal);
    register_flash_callback(write_fact_cal,write_txlut,write_tankcal,write_slot,print_cal);
    SYS_CONSOLE_PRINT("%x\n", DUMMY_FLASH_START);
    if(0xFFFFFFFF != (*((unsigned int *)(DUMMY_FLASH_START))))
    {
        otp_rec_t rec_factory, *rec_factory_ptr;
        otp_rec_t rec_power_on, *rec_power_on_ptr;
        
        //printf("\n write_gain_to_flash %x \n", FLASH_BASE_MEM_GAIN);
        if(0xFFFFFFFF == (*((unsigned int *)(FLASH_BASE_MEM_GAIN))))
        {
            SYS_CONSOLE_PRINT("Write gain to flash\n");
            write_gain_to_flash();
        }
        rec_factory_ptr = otp_read_rec(OTP_REC_FACT_CAL_TAG_V2);
        if((rec_factory_ptr->status == OTP_ERROR_NOT_FOUND) ||
                (rec_factory_ptr->status == OTP_BAD_RECORD))
        {
            SYS_CONSOLE_PRINT("\n FACTORY CAL V2 NOT FOUND SEARCH FOR V1\n");
        rec_factory_ptr = otp_read_rec(OTP_REC_FACT_CAL_TAG);
        if((rec_factory_ptr->status == OTP_ERROR_NOT_FOUND) ||
                (rec_factory_ptr->status == OTP_BAD_RECORD))
        {
                SYS_CONSOLE_PRINT("\n FACTORY CAL V1 NOT FOUND \n");
                fCalState = CAL_READ_FAILURE;
            }
            else
            {
                SYS_CONSOLE_PRINT("\n FACTORY CAL V1 FOUND\n");
                fCalState = CAL_READ_V1_SUCCESS;
            }
            
        }
        else
        {
            SYS_CONSOLE_PRINT("\n FACTORY CAL V2 FOUND\n");
            fCalState = CAL_READ_V2_SUCCESS;
        }
        if(fCalState != CAL_READ_FAILURE)
        {
        memcpy(&rec_factory,rec_factory_ptr,sizeof(rec_factory));
		//memcpy(&rec_factory,rec_factory_ptr,rec_factory_ptr->length);
        //printf("\n rec_factory.dataptr = %x\n",rec_factory.dataptr);
        }
        else
        {
            SYS_CONSOLE_PRINT("\n ERROR : NO FACTORY CAL FOUND\n");
        }
        rec_power_on_ptr = otp_read_rec(OTP_REC_PON_CAL_TAG);
        if((rec_power_on_ptr->status == OTP_ERROR_NOT_FOUND) ||
                (rec_power_on_ptr->status == OTP_BAD_RECORD))
        {
            pCalState = PCAL_READ_FAILURE;
            SYS_CONSOLE_PRINT("\n POWER ON CAL ERROR\n");
            
        }
        else
        {
            pCalState = PCAL_READ_SUCCESS;
			memcpy(&rec_power_on,rec_power_on_ptr,sizeof(rec_factory));
//        memcpy(&rec_power_on,rec_power_on_ptr,rec_factory_ptr->length);
        //printf("\n rec_power_on.dataptr = %x\n",rec_power_on.dataptr);
        }
        if((fCalState != CAL_READ_FAILURE) && (pCalState != PCAL_READ_FAILURE))
        {
            if(fCalState == CAL_READ_V1_SUCCESS)
            {
        read_cal_from_flash(rec_factory.dataptr, rec_power_on.dataptr);
            }
            else
            {
                read_cal_v2_from_flash(rec_factory.dataptr, rec_power_on.dataptr); 
            }
        }
        else
        {
            SYS_CONSOLE_PRINT("\n CALIBRATION FAILURE NO VALID RECORD");
            while(1)
            {
                Nop();
            }
        }
    }
}

