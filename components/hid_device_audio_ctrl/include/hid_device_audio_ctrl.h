#pragma once

#include "sys/types.h"
#include "stdbool.h"

typedef struct
{
    uint8_t play_pause;
    uint8_t mute;
    uint8_t volume_decrement;
} audio_hid_t;

extern const uint8_t hid_device_audio_report_descriptor[];
extern const char *hid_device_audio_ctrl_string_descriptor[5];
extern const uint8_t hid_device_audio_configuration_descriptor[];
bool hid_device_audio_test();