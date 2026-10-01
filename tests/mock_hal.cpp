/**
 * @file mock_hal.cpp
 * @brief Mock HAL implementation for unit testing
 *
 * This file provides stub implementations of STM32 HAL functions
 * needed by the TPS55289 driver for unit testing without real hardware.
 *
 * NOTE: This file depends on tps55289.hpp defining the stub types when
 * TPS55289_UNIT_TEST is defined.
 */

#include "tps55289.hpp"
#include <cstdint>

// Global I2C handle instance (referenced by test_main.cpp)
I2C_HandleTypeDef hi2c1;

// Mock HAL functions - all stub implementations
extern "C" {

void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c) {
    (void)hi2c;
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c) {
    (void)hi2c;
}

HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef* hi2c, uint16_t DevAddress,
                                     uint16_t MemAddress, uint16_t MemAddSize,
                                     uint8_t* pData, uint16_t Size, uint32_t Timeout) {
    (void)hi2c;
    (void)DevAddress;
    (void)MemAddress;
    (void)MemAddSize;
    (void)pData;
    (void)Size;
    (void)Timeout;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef* hi2c, uint16_t DevAddress,
                                    uint16_t MemAddress, uint16_t MemAddSize,
                                    uint8_t* pData, uint16_t Size, uint32_t Timeout) {
    (void)hi2c;
    (void)DevAddress;
    (void)MemAddress;
    (void)MemAddSize;
    (void)pData;
    (void)Size;
    (void)Timeout;
    if (pData) {
        *pData = 0;
    }
    return HAL_OK;
}

void HAL_Init() {
}

void HAL_MspInit() {
}

void HAL_IncTick() {
}

void HAL_Delay(uint32_t Delay) {
    (void)Delay;
}

HAL_StatusTypeDef HAL_RCC_OscConfig(void* RCC_OscInitStruct) {
    (void)RCC_OscInitStruct;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_RCC_ClockConfig(void* RCC_ClkInitStruct, uint32_t FlashLatency) {
    (void)RCC_ClkInitStruct;
    (void)FlashLatency;
    return HAL_OK;
}

void HAL_PWREx_ControlVoltageScaling(int Voltage) {
    (void)Voltage;
}

void HAL_GPIO_Init(void* GPIOx, void* GPIO_Init) {
    (void)GPIOx;
    (void)GPIO_Init;
}

void HAL_GPIO_DeInit(void* GPIOx, uint32_t GPIO_Pin) {
    (void)GPIOx;
    (void)GPIO_Pin;
}

void HAL_I2C_Init(I2C_HandleTypeDef* hi2c) {
    (void)hi2c;
}

HAL_StatusTypeDef HAL_I2CEx_ConfigAnalogFilter(I2C_HandleTypeDef* hi2c, uint32_t AnalogFilter) {
    (void)hi2c;
    (void)AnalogFilter;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2CEx_ConfigDigitalFilter(I2C_HandleTypeDef* hi2c, uint32_t DigitalFilter) {
    (void)hi2c;
    (void)DigitalFilter;
    return HAL_OK;
}

void Error_Handler() {
    // No-op for testing - would normally halt
}

void __disable_irq() {
    // No-op for testing
}

void __enable_irq() {
    // No-op for testing
}

// Additional stubs that might be referenced
void HAL_GPIO_TogglePin(void* GPIOx, uint32_t GPIO_Pin) {
    (void)GPIOx;
    (void)GPIO_Pin;
}

void HAL_GPIO_WritePin(void* GPIOx, uint32_t GPIO_Pin, uint32_t PinState) {
    (void)GPIOx;
    (void)GPIO_Pin;
    (void)PinState;
}

uint32_t HAL_GetTick() {
    return 0;
}

} // extern "C"
