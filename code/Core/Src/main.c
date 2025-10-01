/* USER CODE BEGIN Header */
/**
  ****************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ****************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ****************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_LED_MATRIX 8
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define LED_ON(port, pin) HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)
#define LED_OFF(port, pin) HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
volatile int timer0_counter = 0, timer0_flag = 0;   // clock update
volatile int timer1_counter = 0, timer1_flag = 0;   // 7SEG
volatile int timer2_counter = 0, timer2_flag = 0;   // LED Matrix
volatile int timer3_counter = 0, timer3_flag = 0;   // Animation
const int TIMER_CYCLE = 10;                         // 10ms interrupt

int hour = 3, minute = 0, second = 56;
const int MAX_LED = 4;
int index_led = 0;
int led_buffer[4] = {0, 0, 0, 0};

/* LED MATRIX */
int index_led_matrix = 0;
// Ký tự "A"
uint8_t matrix_buffer[8] = {
    0b00011000,  //    **
    0b00100100,  //   *  *
    0b01000010,  //  *    *
    0b01000010,  //  *    *
    0b01111110,  //  *****
    0b01000010,  //  *    *
    0b01000010,  //  *    *
    0b00000000   //
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void display7SEG(int num);
void update7SEG(int index);
void updateClockBuffer();
void setTimer0(int duration);
void setTimer1(int duration);
void setTimer2(int duration);
void setTimer3(int duration);
void timer_run();
void updateLEDMatrix(int index);
void shiftLeft();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void setTimer0(int duration){
    timer0_counter = duration / TIMER_CYCLE;
    timer0_flag = 0;
}
void setTimer1(int duration){
    timer1_counter = duration / TIMER_CYCLE;
    timer1_flag = 0;
}
void setTimer2(int duration){
    timer2_counter = duration / TIMER_CYCLE;
    timer2_flag = 0;
}
void setTimer3(int duration){
    timer3_counter = duration / TIMER_CYCLE;
    timer3_flag = 0;
}
void timer_run(){
    if (timer0_counter > 0){
        timer0_counter--;
        if (timer0_counter == 0) timer0_flag = 1;
    }
    if (timer1_counter > 0){
        timer1_counter--;
        if (timer1_counter == 0) timer1_flag = 1;
    }
    if (timer2_counter > 0){
        timer2_counter--;
        if (timer2_counter == 0) timer2_flag = 1;
    }
    if (timer3_counter > 0){
        timer3_counter--;
        if (timer3_counter == 0) timer3_flag = 1;
    }
}

uint32_t segmentMap[10][7] = {
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
void display7SEG(int num) {
  GPIO_TypeDef* ports[7] = {SEG0_GPIO_Port,SEG1_GPIO_Port,SEG2_GPIO_Port,
                          SEG3_GPIO_Port,SEG4_GPIO_Port,SEG5_GPIO_Port,SEG6_GPIO_Port};
  uint16_t pins[7] = {SEG0_Pin,SEG1_Pin,SEG2_Pin,SEG3_Pin,SEG4_Pin,SEG5_Pin,SEG6_Pin};

  if (num < 0 || num > 9) return;
  for (int i=0; i<7; i++) {
    HAL_GPIO_WritePin(ports[i], pins[i], segmentMap[num][i] ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }
}

void update7SEG(int index){
	if (index < 0 || index >= MAX_LED) return;

  LED_OFF(EN0_GPIO_Port, EN0_Pin);
  LED_OFF(EN1_GPIO_Port, EN1_Pin);
  LED_OFF(EN2_GPIO_Port, EN2_Pin);
  LED_OFF(EN3_GPIO_Port, EN3_Pin);

  display7SEG(led_buffer[index]);
  switch (index){
    case 0:
      LED_ON(EN0_GPIO_Port, EN0_Pin);
      break;
    case 1:
      LED_ON(EN1_GPIO_Port, EN1_Pin);
      break;
    case 2:
      LED_ON(EN2_GPIO_Port, EN2_Pin);
      break;
    case 3:
      LED_ON(EN3_GPIO_Port, EN3_Pin);
      break;
    default:
      break;
  }
}

void updateClockBuffer() {
    led_buffer[0] = hour / 10;
    led_buffer[1] = hour % 10;
    led_buffer[2] = minute / 10;
    led_buffer[3] = minute % 10;
}

// Tắt tất cả cột
void disableAllColumns() {
    HAL_GPIO_WritePin(GPIOA, ENM0_Pin|ENM1_Pin|ENM2_Pin|ENM3_Pin|ENM4_Pin|ENM5_Pin
            |ENM6_Pin|ENM7_Pin, GPIO_PIN_SET);
}

// Bật đúng 1 cột
void enableColumn(int index) {
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

// Xuất dữ liệu của một cột
void setColumnData(int colIndex) {
    for (int row = 0; row < MAX_LED_MATRIX; row++) {
        int bit_val = (matrix_buffer[row] >> (7 - colIndex)) & 0x01;
        if (bit_val)
            HAL_GPIO_WritePin(GPIOB, (1 << (row+8)), GPIO_PIN_RESET); // LED sáng
        else
            HAL_GPIO_WritePin(GPIOB, (1 << (row+8)), GPIO_PIN_SET);   // LED tắt
    }
}

// Hàm chính
void updateLEDMatrix(int col) {
    disableAllColumns();
    setColumnData(col);
    enableColumn(col);
}

// ====== Exercise 10: Animation ======
void shiftLeft() {
    for (int row = 0; row < MAX_LED_MATRIX; row++) {
        uint8_t bit7 = (matrix_buffer[row] >> 7) & 0x01;   // lấy bit ngoài cùng bên trái
        matrix_buffer[row] <<= 1;                          // dịch trái
        matrix_buffer[row] |= bit7;                        // đưa lại bit vào bên phải (vòng tròn)
    }
}

void shiftRight(uint8_t buffer[8]) {
    for (int i = 0; i < 8; i++) {
        uint8_t lastBit = buffer[i] & 0x01;   // lấy bit ngoài cùng bên phải
        buffer[i] >>= 1;                      // dịch phải
        if (lastBit) buffer[i] |= 0x80;       // nếu bit cũ là 1 thì thêm vào bên trái
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);
	updateClockBuffer();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  setTimer0(1000); // bắt đầu đếm 1 giây
  setTimer1(50);
  setTimer2(20);    // quét LED matrix 2ms
  setTimer3(300);
  while (1)
  {
	 if (timer0_flag == 1) {
		  timer0_flag = 0;
		  setTimer0(1000);

		  second++;
		  if (second >= 60) {
			  second = 0;
			  minute++;
		  }
		  if (minute >= 60) {
			  minute = 0;
			  hour++;
		  }
		  if (hour >= 24) {
			  hour = 0;
		  }
		  updateClockBuffer();   // cập nhật dữ liệu vào led_buffer

		  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4); // led DOT
		  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // led PA5
	  }

	  // Quét LED: mỗi 20ms
	  if (timer1_flag == 1) {
		  timer1_flag = 0;
		  setTimer1(50);
		  update7SEG(index_led);
		  index_led = (index_led + 1) % MAX_LED;
	  }

	  /* LED MATRIX */
		if (timer2_flag == 1){
			timer2_flag = 0;
			setTimer2(20);
			updateLEDMatrix(index_led_matrix);
			index_led_matrix++;
			if (index_led_matrix >= MAX_LED_MATRIX) index_led_matrix = 0;
		}

		// Animation cho LED matrix
	  if (timer3_flag == 1){
		  timer3_flag = 0;
		  setTimer3(300);   // dịch trái mỗi 300ms
		  shiftLeft();
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, ENM0_Pin|ENM1_Pin|DOT_Pin|LED_RED_Pin
                          |EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin
                          |ENM2_Pin|ENM3_Pin|ENM4_Pin|ENM5_Pin
                          |ENM6_Pin|ENM7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SEG0_Pin|SEG1_Pin|SEG2_Pin|ROW2_Pin
                          |ROW3_Pin|ROW4_Pin|ROW5_Pin|ROW6_Pin
                          |ROW7_Pin|SEG3_Pin|SEG4_Pin|SEG5_Pin
                          |SEG6_Pin|ROW0_Pin|ROW1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : ENM0_Pin ENM1_Pin DOT_Pin LED_RED_Pin
                           EN0_Pin EN1_Pin EN2_Pin EN3_Pin
                           ENM2_Pin ENM3_Pin ENM4_Pin ENM5_Pin
                           ENM6_Pin ENM7_Pin */
  GPIO_InitStruct.Pin = ENM0_Pin|ENM1_Pin|DOT_Pin|LED_RED_Pin
                          |EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin
                          |ENM2_Pin|ENM3_Pin|ENM4_Pin|ENM5_Pin
                          |ENM6_Pin|ENM7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SEG0_Pin SEG1_Pin SEG2_Pin ROW2_Pin
                           ROW3_Pin ROW4_Pin ROW5_Pin ROW6_Pin
                           ROW7_Pin SEG3_Pin SEG4_Pin SEG5_Pin
                           SEG6_Pin ROW0_Pin ROW1_Pin */
  GPIO_InitStruct.Pin = SEG0_Pin|SEG1_Pin|SEG2_Pin|ROW2_Pin
                          |ROW3_Pin|ROW4_Pin|ROW5_Pin|ROW6_Pin
                          |ROW7_Pin|SEG3_Pin|SEG4_Pin|SEG5_Pin
                          |SEG6_Pin|ROW0_Pin|ROW1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2) {
		timer_run(); 						// software timer chạy mỗi 10ms
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
