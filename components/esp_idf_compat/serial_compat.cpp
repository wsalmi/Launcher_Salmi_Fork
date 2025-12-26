/**
 * ESP-IDF Serial Compatibility Layer Implementation
 */

#include "serial_compat.h"
#include <cstdio>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Global instance
SerialCompat Serial(UART_NUM_0);

SerialCompat::SerialCompat(uart_port_t port) : uart_num(port) {
    memset(rx_buffer, 0, sizeof(rx_buffer));
}

void SerialCompat::begin(uint32_t baud, uint32_t config) {
    uart_config_t uart_config = {
        .baud_rate = (int)baud,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, 
                                  UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(uart_num, 1024, 0, 0, NULL, 0));
}

void SerialCompat::end() {
    uart_driver_delete(uart_num);
}

size_t SerialCompat::print(const char* str) {
    return write((const uint8_t*)str, strlen(str));
}

size_t SerialCompat::print(const std::string& str) {
    return write((const uint8_t*)str.c_str(), str.length());
}

size_t SerialCompat::print(int num) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", num);
    return print(buf);
}

size_t SerialCompat::print(unsigned int num) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%u", num);
    return print(buf);
}

size_t SerialCompat::print(float num, int digits) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*f", digits, num);
    return print(buf);
}

size_t SerialCompat::println(const char* str) {
    size_t n = print(str);
    return n + print("\r\n");
}

size_t SerialCompat::println(const std::string& str) {
    size_t n = print(str);
    return n + print("\r\n");
}

size_t SerialCompat::println(int num) {
    size_t n = print(num);
    return n + print("\r\n");
}

size_t SerialCompat::println(unsigned int num) {
    size_t n = print(num);
    return n + print("\r\n");
}

size_t SerialCompat::println(float num, int digits) {
    size_t n = print(num, digits);
    return n + print("\r\n");
}

size_t SerialCompat::write(uint8_t byte) {
    return uart_write_bytes(uart_num, &byte, 1);
}

size_t SerialCompat::write(const uint8_t* buffer, size_t size) {
    return uart_write_bytes(uart_num, buffer, size);
}

int SerialCompat::available() {
    size_t available_bytes = 0;
    uart_get_buffered_data_len(uart_num, &available_bytes);
    return (int)available_bytes;
}

int SerialCompat::read() {
    uint8_t byte;
    int len = uart_read_bytes(uart_num, &byte, 1, 0);
    return len > 0 ? byte : -1;
}

int SerialCompat::peek() {
    // UART driver doesn't support peek, would need custom buffer
    // Return -1 for now (not used in serialCommands.cpp)
    return -1;
}

void SerialCompat::flush() {
    uart_wait_tx_done(uart_num, portMAX_DELAY);
}

std::string SerialCompat::readStringUntil(char terminator, uint32_t timeout_ms) {
    std::string result;
    uint32_t start = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    while (true) {
        uint32_t elapsed = (xTaskGetTickCount() * portTICK_PERIOD_MS) - start;
        if (elapsed >= timeout_ms) break;
        
        if (available() > 0) {
            int c = read();
            if (c == terminator || c == '\n' || c == '\r') break;
            if (c > 0) result += (char)c;
            start = xTaskGetTickCount() * portTICK_PERIOD_MS; // Reset timeout on data
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    
    return result;
}
