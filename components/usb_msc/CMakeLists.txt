idf_component_register(
    SRCS "usb_msc.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_tinyusb sd_card
)