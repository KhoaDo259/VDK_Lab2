/*
 * display_7seg.c
 *
 *  Created on: Oct 14, 2025
 *      Author: DoDo
 */

#include "display_7seg.h"

// Định nghĩa các macro cục bộ
#define LED_ON(port, pin) HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)
#define LED_OFF(port, pin) HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)

// Các biến và hằng số private cho module này (static)
static const int MAX_LED = 4;
static int index_led = 0;
static int led_buffer[4] = {0, 0, 0, 0};

// Bảng mã cho LED 7 đoạn (private)
static const uint32_t segmentMap[10][7] = {
    // a,b,c,d,e,f,g
    {0,0,0,0,0,0,1}, // 0
    {1,0,0,1,1,1,1}, // 1
    {0,0,1,0,0,1,0}, // 2
    {0,0,0,0,1,1,0}, // 3
    {1,0,0,1,1,0,0}, // 4
    {0,1,0,0,1,0,0}, // 5
    {0,1,0,0,0,0,0}, // 6
    {0,0,0,1,1,1,1}, // 7
    {0,0,0,0,0,0,0}, // 8
    {0,0,0,0,1,0,0}  // 9
};

// Hàm này chỉ được gọi nội bộ bởi update7SEG, nên để là static
static void display7SEG(int num) {
    GPIO_TypeDef* ports[7] = {SEG0_GPIO_Port,SEG1_GPIO_Port,SEG2_GPIO_Port,
                              SEG3_GPIO_Port,SEG4_GPIO_Port,SEG5_GPIO_Port,SEG6_GPIO_Port};
    uint16_t pins[7] = {SEG0_Pin,SEG1_Pin,SEG2_Pin,SEG3_Pin,SEG4_Pin,SEG5_Pin,SEG6_Pin};

    if (num < 0 || num > 9) return;
    for (int i=0; i<7; i++) {
        HAL_GPIO_WritePin(ports[i], pins[i], segmentMap[num][i] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

// Cập nhật buffer hiển thị.
// Tốt hơn là truyền tham số vào thay vì dùng biến global hour, minute
void updateClockBuffer(int hour, int minute) {
    led_buffer[0] = hour / 10;
    led_buffer[1] = hour % 10;
    led_buffer[2] = minute / 10;
    led_buffer[3] = minute % 10;
}

// Hàm quét LED, nên đổi tên thành `scan7SEG` hoặc `run7SEG` để rõ nghĩa hơn
void update7SEG(){
    if (index_led < 0 || index_led >= MAX_LED) return;

    // Tắt hết các EN
    LED_OFF(EN0_GPIO_Port, EN0_Pin);
    LED_OFF(EN1_GPIO_Port, EN1_Pin);
    LED_OFF(EN2_GPIO_Port, EN2_Pin);
    LED_OFF(EN3_GPIO_Port, EN3_Pin);

    // Hiển thị số và bật EN tương ứng
    display7SEG(led_buffer[index_led]);
    switch (index_led){
        case 0: LED_ON(EN0_GPIO_Port, EN0_Pin); break;
        case 1: LED_ON(EN1_GPIO_Port, EN1_Pin); break;
        case 2: LED_ON(EN2_GPIO_Port, EN2_Pin); break;
        case 3: LED_ON(EN3_GPIO_Port, EN3_Pin); break;
        default: break;
    }

    // Chuyển sang LED tiếp theo cho lần gọi sau
    index_led = (index_led + 1) % MAX_LED;
}
