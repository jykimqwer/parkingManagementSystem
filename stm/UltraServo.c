/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define STK_CTRL 	*(volatile unsigned int*)0xE000E010
#define STK_LOAD 	*(volatile unsigned int*)0xE000E014
#define STK_VAL 		*(volatile unsigned int*)0xE000E018
#define STK_CALIB  *(volatile unsigned int*)0xE000E01C

#define ENABLE 0

#define BUFFER_SIZE  64

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t rxData;
uint8_t rxBuffer[BUFFER_SIZE];
uint8_t rxIndex = 0;

uint32_t echoTime = 0;
uint8_t cnt = 0;

FILE __stdout;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void delay_us(uint16_t);
void HAL_Delay_Porting();
void SysTic_Init();
int Ultra();

int fputc(int ch, FILE* stream)
{
   HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, 0xffff);
   return ch;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	// Timer3 = Ultra1(PB15->Trig1(out), PB14->Echo1(in)) 
	// Timer4_ch1  = Motor1(PB6)

	/*Ultra1(in)*/
	TIM3->PSC = 100-1;
	/*Motor1(in)*/
	TIM4->PSC = 2000-1;
	TIM4->ARR = 1000-1;
	
  /* USER CODE END 2 */
	
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	HAL_TIM_Base_Start(&htim3);
	HAL_UART_Receive_IT(&huart2, &rxData, sizeof(rxData)); 

  while (1)
	{
    /* USER CODE END WHILE */
		//in
		int dis = Ultra();
		printf("dis = %d cm\r\n", dis);
		HAL_Delay(500);

		if(rxData == '1' && dis <= 50)
		{
			HAL_Delay(100);
			HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1); 
			for(int i = 0; i < 70; i += 2)
			{
				TIM4->CCR1 = cnt;
				cnt = i;
			}
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);
			HAL_Delay(3000);
		}
		else if(dis > 50)
		{
			HAL_Delay(100);
			HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
			for(int i = 115; i > 0; i -= 10)
			{
				TIM4->CCR1 = cnt;
				cnt = i;
			}
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);
		}
		HAL_Delay(100);	
		rxData = 0;
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
		if (huart->Instance == USART2){
      HAL_UART_Receive_IT(&huart2, &rxData, sizeof(rxData));
   }
}
void delay_us(uint16_t us) 
{
		__HAL_TIM_SET_COUNTER(&htim3, 0); 
	while(__HAL_TIM_GET_COUNTER(&htim3)< us ); 
}

void SysTic_Init()
{
	STK_LOAD =  100-1; // 1us
	STK_VAL = 0;
	STK_CTRL = 6;
	uwTick = 0;
}

void HAL_Delay_Porting()
{
	STK_LOAD = 100000 - 1;
	STK_CTRL |= 7;
}

int Ultra()
{
	SysTic_Init();
	HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, 1);
	delay_us(15); 
	HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, 0);
	while (HAL_GPIO_ReadPin(Echo_GPIO_Port, Echo_Pin) == 0);
	STK_CTRL |= (1 << ENABLE); 		
	while(HAL_GPIO_ReadPin(Echo_GPIO_Port, Echo_Pin) == 1);
	echoTime = HAL_GetTick(); 	
	STK_CTRL &= ~(1<<ENABLE);
		
	/*340m/s - >  340x100cm/1000000us -> 0.034cm/us*/
	int distance = echoTime / 2 * 0.034;
	//printf("Distance = %d cm \r\n", distance);
		
	HAL_Delay_Porting();
	HAL_Delay(200);
	
	return distance;
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
