#include <xc.h>
#include "system_config.h"
//#include "system_definitions.h"
#include "definitions.h"

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

#define FLASH_BASE_MEM_GAIN  0xB00FF800
#define GAIN_TABLE_SIZE  48
#define FLASH_BASE_MEM_ACTIVE_SLOT (FLASH_BASE_MEM_GAIN + (COUNTRY_CODE_LEN * \
                                    (sizeof(P32MZW1_GAIN_TABLE_ENTRY) + \
                                    OTP_RECORD_HEADER_SIZE)))
#define FLASH_BASE_MEM_FACTORY_CAL (FLASH_BASE_MEM_ACTIVE_SLOT + \
                                    OTP_RECORD_HEADER_SIZE + \
                                    sizeof(unsigned int))
#define COUNTRY_CODE_LEN        6

typedef enum poweronCalState {
    PCAL_READ_FAILURE = 0,
    PCAL_READ_SUCCESS = 1
}powerCalState;

typedef enum factoryCalState {
    CAL_READ_FAILURE = 0,
    CAL_READ_V1_SUCCESS = 1,
    CAL_READ_V2_SUCCESS = 3
}factoryCalState;

typedef struct
    {
    char countryCode[COUNTRY_CODE_LEN];
    unsigned short version;
    unsigned short channels;
    unsigned short channelsBE;
    unsigned long gainParams[12];
} P32MZW1_GAIN_TABLE_ENTRY;

typedef struct regulatory_info {
    unsigned short max_chan;
    unsigned short be_chan;
    unsigned char  r_code;    
} REG_INFO;

#define OTP_REC_FACT_CAL_TAG  (0x10)
#define OTP_REC_PON_CAL_TAG   (0x20)
#define OTP_REC_DEVICE_MAC_ID_TAG    (0x30)
#define OTP_REC_ETHERNET_MAC_ID_TAG  (0xA0)
#define OTP_REC_FACT_CAL_TAG_V2  (0xB0)
#define OTP_REC_SIGNATURE        (0xC0)

#define OTP_RECORD_HEADER_SIZE  (sizeof(uint16) + /*csum*/\
                                 sizeof(uint16) +   /* length   */    \
                                 sizeof(uint8)  +   /* tag    */    \
                                 sizeof(uint8) *3)     /* header */

#define OTP_REC_TAG_BITS        (5)   // Number of bits in the tag field
#define OTP_REC_FLAG_BITS       (8 - OTP_REC_TAG_BITS)
#define OTP_REC_FLAG_MASK       ((1 << (OTP_REC_FLAG_BITS)) - 1)
#define OTP_REC_TAG_MASK        (0xFF ^ OTP_REC_FLAG_MASK)
#define OTP_REC_GET_TAG(x)      ((x) >> OTP_REC_FLAG_BITS)
#define OTP_REC_SET_TAG(x)      ((x) << OTP_REC_FLAG_BITS)
#define OTP_REC_SET_FLAGS(x)    ((x) & OTP_REC_FLAG_MASK)
#define OTP_REC_MAX_TAG_VAL     ((OTP_REC_TAG_MASK >> OTP_REC_FLAG_BITS) - 1) // reserve 11..b
#define OTP_REC_MAX_LENGTH      (1024)
#define OTP_REC_BADREC_OFFSET   (256) // Bytes to skip ahead if writing after bad data found
#define OTP_SUCCESS         0
#define OTP_NEW_RECORD      1
#define OTP_DELETED_RECORD  2
#define OTP_BAD_RECORD      3
#define OTP_CHECKSUM_ERROR  4
#define OTP_ERROR           10
#define OTP_ERROR_EXHAUSTED 11
#define OTP_ERROR_NOT_FOUND 12
#define OTP_FLAG_UNCOMMITTED    0x02
#define OTP_FLAG_LIVE           0x01
#define DUMMY_FLASH_START 0xBFC56000
#define DUMMY_FLASH_END 0xBFC56FBF

typedef struct otp_rec{
    unsigned short  csum;
    unsigned short  length;
    unsigned char   tag;
    
    unsigned char   rechdr[3];
    unsigned char  * hdrptr;
    unsigned char * dataptr;
    unsigned char   status;
} otp_rec_t;

unsigned int flash_write(unsigned int flash_addr, unsigned char *flash_data, unsigned int length);
unsigned int get_remaining_otp_memory(void);
unsigned int otp_scan(void);
void print_all_record(void);
otp_rec_t * otp_read_rec(unsigned char tag);
unsigned char otp_write_rec(otp_rec_t *rec);
int  write_fact_cal( void *cal_ptr);
int  write_flash_fact_cal( void *cal_ptr);
int  write_txlut( void *gain_ptr, char * c_code, void *r_info, unsigned short rf_ver, unsigned char slot,unsigned char otp);
unsigned int  get_factory_cal_from_flash();
#ifdef MCHP_HUT_OTP_SIGNATURE
void verifyOTPRecords(void);
void signOTPRecords(void);
#endif
unsigned int  Get_reg_by_slot(unsigned char slot);
int  write_tankcal( void *tank1_cal,void *tank2_cal, unsigned short b_gap);
int  write_slot( unsigned char slot_no);
int  print_cal(void);
void app_flash_otp_init(void);

