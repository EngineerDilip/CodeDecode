#include <stdint.h>
#include <string.h>
#include <stdio.h>
#define DSP_VALID_MASK 0X04
#define DSP_TYPE_MASK 0X03
#define DSP_REASON_MASK 0X78
#define DSP_SUCCESS_MASK 0X04
#define DSP_STAMP_MASK 0X03
#define DSP_APPDB_MASK 0x03
#define DSP_ENTRY_MASK 0x0F
#define DSP_RDB_MASK  0X20
#define DSP_AGE_MASK 0X1C
#define DSP_LIMIT_MASK 0X03
#define DSP_RESULTYPE_MASK 0X3F
#define DSP_ENTRYGROUP_MASK 0XF0
#define DSP_MACSTRENGTH_MASK 0X0C
#define DSP_DESTINATION_LSB_MASK 0X03
#define DSP_DESTINATION_MSB_MASK 0XE0
#define DSP_SRC_MAC_MSB_MASK 0XC0
#define DSP_SRC_MAC_LSB_MASK 0X3F
#define DSP_VSI_LSB_MASK 0xC0
#define DSP_VSI_MSB_MASK 0x7F
# define DSP_SOURCE_MAC_START 16
#define DSP_OUTLIF_LSB_MASK 0X80
#define DSP_OUTLIF_MSB_MASK 0X1F

/* 
 * The DSP payload packet structure. The packet layout is as follows:
 * 
 *   
                   Bits
Reserved            5
Valid               1 
Type                3
Reason              4
Success             1 
Stamp               8
APP-DB              6
Entry Encoded       6
Payload             64
Key                 80    [lsb aligned]
Age                 4
Exceed-Limit        2
----------------------------------
                        184 
*****************************************************************************/
typedef struct bcm_l2_addr_s {
   uint8_t mac[6];                      /* 802.3 MAC address. */
   int port;                           /* Zero-based port number. */
   int encap_id;                       /* out logical interface */
   int vid;
} bcm_l2_addr_t;



void halDspDecode(bcm_l2_addr_t *mac_addr, uint8_t *event, uint8_t dsp_payload[23])
{
   int i,srcMacByte = DSP_SOURCE_MAC_START;
   uint8_t srcMac[6]={0};
   uint16_t sysPort;

   uint8_t reserve = (dsp_payload[0] >> 3);
   uint8_t valid = ((dsp_payload[0] & DSP_VALID_MASK) >> 2);
   uint8_t type = ((dsp_payload[0] & DSP_TYPE_MASK) << 1) | (dsp_payload[1] >> 7);
   uint8_t reason = (dsp_payload[1] & DSP_REASON_MASK) >> 3;
   uint8_t success = (dsp_payload[1] & DSP_SUCCESS_MASK) >> 2;
   uint8_t stamp = ((dsp_payload[1] & DSP_STAMP_MASK) << 6) | (dsp_payload[2] >> 2);
    uint8_t app_db = ((dsp_payload[2] & DSP_APPDB_MASK) << 4) | (dsp_payload[3]  >> 4);
   uint8_t entryCode = ((dsp_payload[3] & DSP_ENTRY_MASK) << 2) | (dsp_payload[4]  >> 6);
   //payload 64bits
   uint8_t resultType = ((dsp_payload[4] & DSP_RESULTYPE_MASK));
   uint8_t entryGroup = ((dsp_payload[5] & DSP_ENTRYGROUP_MASK) >>4);
   uint8_t macStrength = ((dsp_payload[5] & DSP_MACSTRENGTH_MASK) >>2);
   uint32_t Destination = ( ( (uint32_t) dsp_payload[5] & DSP_DESTINATION_LSB_MASK ) << 19 |
                                                     ( ((uint32_t) dsp_payload[6]) << 11 ) |
                                                     ( ((uint32_t)dsp_payload[7]) << 3 ) |
                                                     ( (uint32_t)dsp_payload[8] & DSP_DESTINATION_MSB_MASK) >>5 );

   uint32_t outlif = ( ( (uint32_t) dsp_payload[8] & DSP_OUTLIF_MSB_MASK ) << 17 |
                                            ( ((uint32_t) dsp_payload[9]) << 9 ) |
                                            ( ((uint32_t)dsp_payload[10]) << 1 ) |
                                            ( (uint32_t)dsp_payload[11] & DSP_OUTLIF_LSB_MASK) >>7 );
   uint8_t dest_type= (uint8_t)(Destination>>18);
   printf("dest_type=%d\n",dest_type);
   if(dest_type == 0x03)
   {
      sysPort = (uint16_t)(Destination & (0x00FF));
      printf("sysPort=%02x\n",sysPort);
   }
   //key     80bits
   // extract source mac
   for (i=0;i<6;i++)
   {
      srcMac[i] = ((dsp_payload[srcMacByte+1] & (DSP_SRC_MAC_MSB_MASK)) >> 6) | ((dsp_payload[srcMacByte] & (DSP_SRC_MAC_LSB_MASK)) <<2);
      printf("srcMac=%02x\n",srcMac[i]);
      srcMacByte++;
   }
   uint32_t vsi =( ( (uint32_t) dsp_payload[14] & DSP_VSI_MSB_MASK ) << 10 | ( ((uint32_t) dsp_payload[15]) << 2 ) | ( ((uint32_t)dsp_payload[16] & DSP_VSI_LSB_MASK) >> 6 ));
   uint8_t rdb = ((dsp_payload[22] & DSP_RDB_MASK)  >> 5);
   uint8_t  age = ((dsp_payload[22] & DSP_AGE_MASK)  >> 2);
   uint8_t limit = ((dsp_payload[22] & DSP_LIMIT_MASK));
   printf("reserved=%d,valid=%d, event=%d, reserve=%d, success=%d, stamp=%d, app_db=%d, entryCode=%x,rdb=%d, age=%d,limit=%d,resultType=%d, entryGroup=%d, macStrength=%d ,destination=%04x, VSI=%d\n"
          ,reserve,valid,type,reason,success,stamp,app_db,entryCode,rdb,age,limit,resultType,entryGroup,macStrength,Destination,vsi);

   printf("outlif=%04x\n",outlif);

      //Fll the entry here
   *event = type;
   mac_addr->encap_id = outlif ;
   mac_addr->port = sysPort ;
   mac_addr->vid = vsi;
   memcpy(mac_addr->mac,srcMac, (sizeof(uint8_t)*6));

}

int main()
{
bcm_l2_addr_t bcm_l2_addr;
memset(&bcm_l2_addr,0x00,sizeof(bcm_l2_addr_t));
uint8_t operation =0;
uint8_t dsp_payload[23]={0x04,0x00,0x00,0x26,0x30,0x05,0x80,0x00,0x80,0x48,0x15,0x00,0x00,0x00,0x00,0x04,0xc0,0x00,0x42,0x80,0x2e,0xc0,0x1c};
halDspDecode(&bcm_l2_addr, &operation, dsp_payload);
printf("operation=%d,bcm_l2_addr.encap_id=%4x,bcm_l2_addr.port=%d,bcm_l2_addr.vid=%d\n",operation,(bcm_l2_addr.encap_id),(bcm_l2_addr.port),(bcm_l2_addr.vid));
printf("src mac=%02x:%02x:%02x:%02x:%02x:%02x\n",bcm_l2_addr.mac[0],bcm_l2_addr.mac[1],bcm_l2_addr.mac[2],bcm_l2_addr.mac[3],bcm_l2_addr.mac[4],bcm_l2_addr.mac[5]);
return 0;
}
