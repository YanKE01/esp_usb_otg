#include <stdio.h>
#include "hid_device_audio_ctrl.h"
#include "class/hid/hid_device.h"
#include "string.h"

#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

const char *hid_device_audio_ctrl_string_descriptor[] = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04}, /*!< support language is english */
    "TinyUSB",            /*!< manufacturer */
    "TinyUSB Device",     /*!< product */
    "123456",             /*!< chip id */
    "Example HID interface",
};

tusb_desc_device_t hid_device_audio_ctrl_device_descriptor = {
    .bLength = sizeof(hid_device_audio_ctrl_device_descriptor), // 设备描述符的字节大小数
    .bDescriptorType = 0x01,                                    // DEVICE
    .bcdUSB = 0x0200,                                           // USB2.0
    .bDeviceClass = 0x00,                                       // Class代码 0x00
    .bDeviceSubClass = 0x00,                                    // Subclass代码 0x00
    .bDeviceProtocol = 0x00,                                    // Protocol代码 0x000, class、subclass与protocol组成了base class
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,                  // 端点0的最大包大小
    .idVendor = 0x303A,                                         // 厂商编号
    .idProduct = 0x4002,                                        // 设备变换
    .bcdDevice = 0x100,                                         // 设备出厂编号
    .iManufacturer = 0x01,                                      // 厂商字符串索引，字符串描述符中的第一个（个数从0开始）
    .iProduct = 0x02,                                           // 产品字符串索引，字符串描述符中的第二个
    .iSerialNumber = 0x03,                                      // 序列号字符串索引，字符串索引中的第三个
    .bNumConfigurations = 0x01,                                 // 配置描述符就1个
};

const uint8_t hid_device_audio_ctrl_report_descriptor[] = {
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(2)),
};

const uint8_t hid_device_audio_ctrl_configuration_descriptor[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_HID_DESCRIPTOR(0, 0, false, sizeof(hid_device_audio_ctrl_report_descriptor), 0x81, CFG_TUD_HID_EP_BUFSIZE, 5),
};

bool hid_device_audio_ctrl_test()
{
    uint16_t data = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
    tud_hid_report(2, &data, 2);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    data = 0;

    return tud_hid_report(2, &data, 2);
}