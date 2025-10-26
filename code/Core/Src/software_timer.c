/*
 * software_timer.c
 *
 *  Created on: Oct 14, 2025
 *      Author: DoDo
 */

#include "software_timer.h"

// Mảng để lưu trữ các bộ đếm thời gian
static volatile int timer_counters[NO_OF_TIMERS];
// Mảng để lưu trữ các cờ báo hiệu timer, được khai báo extern trong file .h
volatile int timer_flags[NO_OF_TIMERS];

const int TIMER_CYCLE = 10; // Chu kỳ ngắt timer phần cứng là 10ms

// Hàm setTimer mới, nhận vào chỉ số của timer và thời gian đếm
void setTimer(int index, int duration) {
    // Kiểm tra xem chỉ số có hợp lệ không
    if (index >= 0 && index < NO_OF_TIMERS) {
        timer_counters[index] = duration / TIMER_CYCLE;
        timer_flags[index] = 0;
    }
}

// timer_run giờ sẽ duyệt qua một vòng lặp, rất dễ để thêm timer mới sau này
void timer_run() {
    for (int i = 0; i < NO_OF_TIMERS; i++) {
        if (timer_counters[i] > 0) {
            timer_counters[i]--;
            if (timer_counters[i] <= 0) {
                timer_flags[i] = 1;
            }
        }
    }
}
