/**
 * ESP-IDF Serial Compatibility Layer
 *
 * Provides Arduino-like Serial interface using ESP-IDF uart_driver
 * Compatible with existing Arduino code during migration phase
 */

#pragma once

#include "driver/uart.h"
#include "esp_log.h"
#include <cstdint>
#include <string>

class SerialCompat {
private:
    uart_port_t uart_num;
    uint8_t rx_buffer[128];
    static constexpr const char *TAG = "SerialCompat";

public:
    SerialCompat(uart_port_t port = UART_NUM_0);

    void begin(uint32_t baud, uint32_t config = UART_DATA_8_BITS | UART_STOP_BITS_1 | UART_PARITY_DISABLE);
    void end();

    // Write methods
    size_t print(const char *str);
    size_t print(const std::string &str);
    size_t print(int num);
    size_t print(unsigned int num);
    size_t print(float num, int digits = 2);

    size_t println(const char *str = "");
    size_t println(const std::string &str);
    size_t println(int num);
    size_t println(unsigned int num);
    size_t println(float num, int digits = 2);

    size_t write(uint8_t byte);
    size_t write(const uint8_t *buffer, size_t size);

    // Read methods
    int available();
    int read();
    int peek();
    void flush();

    // Arduino String compatibility
    std::string readStringUntil(char terminator, uint32_t timeout_ms = 300);
};

// Global instance for backward compatibility
extern SerialCompat Serial;
