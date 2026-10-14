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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "rylr689_driver.h"
//#include "gpio_driver.h"
#include <string.h>
#include "stm32_commons.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define DEBUG_MODE__
#define WAIT_FOR_ACTION_DURATION (3U) /* 5 seconds. */


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

static uint8_t readIRBeamSensor(void);
static void waitForAction(void);


static void testApp(const rylr689_instance_t *radio, uint8_t mailboxEmptyInit, uint8_t mailboxEmptyFinal);

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
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  uint8_t status;
  rylr689_instance_t radio1 = {
		  .spiHandle = &hspi1,
		  .spiReceive = wrapper_spi_receive,
		  .spiTransmit = wrapper_spi_transmit,
		  .pinWrite = wrapper_pin_write,
		  .pinRead = wrapper_pin_read,
		  .rxBuff = &status,
		  .rxBuffSize = RX_BUFF_SIZE,
		  .txBuff = &status,
		  .txBuffSize = TX_BUFF_SIZE
  };

  radio1.csPin.pin = LoRa_Module_CS_pin_Pin;
  radio1.csPin.port = LoRa_Module_CS_pin_GPIO_Port;
  radio1.busyPin.pin = LoRa_Module_Busy_pin_Pin;
  radio1.busyPin.port = LoRa_Module_Busy_pin_GPIO_Port;
  radio1.nresetPin.pin = LoRa_Module_NRESET_pin_Pin;
  radio1.nresetPin.port = LoRa_Module_NRESET_pin_GPIO_Port;
  radio1.itPin.pin = LoRa_Module_DIOQ_Pin_Pin;
  radio1.itPin.port = LoRa_Module_DIOQ_Pin_GPIO_Port;
  radio1.rfswV1Pin.pin = LoRa_Module_RFSW_V1_PIN_POS_Pin;
  radio1.rfswV1Pin.port = LoRa_Module_RFSW_V1_PIN_POS_GPIO_Port;
  radio1.rfswV2Pin.pin = LoRa_Module_RFSW_V2_PIN_POS_Pin;
  radio1.rfswV2Pin.port = LoRa_Module_RFSW_V2_PIN_POS_GPIO_Port;

  rylr689_radio_init(&radio1);
  /* Turn off built-in LED. */
  HAL_GPIO_WritePin(BUILT_IN_LED_GPIO_Port, BUILT_IN_LED_Pin, GPIO_PIN_SET);

#ifdef DEBUG_MODE__
  while(1) {
	  testApp(&radio1, 1, 1); /* Turns on Built-in LED. */
	  /* Turn off built-in LED. */
	  HAL_GPIO_WritePin(BUILT_IN_LED_GPIO_Port, BUILT_IN_LED_Pin, GPIO_PIN_SET);
	  testApp(&radio1, 1, 0);
	  testApp(&radio1, 0, 1);
	  testApp(&radio1, 0, 0);
  }
#else
  uint8_t mailboxEmptyInit;
  uint8_t mailboxEmptyFinal;


  /* HAL RTC init sets up a timer that fires interrupt each tick,
   * this sets the WUF flag. So disable timer since it is not needed
   * yet. */
  HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);

  /* This is guaranteed to be false during initial device setup.
   * That is, no power source; completely dead from the beginning. */
	if(wakeFromStandby()) {

		/* Clear wake up flags. */
		clearWakeUpFlags();
		/* Check initial mail status. */
		mailboxEmptyInit = readIRBeamSensor();

		/* Go to sleep. */
		waitForAction();

		/* Verify final state of mailbox. */
		mailboxEmptyFinal = readIRBeamSensor();

		/* Sensor reading == 1 means beam is not broken. That is, mailbox is empty.
		* Sensor reading == 0 means beam is broken. That is, mailbox is not empty.
		* Mail present statuses:
		* 	0: Mail taken out.
		* 	1: New mail arrived.
		* 	2: Old plus possibly new mail.
		* */
		if(mailboxEmptyInit) { /* Mail is initially empty. */
			if(!mailboxEmptyFinal) { /* Mail is not empty anymore. */
				*(radio1.rxBuff) = 1;
				/* Send "New mail arrived" notification. */
				rylr689_radio_send(&radio1, 1);
			}
			else {	/* TESTING ONLY! REMOVE LATER! */

				/* Turn on Built-in LED if false alarm. */
				HAL_GPIO_WritePin(BUILT_IN_LED_GPIO_Port, BUILT_IN_LED_Pin, GPIO_PIN_RESET);
				HAL_Delay(1000);
				*(radio1.rxBuff) = 3;
				rylr689_radio_send(&radio1, 1);
			}
			/* Otherwise, do not send any notification.
			* False alarm.*/
		}
		else { /* Mail is initially not empty. */
			if(mailboxEmptyFinal) { /* Mail is now empty. */
				*(radio1.rxBuff) = 0;
				/* Send "Mail taken out" notification. */
				rylr689_radio_send(&radio1, 1);
			}
			else { /* Old and possibly new mail inside the mailbox. */
				*(radio1.rxBuff) = 2;
				/* Send "Old and possibly new mail" notification. */
				rylr689_radio_send(&radio1, 1);
			}
		}
	}

  enterStandby();
#endif /* #ifdef DEBUG_MODE__ */
  /* USER CODE END 2 */

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
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
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 0, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BUILT_IN_LED_GPIO_Port, BUILT_IN_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LoRa_Module_RFSW_V1_PIN_POS_Pin|LoRa_Module_RFSW_V2_PIN_POS_Pin|IR_BEAM_REC_PWR_Pin|IR_BEAM_TRANS_PWR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LoRa_Module_CS_pin_GPIO_Port, LoRa_Module_CS_pin_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LoRa_Module_NRESET_pin_GPIO_Port, LoRa_Module_NRESET_pin_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : BUILT_IN_LED_Pin */
  GPIO_InitStruct.Pin = BUILT_IN_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BUILT_IN_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LoRa_Module_RFSW_V1_PIN_POS_Pin LoRa_Module_RFSW_V2_PIN_POS_Pin IR_BEAM_REC_PWR_Pin IR_BEAM_TRANS_PWR_Pin
                           LoRa_Module_NRESET_pin_Pin */
  GPIO_InitStruct.Pin = LoRa_Module_RFSW_V1_PIN_POS_Pin|LoRa_Module_RFSW_V2_PIN_POS_Pin|IR_BEAM_REC_PWR_Pin|IR_BEAM_TRANS_PWR_Pin
                          |LoRa_Module_NRESET_pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : IR_BEAM_OUT_Pin */
  GPIO_InitStruct.Pin = IR_BEAM_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(IR_BEAM_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LoRa_Module_CS_pin_Pin */
  GPIO_InitStruct.Pin = LoRa_Module_CS_pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LoRa_Module_CS_pin_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LoRa_Module_Busy_pin_Pin */
  GPIO_InitStruct.Pin = LoRa_Module_Busy_pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LoRa_Module_Busy_pin_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LoRa_Module_DIOQ_Pin_Pin */
  GPIO_InitStruct.Pin = LoRa_Module_DIOQ_Pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(LoRa_Module_DIOQ_Pin_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

static void testApp(const rylr689_instance_t *radio, uint8_t mailboxEmptyInit, uint8_t mailboxEmptyFinal) {
	/* Mail present statuses:
	* 	 0: Mail taken out.
	* 	 1: New mail arrived.
	* 	 2: Old plus possibly new mail.
	* */
	if(mailboxEmptyInit) { /* Mail is initially empty. */
		if(!mailboxEmptyFinal) { /* Mail is not empty anymore. */
			*(radio->rxBuff) = 1;
			/* Send "New mail arrived" notification. */
			rylr689_radio_send(radio, 1);
		}
		else {
			/* Turn on Built-in LED if false alarm. */
			HAL_GPIO_WritePin(BUILT_IN_LED_GPIO_Port, BUILT_IN_LED_Pin, GPIO_PIN_RESET);
			*(radio->rxBuff) = 3;
			rylr689_radio_send(radio, 1);
		}
	}
	else { /* Mail is initially not empty. */
		if(mailboxEmptyFinal) { /* Mail is now empty. */
			*(radio->rxBuff) = 0;
			/* Send "Mail taken out" notification. */
			rylr689_radio_send(radio, 1);
		}
		else { /* Old and possibly new mail inside the mailbox. */
			*(radio->rxBuff) = 2;
			rylr689_radio_send(radio, 1);
			/* Send "Old and possibly new mail" notification. */
		}
	}
	HAL_Delay(3000);
}



static uint8_t readIRBeamSensor(void) {
	HAL_GPIO_WritePin(IR_BEAM_REC_PWR_GPIO_Port, IR_BEAM_REC_PWR_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(IR_BEAM_TRANS_PWR_GPIO_Port, IR_BEAM_TRANS_PWR_Pin, GPIO_PIN_SET);
	HAL_Delay(10); /* 50 ms delay. */

	/* Reset (0) means beam is broken. */
	uint8_t status = HAL_GPIO_ReadPin(IR_BEAM_OUT_GPIO_Port, IR_BEAM_OUT_Pin);

	HAL_GPIO_WritePin(IR_BEAM_REC_PWR_GPIO_Port, IR_BEAM_REC_PWR_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(IR_BEAM_TRANS_PWR_GPIO_Port, IR_BEAM_TRANS_PWR_Pin, GPIO_PIN_RESET);

	return status;
}

void waitForAction(void) {
	HAL_SuspendTick();
	HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, WAIT_FOR_ACTION_DURATION, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);

	/* Clear flags and pending interrupts to ensure it will enter Stop mode. */
	clearWakeUpFlags();
	EXTI->PR |= (0x67FFFFU);
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

	/* Sleeps. */

	/* Recover*/
	SystemClock_Config();
	HAL_ResumeTick();
	HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);

	/* Clear wake up flag for clean sleep state after waking. */
	clearWakeUpFlags();
	/* IT flags are handled automatically by HAL. */
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
