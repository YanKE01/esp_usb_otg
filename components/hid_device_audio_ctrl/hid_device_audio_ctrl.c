#include <stdio.h>
#include "hid_device_audio_ctrl.h"
#include "class/hid/hid_device.h"
#include "tinyusb.h"
#include "string.h"

#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

const char *hid_device_audio_ctrl_string_descriptor[5] = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04}, /*!< support language is english */
    "TinyUSB",            /*!< manufacturer */
    "TinyUSB Device",     /*!< product */
    "123456",             /*!< chip id */
    "Example HID interface",
};

const uint8_t hid_device_audio_report_descriptor[] = {
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(2)),
};

const uint8_t hid_device_audio_configuration_descriptor[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_HID_DESCRIPTOR(0, 0, false, sizeof(hid_device_audio_report_descriptor), 0x81, CFG_TUD_HID_EP_BUFSIZE, 5),
};

bool hid_device_audio_test()
{
    uint16_t data = HID_USAGE_CONSUMER_SCAN_NEXT;
    tud_hid_report(2, &data, 2);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    data = 0;

    return tud_hid_report(2, &data, 2);
}