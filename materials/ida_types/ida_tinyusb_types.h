/*
 * IDA import subset from TinyUSB commit
 * 86ad6e56c1700e85f1c5678607a762cfe3aa2f47
 * Pico SDK 2.2.0.
 *
 * This is intentionally sanitized for IDA's C parser: TinyUSB macros,
 * includes, inline helpers, and static assertions were removed.
 */
#ifndef IDA_TINYUSB_TYPES_H
#define IDA_TINYUSB_TYPES_H

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef signed int     int32_t;

typedef enum {
  TUSB_DIR_OUT = 0,
  TUSB_DIR_IN = 1,
  TUSB_DIR_IN_MASK = 0x80
} tusb_dir_t;

typedef enum {
  TUSB_XFER_CONTROL = 0,
  TUSB_XFER_ISOCHRONOUS = 1,
  TUSB_XFER_BULK = 2,
  TUSB_XFER_INTERRUPT = 3
} tusb_xfer_type_t;

typedef enum {
  TUSB_REQ_GET_STATUS = 0,
  TUSB_REQ_CLEAR_FEATURE = 1,
  TUSB_REQ_SET_FEATURE = 3,
  TUSB_REQ_SET_ADDRESS = 5,
  TUSB_REQ_GET_DESCRIPTOR = 6,
  TUSB_REQ_SET_DESCRIPTOR = 7,
  TUSB_REQ_GET_CONFIGURATION = 8,
  TUSB_REQ_SET_CONFIGURATION = 9,
  TUSB_REQ_GET_INTERFACE = 10,
  TUSB_REQ_SET_INTERFACE = 11,
  TUSB_REQ_SYNCH_FRAME = 12
} tusb_request_code_t;

typedef enum {
  TUSB_REQ_TYPE_STANDARD = 0,
  TUSB_REQ_TYPE_CLASS = 1,
  TUSB_REQ_TYPE_VENDOR = 2,
  TUSB_REQ_TYPE_INVALID = 3
} tusb_request_type_t;

typedef enum {
  TUSB_REQ_RCPT_DEVICE = 0,
  TUSB_REQ_RCPT_INTERFACE = 1,
  TUSB_REQ_RCPT_ENDPOINT = 2,
  TUSB_REQ_RCPT_OTHER = 3
} tusb_request_recipient_t;

typedef enum {
  TUSB_CLASS_UNSPECIFIED = 0,
  TUSB_CLASS_MSC = 8,
  TUSB_CLASS_VENDOR_SPECIFIC = 0xff
} tusb_class_code_t;

typedef enum {
  XFER_RESULT_SUCCESS = 0,
  XFER_RESULT_FAILED,
  XFER_RESULT_STALLED,
  XFER_RESULT_TIMEOUT,
  XFER_RESULT_INVALID
} xfer_result_t;

#pragma pack(push, 1)

typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t bcdUSB;
  uint8_t bDeviceClass;
  uint8_t bDeviceSubClass;
  uint8_t bDeviceProtocol;
  uint8_t bMaxPacketSize0;
  uint16_t idVendor;
  uint16_t idProduct;
  uint16_t bcdDevice;
  uint8_t iManufacturer;
  uint8_t iProduct;
  uint8_t iSerialNumber;
  uint8_t bNumConfigurations;
} tusb_desc_device_t;

typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t wTotalLength;
  uint8_t bNumInterfaces;
  uint8_t bConfigurationValue;
  uint8_t iConfiguration;
  uint8_t bmAttributes;
  uint8_t bMaxPower;
} tusb_desc_configuration_t;

typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  uint8_t bNumEndpoints;
  uint8_t bInterfaceClass;
  uint8_t bInterfaceSubClass;
  uint8_t bInterfaceProtocol;
  uint8_t iInterface;
} tusb_desc_interface_t;

typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bEndpointAddress;
  uint8_t bmAttributes;
  uint16_t wMaxPacketSize;
  uint8_t bInterval;
} tusb_desc_endpoint_t;

typedef struct {
  uint8_t bmRequestType;
  uint8_t bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
} tusb_control_request_t;


typedef struct {
  char const* name;
  void     (* init             ) (void);
  bool     (* deinit           ) (void);
  void     (* reset            ) (uint8_t rhport);
  uint16_t (* open             ) (uint8_t rhport, tusb_desc_interface_t const * desc_intf, uint16_t max_len);
  bool     (* control_xfer_cb  ) (uint8_t rhport, uint8_t stage, tusb_control_request_t const * request);
  bool     (* xfer_cb          ) (uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes);
  void     (* sof              ) (uint8_t rhport, uint32_t frame_count); // optional
} usbd_class_driver_t;

#define CFG_TUD_ENDPOINT0_SIZE  64

#define CFG_TUD_INTERFACE_MAX   16


typedef enum {
  TUSB_ROLE_INVALID = 0u,
  TUSB_ROLE_DEVICE  = 0x1,
  TUSB_ROLE_HOST    = 0x2,
} tusb_role_t;

typedef enum {
  TUSB_SPEED_FULL = 0,
  TUSB_SPEED_LOW  = 1,
  TUSB_SPEED_HIGH = 2,
  TUSB_SPEED_AUTO = 0xaa,
  TUSB_SPEED_INVALID = 0xff,
} tusb_speed_t;

typedef struct {
  tusb_role_t role;
  tusb_speed_t speed;
} tusb_rhport_init_t;

typedef struct {
  uint16_t len;
  uint8_t *buffer;
} tusb_buffer_t;

#pragma pack(pop)

#endif
