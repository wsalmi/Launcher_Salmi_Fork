idf_component_register(
    SRCS "serial_compat.cpp" "wifi_compat.cpp" "serialCommands_idf.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_wifi nvs_flash esp_netif
)
