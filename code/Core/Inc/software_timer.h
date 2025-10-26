/*
 * software_timer.h
 *
 *  Created on: Oct 14, 2025
 *      Author: DoDo
 */

#ifndef SRC_SOFTWARE_TIMER_H_
#define SRC_SOFTWARE_TIMER_H_

#include "main.h" // Cần thiết cho các định nghĩa HAL cơ bản

#define NO_OF_TIMERS 4 // Định nghĩa số lượng timer chúng ta sử dụng

// Dùng một mảng để quản lý tất cả các cờ timer
extern volatile int timer_flags[NO_OF_TIMERS];

// Hàm set timer đã được tối ưu
void setTimer(int index, int duration);

// Hàm timer_run không thay đổi
void timer_run();


#endif /* SRC_SOFTWARE_TIMER_H_ */
