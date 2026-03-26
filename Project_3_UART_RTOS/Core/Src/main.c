/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include "queue.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
enum {
	LED_ON = 0,
	LED_OFF,
	LOG,
	ERROR_COMMAND,
	UNKNOWN_COMMAND,
};
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* Definitions for UART_Task */
osThreadId_t UART_TaskHandle;
const osThreadAttr_t UART_Task_attributes = {
  .name = "UART_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Process_task */
osThreadId_t Process_taskHandle;
const osThreadAttr_t Process_task_attributes = {
  .name = "Process_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LEDControl_task */
osThreadId_t LEDControl_taskHandle;
const osThreadAttr_t LEDControl_task_attributes = {
  .name = "LEDControl_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Log_task */
osThreadId_t Log_taskHandle;
const osThreadAttr_t Log_task_attributes = {
  .name = "Log_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Definitions for received_ISR */
QueueHandle_t received_ISRHandle;



/* Definitions for received_command */
osMessageQueueId_t received_commandHandle;
const osMessageQueueAttr_t received_command_attributes = {
  .name = "received_command"
};

/* Definitions for led_control */
osMessageQueueId_t led_controlHandle;
const osMessageQueueAttr_t led_control_attributes = {
  .name = "led_control"
};

/* Definitions for log */
osMessageQueueId_t logHandle;
const osMessageQueueAttr_t log_attributes = {
  .name = "log"
};

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
void Start_UART_Task(void *argument);
void Start_Process_Task(void *argument);
void Start_LEDControl_Task(void *argument);
void Start_Log_Task(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t rx_byte; //store byte from RX ISR
volatile uint8_t tx_busy_flag = 0; //tx busy flag
uint8_t error_flag = 0; //overflow flag
uint8_t received_command_done = 0;
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if(huart->Instance == USART1) {
		tx_busy_flag = 0;
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(huart->Instance == USART1) {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xQueueSendFromISR(received_ISRHandle,&rx_byte, &xHigherPriorityTaskWoken);
		HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of received_done */
	
  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
	/* creation of received_ISR */
  received_ISRHandle = xQueueCreate(64, sizeof(uint8_t));
  /* creation of received_command */
  received_commandHandle = osMessageQueueNew (16, sizeof(uint16_t), &received_command_attributes);
	/* creation of led_control */
  led_controlHandle = osMessageQueueNew (16, sizeof(uint16_t), &led_control_attributes);
	/* creation of log */
  logHandle = osMessageQueueNew (16, sizeof(uint16_t), &log_attributes);
  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of UART_Task */
  UART_TaskHandle = osThreadNew(Start_UART_Task, NULL, &UART_Task_attributes);

  /* creation of Process_task */
  Process_taskHandle = osThreadNew(Start_Process_Task, NULL, &Process_task_attributes);

  /* creation of LEDControl_task */
  LEDControl_taskHandle = osThreadNew(Start_LEDControl_Task, NULL, &LEDControl_task_attributes);

  /* creation of Log_task */
  Log_taskHandle = osThreadNew(Start_Log_Task, NULL, &Log_task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_Start_UART_Task */
/**
  * @brief  Function implementing the UART_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Start_UART_Task */
void Start_UART_Task(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	uint8_t byte = 0;
	static char command[8] = {0};
	uint8_t index = 0;
  for(;;)
	
  {		
		if(xQueueReceive(received_ISRHandle,&byte,portMAX_DELAY) == pdTRUE){
			if(byte == '\r') {
				command[index] = '\0';
				uint8_t cmd = 0;
				if (error_flag) {
					cmd = ERROR_COMMAND;
					error_flag = 0;
				} else {
					if (strcmp(command,"LED_ON") == 0) {
						cmd = LED_ON;
					} else if (strcmp(command,"LED_OFF") == 0) {
						cmd = LED_OFF;
					} else if (strcmp(command,"LOG") == 0) {
						cmd = LOG;
					} else {
						cmd = UNKNOWN_COMMAND;
					}
				}
				osMessageQueuePut(received_commandHandle,&cmd,0,osWaitForever);
				index = 0;
			} else {
				if (index < sizeof(command) - 1) {
					command[index++] = byte;
				} else { //overflow
					error_flag = 1;
					memset(command, 0,sizeof(command));
					index = 0;
				}
			}	
		}
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_Start_Process_Task */
/**
* @brief Function implementing the Process_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Process_Task */
void Start_Process_Task(void *argument)
{
  /* USER CODE BEGIN Start_Process_Task */
  /* Infinite loop */
	uint8_t command = 0;
  for(;;)
  {
		osMessageQueueGet(received_commandHandle,&command,NULL,osWaitForever);
		switch (command) {
			case LED_ON:
				osMessageQueuePut(led_controlHandle,&command,0,0);
				osMessageQueuePut(logHandle,&command,0,0);
				break;
			case LED_OFF:
				osMessageQueuePut(led_controlHandle,&command,0,0);
				osMessageQueuePut(logHandle,&command,0,0);
				break;
			case LOG:
				osMessageQueuePut(logHandle,&command,0,0);
				break;
			case ERROR_COMMAND:
				osMessageQueuePut(logHandle,&command,0,0);
				break;
			case UNKNOWN_COMMAND:
				osMessageQueuePut(logHandle,&command,0,0);
				break;
			default:
				break;
		}
  }
  /* USER CODE END Start_Process_Task */
}

/* USER CODE BEGIN Header_Start_LEDControl_Task */
/**
* @brief Function implementing the LEDControl_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_LEDControl_Task */
void Start_LEDControl_Task(void *argument)
{
  /* USER CODE BEGIN Start_LEDControl_Task */
  /* Infinite loop */
	int8_t buffer = 0;
  for(;;)
  {
		osMessageQueueGet(led_controlHandle,&buffer,NULL,osWaitForever);
		switch (buffer) {
			case LED_OFF:
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,1);
				break;
			case LED_ON:
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,0);
				break;
			default:
				break;
		}
  }
  /* USER CODE END Start_LEDControl_Task */
}

/* USER CODE BEGIN Header_Start_Log_Task */
/**
* @brief Function implementing the Log_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Log_Task */
void Start_Log_Task(void *argument)
{
  /* USER CODE BEGIN Start_Log_Task */
  /* Infinite loop */
	int8_t buffer = 0;
  for(;;)
  {
		osMessageQueueGet(logHandle,&buffer,NULL,osWaitForever);
		if (tx_busy_flag == 0) {
			tx_busy_flag = 1;
			switch (buffer) {
				case LED_OFF:
					HAL_UART_Transmit_IT(&huart1,(const uint8_t*)"OFF_ok\n",strlen("OFF_ok\n"));
					break;
				case LED_ON:
					HAL_UART_Transmit_IT(&huart1,(const uint8_t*)"ON_ok\n",strlen("ON_ok\n"));
					break;
				case ERROR_COMMAND:
					HAL_UART_Transmit_IT(&huart1,(const uint8_t*)"error_command\n",strlen("error_command\n"));
					break;
				case UNKNOWN_COMMAND:
					HAL_UART_Transmit_IT(&huart1,(const uint8_t*)"unknown_command\n",strlen("unknown_command\n"));
					break;
				case LOG:
					const char *msg = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) ?
                  "LED is OFF\n" : "LED is ON\n";

					HAL_UART_Transmit_IT(&huart1, (uint8_t *)msg, strlen(msg));
					break;
				default:
					break;
			}
		}
  }
  /* USER CODE END Start_Log_Task */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
#ifdef USE_FULL_ASSERT
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
