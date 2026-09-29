/* Minimal TinyUSB API declarations for IDA type propagation.
 * Exact revision: 86ad6e56c1700e85f1c5678607a762cfe3aa2f47. */
#ifndef IDA_TINYUSB_API_H
#define IDA_TINYUSB_API_H

unsigned char usbd_edpt_xfer(uint8_t rhport, uint8_t ep_addr,
                             uint8_t *buffer, uint16_t total_bytes);
unsigned char usbd_edpt_stall(uint8_t rhport, uint8_t ep_addr);
unsigned char usbd_edpt_clear_stall(uint8_t rhport, uint8_t ep_addr);
unsigned char usbd_edpt_stalled(uint8_t rhport, uint8_t ep_addr);

int32_t tud_msc_scsi_cb(uint8_t lun, const uint8_t scsi_command[16],
                        void *buffer, uint16_t buffer_size);

#endif
