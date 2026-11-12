/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
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
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Wave_Display.h"
#include "Character_Display.h"
#include "Voltage_Encoder.h"
#include "Trigger_Button.h"
#include "TimeBase.h"
#include "Trigger_Encoder.h"
#include "ADC.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
volatile uint8_t lcd_stage = 0;


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
uint32_t MyBufferTask00[ 256 ];
osStaticThreadDef_t MycontrolBlocTask00;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_mem = &MyBufferTask00[0],
  .stack_size = sizeof(MyBufferTask00),
  .cb_mem = &MycontrolBlocTask00,
  .cb_size = sizeof(MycontrolBlocTask00),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myTask02 */
osThreadId_t myTask02Handle;
uint32_t MyBufferTask02[ 256 ];
osStaticThreadDef_t MycontrolBlocTask02;
const osThreadAttr_t myTask02_attributes = {
  .name = "myTask02",
  .stack_mem = &MyBufferTask02[0],
  .stack_size = sizeof(MyBufferTask02),
  .cb_mem = &MycontrolBlocTask02,
  .cb_size = sizeof(MycontrolBlocTask02),
  .priority = (osPriority_t) osPriorityBelowNormal1,
};
/* Definitions for myTask03 */
osThreadId_t myTask03Handle;
uint32_t MyBufferTask03[ 256 ];
osStaticThreadDef_t MycontrolBlocTask03;
const osThreadAttr_t myTask03_attributes = {
  .name = "myTask03",
  .stack_mem = &MyBufferTask03[0],
  .stack_size = sizeof(MyBufferTask03),
  .cb_mem = &MycontrolBlocTask03,
  .cb_size = sizeof(MycontrolBlocTask03),
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for myTask04 */
osThreadId_t myTask04Handle;
uint32_t MyBufferTask04[ 256 ];
osStaticThreadDef_t MycontrolBlocTask04;
const osThreadAttr_t myTask04_attributes = {
  .name = "myTask04",
  .stack_mem = &MyBufferTask04[0],
  .stack_size = sizeof(MyBufferTask04),
  .cb_mem = &MycontrolBlocTask04,
  .cb_size = sizeof(MycontrolBlocTask04),
  .priority = (osPriority_t) osPriorityBelowNormal5,
};
/* Definitions for myTask05 */
osThreadId_t myTask05Handle;
uint32_t MyBufferTask05[ 256 ];
osStaticThreadDef_t MycontrolBlocTask05;
const osThreadAttr_t myTask05_attributes = {
  .name = "myTask05",
  .stack_mem = &MyBufferTask05[0],
  .stack_size = sizeof(MyBufferTask05),
  .cb_mem = &MycontrolBlocTask05,
  .cb_size = sizeof(MycontrolBlocTask05),
  .priority = (osPriority_t) osPriorityBelowNormal3,
};
/* Definitions for myTask06 */
osThreadId_t myTask06Handle;
uint32_t MyBufferTask06[ 256 ];
osStaticThreadDef_t MycontrolBlocTask06;
const osThreadAttr_t myTask06_attributes = {
  .name = "myTask06",
  .stack_mem = &MyBufferTask06[0],
  .stack_size = sizeof(MyBufferTask06),
  .cb_mem = &MycontrolBlocTask06,
  .cb_size = sizeof(MycontrolBlocTask06),
  .priority = (osPriority_t) osPriorityBelowNormal5,
};
/* Definitions for myTask07 */
osThreadId_t myTask07Handle;
uint32_t MyBufferTask07[ 256 ];
osStaticThreadDef_t MycontrolBlocTask07;
const osThreadAttr_t myTask07_attributes = {
  .name = "myTask07",
  .stack_mem = &MyBufferTask07[0],
  .stack_size = sizeof(MyBufferTask07),
  .cb_mem = &MycontrolBlocTask07,
  .cb_size = sizeof(MycontrolBlocTask07),
  .priority = (osPriority_t) osPriorityBelowNormal5,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void Character_Reading(void)
{
  ; /*TO DO*/
}
/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
	WaveD_Init();
	Scope_ADC_Init();
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of myTask02 */
  myTask02Handle = osThreadNew(WaveD_Task, NULL, &myTask02_attributes);

  /* creation of myTask03 */
  myTask03Handle = osThreadNew(Task_Character_Display, NULL, &myTask03_attributes);

  /* creation of myTask04 */
  myTask04Handle = osThreadNew(Encoder_Task, NULL, &myTask04_attributes);

  /* creation of myTask05 */
  myTask05Handle = osThreadNew(Trigger_Update_Task, NULL, &myTask05_attributes);

  /* creation of myTask06 */
  myTask06Handle = osThreadNew(Time_Base_Task, NULL, &myTask06_attributes);

  /* creation of myTask07 */
  myTask07Handle = osThreadNew(Trigger_Encoder_Task, NULL, &myTask07_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartDefaultTask */
/**
* @brief Function implementing the defaultTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN defaultTask */


	Scope_Task_Body();
  /* USER CODE END defaultTask */
}

/* USER CODE BEGIN Header_WaveD_Task */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_WaveD_Task */
void WaveD_Task(void *argument)
{
  /* USER CODE BEGIN myTask02 */
  WaveD_Task_Body();
  /* USER CODE END myTask02 */
}

/* USER CODE BEGIN Header_Task_Character_Display */
/**
* @brief Function implementing the myTask03 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Task_Character_Display */
void Task_Character_Display(void *argument)
{
  /* USER CODE BEGIN myTask03 */

	extern uint16_t current;
	extern uint16_t time;
	extern uint8_t mode;
	extern uint16_t start_point;
	uint8_t  new_mode    = 0xFF;
	uint16_t new_current = 0xFFFF;
	uint16_t new_time    = 0xFFFF;
	uint16_t new_start   = 0xFFFF;
	lcd_stage = 1;
	Character_Display_Init();
	lcd_stage = 2;
	Character_Display_ShowZero();
	lcd_stage = 3;

  /* Infinite loop */
  for(;;)
  {
	  osDelay(50);
	  lcd_stage = 4;
	  // TODO, MAKE GO FASTER Update_Character_Display_Time(time);

	  if(new_mode!=mode)
	  {
		  Update_Character_Display_Trigger(mode);
	  }

	  if(new_current!=current)
	  {
		  Update_Character_Display_Voltage(current);
	  }

	  if(new_time!=time)
	  {
		  Update_Character_Display_Time(time);
	  }

	  if(new_start!=start_point)
	  {
		  Update_Character_Display_Volt_Start(start_point);
	  }

	  new_mode = mode;
	  new_time = time;
	  new_current = current;
	  new_start = start_point;

  }
  /* USER CODE END myTask03 */
}

/* USER CODE BEGIN Header_Encoder_Task */
/**
* @brief Function implementing the myTask04 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Encoder_Task */
void Encoder_Task(void *argument)
{
  /* USER CODE BEGIN myTask04 */
  /* Infinite loop */
  for(;;)
  {
	Raise_Lower_Voltage();
    osDelay(1);
  }
  /* USER CODE END myTask04 */
}

/* USER CODE BEGIN Header_Trigger_Update_Task */
/**
* @brief Function implementing the myTask05 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Trigger_Update_Task */
void Trigger_Update_Task(void *argument)
{
  /* USER CODE BEGIN myTask05 */
  /* Infinite loop */
  for(;;)
  {
	Rise_Or_Fall();
    osDelay(1);
  }
  /* USER CODE END myTask05 */
}

/* USER CODE BEGIN Header_Time_Base_Task */
/**
* @brief Function implementing the myTask06 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Time_Base_Task */
void Time_Base_Task(void *argument)
{
  /* USER CODE BEGIN myTask06 */
  /* Infinite loop */
  for(;;)
  {
	Raise_Lower_Time();
    osDelay(1);
  }
  /* USER CODE END myTask06 */
}

/* USER CODE BEGIN Header_Trigger_Encoder_Task */
/**
* @brief Function implementing the myTask07 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Trigger_Encoder_Task */
void Trigger_Encoder_Task(void *argument)
{
  /* USER CODE BEGIN myTask07 */
  /* Infinite loop */
  for(;;)
  {
	Raise_Lower_Start();
    osDelay(1);
  }
  /* USER CODE END myTask07 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void Transmit_Spi()
{
		/*TODO*/
}
/* USER CODE END Application */

