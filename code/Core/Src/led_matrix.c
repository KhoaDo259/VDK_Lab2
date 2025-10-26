/*
 * led_matrix.c
 *
 *  Created on: Oct 14, 2025
 *      Author: DoDo
 */

#include "led_matrix.h"
#include <stdint.h>

// Các biến và hằng số private cho module
#define MAX_LED_MATRIX 8
static int index_led_matrix = 0;

static uint8_t matrix_buffer[8] = {
    0b00011000,  //    **
    0b00100100,  //   * *
    0b01000010,  //  * *
    0b01000010,  //  * *
    0b01111110,  //  ******
    0b01000010,  //  * *
    0b01000010,  //  * *
    0b00000000   //
};

// --- CẬP NHẬT THEO LOGIC TÍCH CỰC ÂM (ACTIVE LOW) ---

// Để tắt tất cả các cột, ta đưa các chân ENM lên mức CAO (SET)
static void disableAllColumns() {
    HAL_GPIO_WritePin(GPIOA, ENM0_Pin|ENM1_Pin|ENM2_Pin|ENM3_Pin|ENM4_Pin|ENM5_Pin
                            |ENM6_Pin|ENM7_Pin, GPIO_PIN_SET);
}

// Để bật một cột, ta kéo chân ENM tương ứng xuống mức THẤP (RESET)
static void enableColumn(int index) {
    switch(index){
        case 0: HAL_GPIO_WritePin(GPIOA, ENM0_Pin, GPIO_PIN_RESET); break;
        case 1: HAL_GPIO_WritePin(GPIOA, ENM1_Pin, GPIO_PIN_RESET); break;
        case 2: HAL_GPIO_WritePin(GPIOA, ENM2_Pin, GPIO_PIN_RESET); break;
        case 3: HAL_GPIO_WritePin(GPIOA, ENM3_Pin, GPIO_PIN_RESET); break;
        case 4: HAL_GPIO_WritePin(GPIOA, ENM4_Pin, GPIO_PIN_RESET); break;
        case 5: HAL_GPIO_WritePin(GPIOA, ENM5_Pin, GPIO_PIN_RESET); break;
        case 6: HAL_GPIO_WritePin(GPIOA, ENM6_Pin, GPIO_PIN_RESET); break;
        case 7: HAL_GPIO_WritePin(GPIOA, ENM7_Pin, GPIO_PIN_RESET); break;
        default: break;
    }
}

static void setColumnData(int colIndex) {
    // Mảng ánh xạ từ hàng logic (0-7) sang chân GPIO vật lý (phần này đã sửa đúng)
    uint16_t row_pins[MAX_LED_MATRIX] = {
        ROW0_Pin, ROW1_Pin, ROW2_Pin, ROW3_Pin,
        ROW4_Pin, ROW5_Pin, ROW6_Pin, ROW7_Pin
    };

    for (int row = 0; row < MAX_LED_MATRIX; row++) {
        int bit_val = (matrix_buffer[row] >> (7 - colIndex)) & 0x01;

        if (bit_val) {
            // BẬT LED: Kéo chân ROW tương ứng xuống mức THẤP (RESET)
            HAL_GPIO_WritePin(GPIOB, row_pins[row], GPIO_PIN_RESET);
        } else {
            // TẮT LED: Đưa chân ROW tương ứng lên mức CAO (SET)
            HAL_GPIO_WritePin(GPIOB, row_pins[row], GPIO_PIN_SET);
        }
    }
}


// Hàm quét LED Matrix, không cần thay đổi
void updateLEDMatrix() {
    disableAllColumns();
    setColumnData(index_led_matrix);
    enableColumn(index_led_matrix);

    index_led_matrix++;
    if (index_led_matrix >= MAX_LED_MATRIX) {
        index_led_matrix = 0;
    }
}

// Hàm public cho hiệu ứng animation, không cần thay đổi
void shiftLeft() {
    for (int row = 0; row < MAX_LED_MATRIX; row++) {
        uint8_t firstBit = (matrix_buffer[row] >> 7) & 0x01;
        matrix_buffer[row] <<= 1;
        matrix_buffer[row] |= firstBit;
    }
}

void shiftRight() {
    for (int i = 0; i < 8; i++) {
        uint8_t lastBit = matrix_buffer[i] & 0x01;
        matrix_buffer[i] >>= 1;
        if (lastBit) matrix_buffer[i] |= 0x80;
    }
}
