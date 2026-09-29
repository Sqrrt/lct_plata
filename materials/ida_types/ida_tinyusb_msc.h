/* Exact MSC types from TinyUSB commit
 * 86ad6e56c1700e85f1c5678607a762cfe3aa2f47, sanitized for IDA. */
#ifndef IDA_TINYUSB_MSC_H
#define IDA_TINYUSB_MSC_H

enum {
  MSC_CBW_SIGNATURE = 0x43425355,
  MSC_CSW_SIGNATURE = 0x53425355
};

enum {
  MSC_STAGE_CMD = 0,
  MSC_STAGE_DATA = 1,
  MSC_STAGE_STATUS = 2,
  MSC_STAGE_STATUS_SENT = 3,
  MSC_STAGE_NEED_RESET = 4
};

typedef enum {
  MSC_SUBCLASS_RBC = 1,
  MSC_SUBCLASS_SFF_MMC = 2,
  MSC_SUBCLASS_QIC = 3,
  MSC_SUBCLASS_UFI = 4,
  MSC_SUBCLASS_SFF = 5,
  MSC_SUBCLASS_SCSI = 6
} msc_subclass_type_t;

typedef enum {
  MSC_PROTOCOL_CBI = 0,
  MSC_PROTOCOL_CBI_NO_INTERRUPT = 1,
  MSC_PROTOCOL_BOT = 0x50
} msc_protocol_type_t;

typedef enum {
  MSC_CSW_STATUS_PASSED = 0,
  MSC_CSW_STATUS_FAILED = 1,
  MSC_CSW_STATUS_PHASE_ERROR = 2
} msc_csw_status_t;

#pragma pack(push, 1)

typedef struct {
  uint32_t signature;
  uint32_t tag;
  uint32_t total_bytes;
  uint8_t dir;
  uint8_t lun;
  uint8_t cmd_len;
  uint8_t command[16];
} msc_cbw_t;

typedef struct {
  uint32_t signature;
  uint32_t tag;
  uint32_t data_residue;
  uint8_t status;
} msc_csw_t;

/* The firmware reuses this memory for CBW, CSW, and SCSI data. */
/* Fixed-format SCSI REQUEST SENSE response, 18 bytes. */
typedef struct {
  uint8_t response_code : 7;
  uint8_t valid : 1;
  uint8_t reserved;
  uint8_t sense_key : 4;
  uint8_t reserved2 : 1;
  uint8_t ili : 1;
  uint8_t end_of_medium : 1;
  uint8_t filemark : 1;
  uint32_t information;
  uint8_t add_sense_len;
  uint32_t command_specific_info;
  uint8_t add_sense_code;
  uint8_t add_sense_qualifier;
  uint8_t field_replaceable_unit_code;
  uint8_t sense_key_specific[3];
} scsi_sense_fixed_resp_t;

typedef union {
  uint8_t raw[32];
  msc_cbw_t cbw;
  msc_csw_t csw;
  scsi_sense_fixed_resp_t sense;
} msc_transfer_buffer_t;

/* TinyUSB CFG_TUD_MSC_EP_BUFSIZE is 512 in this firmware. */
typedef struct {
  uint8_t buf[512];
} mscd_ep_buffer_t;

/* TinyUSB mscd_interface_t with explicit padding for IDA. */
typedef struct {
  msc_cbw_t cbw;                 /* offset 0, size 31 */
  uint8_t _pad_after_cbw[1];     /* align CSW to 4 bytes */
  msc_csw_t csw;                 /* offset 32, size 13 */
  uint8_t itf_num;               /* offset 45 */
  uint8_t ep_in;                 /* offset 46 */
  uint8_t ep_out;                /* offset 47 */
  uint8_t stage;                 /* offset 48 */
  uint8_t _pad_before_lengths[3];
  uint32_t total_len;
  uint32_t xferred_len;
  uint8_t sense_key;
  uint8_t add_sense_code;
  uint8_t add_sense_qualifier;
  uint8_t _pad_end[1];
} mscd_interface_t;

#pragma pack(pop)

typedef enum {
  SCSI_CMD_TEST_UNIT_READY = 0x00,
  SCSI_CMD_REQUEST_SENSE = 0x03,
  SCSI_CMD_INQUIRY = 0x12,
  SCSI_CMD_MODE_SENSE_6 = 0x1a,
  SCSI_CMD_START_STOP_UNIT = 0x1b,
  SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL = 0x1e,
  SCSI_CMD_READ_FORMAT_CAPACITY = 0x23,
  SCSI_CMD_READ_CAPACITY_10 = 0x25,
  SCSI_CMD_READ_10 = 0x28,
  SCSI_CMD_WRITE_10 = 0x2a
} scsi_cmd_type_t;

#endif
