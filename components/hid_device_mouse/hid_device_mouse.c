#include "hid_device_mouse.h"
#include "class/hid/hid_device.h"

void hid_device_mouse_draw_square_next_delta(int8_t *delta_x_ret, int8_t *delta_y_ret)
{
    static mouse_dir_t cur_dir = MOUSE_DIR_RIGHT;
    static uint32_t distance = 0;

    // Calculate next delta
    if (cur_dir == MOUSE_DIR_RIGHT)
    {
        *delta_x_ret = DELTA_SCALAR;
        *delta_y_ret = 0;
    }
    else if (cur_dir == MOUSE_DIR_DOWN)
    {
        *delta_x_ret = 0;
        *delta_y_ret = DELTA_SCALAR;
    }
    else if (cur_dir == MOUSE_DIR_LEFT)
    {
        *delta_x_ret = -DELTA_SCALAR;
        *delta_y_ret = 0;
    }
    else if (cur_dir == MOUSE_DIR_UP)
    {
        *delta_x_ret = 0;
        *delta_y_ret = -DELTA_SCALAR;
    }

    // Update cumulative distance for current direction
    distance += DELTA_SCALAR;
    // Check if we need to change direction
    if (distance >= DISTANCE_MAX)
    {
        distance = 0;
        cur_dir++;
        if (cur_dir == MOUSE_DIR_MAX)
        {
            cur_dir = 0;
        }
    }
}

const char *hid_device_mouse_string_descriptor[5] = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04}, /*!< support language is english */
    "TinyUSB",            /*!< manufacturer */
    "TinyUSB Device",     /*!< product */
    "123456",             /*!< chip id */
    "Example HID interface",
};

const uint8_t hid_device_mouse_report_descriptor[] = {
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),     /*!< 选择通用桌面控制 */
    HID_USAGE(HID_USAGE_DESKTOP_MOUSE),         /*!< 选择鼠标 */
    HID_COLLECTION(HID_COLLECTION_APPLICATION), /*!< 创建一个功能集合 */
    HID_REPORT_ID(HID_ITF_PROTOCOL_MOUSE)
        HID_USAGE(HID_USAGE_DESKTOP_POINTER), /*!< 选择指针*/
    HID_COLLECTION(HID_COLLECTION_PHYSICAL),  /*!< 创建数据集合 */
    // 设置按键
    HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON), /*!< 开始设置鼠标上的按键 */
    HID_USAGE_MIN(1),
    HID_USAGE_MAX(3), /*!< 一共设置了三个按键，分别是左右中 */
    HID_LOGICAL_MIN(0),
    HID_LOGICAL_MAX(1),                                /*!< 按键的数据范围只有0和1 */
    HID_REPORT_COUNT(3),                               /*!< 这里一共三个usage，就是之前对应和三个按键 */
    HID_REPORT_SIZE(1),                                /*!< 这里指的是每一个usage的大小，因为只有0和1，所以只占1位 */
    HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), /*!< 数据类型 */
    HID_REPORT_COUNT(1),
    HID_REPORT_SIZE(5),      /*!< 前面三个按键一共三个bit，我们要凑一个字节 */
    HID_INPUT(HID_CONSTANT), /*!< 这里的数据类型随便 */
    // 设置x和y轴
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
    HID_USAGE(HID_USAGE_DESKTOP_X),
    HID_USAGE(HID_USAGE_DESKTOP_Y), /*!< 一共设置了x和y轴 */
    HID_LOGICAL_MIN(0x81),
    HID_LOGICAL_MAX(0x7f),                             /*!< 这里设置了x轴和y轴的范围：-127到128 */
    HID_REPORT_COUNT(2),                               /*!< 只有两个usage */
    HID_REPORT_SIZE(8),                                /*!< 每一个usage的大小为一个字节 */
    HID_INPUT(HID_DATA | HID_VARIABLE | HID_RELATIVE), /*!< 鼠标移动是相对的 */
    HID_COLLECTION_END,
    HID_COLLECTION_END};

const uint8_t hid_device_mouse_configuration_descriptor[] = {
    // 配置描述符
    0x09,                               /*!< config:blength */
    0x02,                               /*!< config:bdescriptorType */
    U16_TO_U8S_LE(9 + 1 * (9 + 9 + 7)), /*!< config:wTotalLength, 所有描述符大小和=配置描述符大小+hid接口数*(接口描述符+HID描述符+端点描述符) */
    1,                                  /*!< config:bNumInterfaces,只有一个接口描述符 */
    1,                                  /*!< config:bConfigurationValue,只有一个配置描述符 */
    0,                                  /*!< config:iConfiguration,只有1个配置描述符，所以索引为0 */
    0xA0,                               /*!< config:bmAttributes,供电模式，10100000,支持远程唤醒 */
    0x32,                               /*!< config:maxpower,设置的为需要100ma，所以填的50 */

    // 接口描述符
    0x09, /*!< interface:blength 描述符长度*/
    0x04, /*!< interface:bDescriptorType 描述符类型*/
    0x00, /*!< interface:bInterfaceNumber 接口编号,从0开始递增*/
    0x00, /*!< interface:bAlternateSetting 备用接口编号，很少用，设置为0*/
    0x01, /*!< interface:bNumEndpoints 端点数 */
    0x03, /*!< interface:bInterfaceClass 接口类型，HID为3 */
    0x00, /*!< interface:bInterfaceSubClass 子类型 0表示为未定义 */
    0x00, /*!< interface:bInterfaceProtocol */
    0x00, /*!< interface:iInterface 接口字符串索引值，0表示无字符串 */

    // HID描述符
    0x09,                                                      /*!< HID描述符长度 */
    0X21,                                                      /*!< 描述符类型: HID */
    U16_TO_U8S_LE(0x0110),                                     /*!< HID描述符规范码  */
    0x00,                                                      /*!< HID描述符：硬件国家码，0表示不说明 */
    0x01,                                                      /*!< HID描述符：类别描述符个数，至少一个 */
    0x22,                                                      /*!< HID描述符：描述符类型，这里是报表 */
    U16_TO_U8S_LE(sizeof(hid_device_mouse_report_descriptor)), /*!< HID描述符：报表描述符总长度 */

    // 端点描述符
    0x07,              /*!< 端点描述符：长度 */
    0x05,              /*!< 端点描述符：描述符类型为0x05 */
    0x81,              /*!< 端点描述符：端点地址，0x81=10000001,第7位为1表示输入，一个端点，多以d0-d3为1 */
    0x03,              /*!< 端点描述符：端点属性，中断传输，00000011 */
    U16_TO_U8S_LE(16), /*!< 端点描述符：最大包长度 */
    10,                /*!< 端点描述符：端点查询时间 */

};

bool hid_device_mouse_send(int x, int y)
{
    int8_t data[3] = {0x00, x, y};
    return tud_hid_report(HID_ITF_PROTOCOL_MOUSE, data, sizeof(data));
}