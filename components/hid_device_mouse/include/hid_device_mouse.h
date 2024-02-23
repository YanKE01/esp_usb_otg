#pragma once

#include "tinyusb.h"

#define DISTANCE_MAX 125
#define DELTA_SCALAR 5
typedef enum
{
    MOUSE_DIR_RIGHT,
    MOUSE_DIR_DOWN,
    MOUSE_DIR_LEFT,
    MOUSE_DIR_UP,
    MOUSE_DIR_MAX,
} mouse_dir_t;



extern const char *hid_device_mouse_string_descriptor[5];
extern const uint8_t hid_device_mouse_report_descriptor[];
extern const uint8_t hid_device_mouse_configuration_descriptor[];
extern tusb_desc_device_t hid_device_mouse_device_descriptor;

/**
 * @brief 发送坐标
 * 
 * @param x 
 * @param y 
 * @return true 
 * @return false 
 */
bool hid_device_mouse_send(int x, int y);

/**
 * @brief draw square
 * 
 * @param delta_x_ret 
 * @param delta_y_ret 
 */
void hid_device_mouse_draw_square_next_delta(int8_t *delta_x_ret, int8_t *delta_y_ret);

/**
 * @brief mouse demo
 * 
 */
void hid_device_mouse_demo(void);
