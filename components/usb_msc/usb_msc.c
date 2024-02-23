#include "usb_msc.h"
#include "esp_log.h"
#include "tusb_msc_storage.h"

static const char *TAG = "USB MSC";

const char *usb_msc_string_descriptor[] = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04}, /*!< support language is english */
    "TinyUSB",            /*!< manufacturer */
    "TinyUSB Device",     /*!< product */
    "123456",             /*!< chip id */
    "Example MSC",
};

tusb_desc_device_t usb_msc_device_descriptor = {
    .bLength = sizeof(usb_msc_device_descriptor),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC, // 这个是杂项类
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD, // 接口关联描述符，一个usb可以有多个接口，每一个接口有一个功能，IAD可以将这些功能组合
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x303A, // This is Espressif VID. This needs to be changed according to Users / Customers
    .idProduct = 0x4002,
    .bcdDevice = 0x100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01,
};

#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN)

const uint8_t usb_msc_configuration[] = {
    // 配置描述符
    0x09,                               // 配置描述符的字节数
    0x02,                               // 配置描述符的编号类型为2
    U16_TO_U8S_LE(9 + 1 * (9 + 7 + 7)), // 所有描述符的大小的总和 配置描述符、2个端点描述符 in 和 out
    1,                                  // 接口数量，这里只有1个
    1,                                  // 配置描述符数量，这里只有1个
    0,                                  // 当前配置描述符的索引，只有1个，所以是0
    0xA0,                               // 供电模式选择，支持自动唤醒
    0x32,                               // 最大电流，这里是最大电流/2的结果，我们写的是100，所以这里是50

    // 接口描述符
    0x09, // 接口描述符长度为9个字节
    0x04, // 接口描述符类型,接口描述符是4
    0x00, // 接口编号，从0开始
    0x00, // 备用接口编号，从0开始
    2,    // 端点数为2，一个in和一个out
    0x08, // 接口类型 还是看那个class code那个文档 Base Class 08h (Mass Storage)
    0x06, // subclass为SCSI命令集
    0x50, // protocol为Bulk-Only Transport
    0,    // 接口字符串的索引值为0，即为无字符串

    // 端点描述符 out
    0x07,              // 端点描述符的长度为7个字节
    0x05,              // 端点描述符的类型为0x05
    0x01,              // 端点地址，我们这里为输出，0 000 0001,端点号为1，输出
    0x02,              // 批量传输 00 00 00 10
    U16_TO_U8S_LE(64), // 最大包长度，我们这里设置的为64，也可以是512
    0,                 // 端点查询时间为0

    // 端点描述符 in
    0x07,
    0x05,
    0x81,              // 端点地址，我们这里输入 1 000 0001,端点号为1，输入
    0x02,              // 批量传输
    U16_TO_U8S_LE(64), // 最大包长度，我们这里设置的为64，也可以是512
    0,                 // 端点查询时间为0
};

static void usb_msc_mount_changed_cb(tinyusb_msc_event_t *event)
{
    ESP_LOGI(TAG, "Storage mounted to application: %s", event->mount_changed_data.is_mounted ? "Yes" : "No");
}

esp_err_t usb_msc_init(sdmmc_card_t **card)
{
    esp_err_t ret = ESP_FAIL;
    tinyusb_msc_sdmmc_config_t config_sdmmc = {
        .card = *card,
        .callback_mount_changed = usb_msc_mount_changed_cb,
        .mount_config.max_files = 5, // 最大文件打开数量
    };

    ret = tinyusb_msc_storage_init_sdmmc(&config_sdmmc);
    ret = tinyusb_msc_register_callback(TINYUSB_MSC_EVENT_MOUNT_CHANGED, usb_msc_mount_changed_cb); /* Other way to register the callback i.e. registering using separate API. If the callback had been already registered, it will be overwritten. */

    if (ret != ESP_OK)
    {
        return ret;
    }

    // config descriptor
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = &usb_msc_device_descriptor,
        .string_descriptor = usb_msc_string_descriptor,
        .string_descriptor_count = sizeof(usb_msc_string_descriptor) / sizeof(usb_msc_string_descriptor[0]),
        .external_phy = false,
        .configuration_descriptor = usb_msc_configuration,
    };

    ret = tinyusb_driver_install(&tusb_cfg);

    if (ret != ESP_OK)
    {
        return ret;
    }
    return ret;
}