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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
//#include "dht.h"
#include "clcd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
#define ARR_CNT 6
#define CMD_SIZE 50

#define FLOOR_SIZE 	5
#define ACTUATOR_SIZE  4
#define MONITOR		0
#define CONTROL		1
#define ALARM		2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
uint8_t rx2char;
volatile unsigned char rx2Flag = 0;
volatile char rx2Data[50];
volatile unsigned char btFlag = 0;
uint8_t btchar;
char btData[50];
volatile int keyNo = 9;
volatile int tim3Flag1Sec;
volatile unsigned int tim3Cnt;
volatile int updateTimeFlag = 1;
volatile uint32_t lastButtonPressTime = 0;
int modeState;
int floorState = 1; //0F -> ALL
int actuatorState;
int onoffState;

typedef struct {
  int year;
  int month;
  int day;
  int hour;
  int min;
  int sec;
} DATETIME;

DATETIME dateTime = {0, 0, 0, 12, 0, 0};

int alarmState;
DATETIME openTime = {0, 0, 0, 8, 0, 0};
DATETIME closeTime = {0, 0, 0, 22, 0, 0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
void bluetooth_Event();
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
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart2, &rx2char,1);
  HAL_UART_Receive_IT(&huart6, &btchar,1);
  //DHT11_Init();
  LCD_init(&hi2c1);

  //DHT11_TypeDef dht11Data;
  char buff[30];

  printf("start main() \r\n");
  LCD_update(modeState, floorState, actuatorState, onoffState);

  if(HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
  {
	  Error_Handler();
  }

//  char buf[10] = "Hello\r";
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  ///////////////BUTTON EVENT////////////////////
	  if(keyNo != 0)
	  {
		  button_event();
		  LCD_update(modeState, floorState, actuatorState, onoffState);
		  keyNo = 0;
	  }
	  ////////////////////////////////////////////////////
	  if(rx2Flag)
	  {
		  printf("recv2 : %s\r\n",rx2Data);
		  rx2Flag =0;
	//	    HAL_UART_Transmit(&huart6, (uint8_t *)buf, strlen(buf), 0xFFFF);
	  }
	  if(tim3Flag1Sec)
	  {
		  tim3Flag1Sec = 0;
		  if(modeState == ALARM)
		  {
			  LCD_update(modeState, floorState, actuatorState, onoffState);
		  }
	  }
	  if(updateTimeFlag)
	  {
		  updateTimeFlag = 0;
		  char message[100];
		  sprintf(message, "[GETTIME]\n");
		  HAL_UART_Transmit(&huart6, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);

	  }
	  if(btFlag)
	  {
//		printf("bt : %s\r\n",btData);
			btFlag =0;
			bluetooth_Event();
	  }
	  //dht11Data = DHT11_readData();
	  //sprintf(buff,"h: %d%% t: %d.%d'C", dht11Data.rh_byte1, dht11Data.temp_byte1, dht11Data.temp_byte2);
	  //printf("%s\r\n", buff);
	  //LCD_writeStringXY(1, 0, buff);
	  HAL_Delay(20);


//	  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
//	  HAL_Delay(500);

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* EXTI0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  /* EXTI1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
  /* EXTI2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
  /* EXTI3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
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
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 84-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

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
  huart2.Init.Mode = UART_MODE_TX_RX;
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
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 9600;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : MODE_BTN_Pin FLOOR_BTN_Pin ACTUATOR_BTN_Pin SWITCH_BTN_Pin */
  GPIO_InitStruct.Pin = MODE_BTN_Pin|FLOOR_BTN_Pin|ACTUATOR_BTN_Pin|SWITCH_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void button_event()
{
    char message[CMD_SIZE] = {0};
    switch(keyNo) {
        case 1: //MODE TOGGLE
        	modeState = (modeState + 1) % 3;
            if(modeState == MONITOR)
            {
                //modeState = CONTROL;
                sprintf(message, "[BMS_SQL]GET@%dF@%s\n",
                        floorState,
                        actuatorState == 0 ? "FAN" :
                        actuatorState == 1 ? "BLIND" :
                        actuatorState == 2 ? "LAMP" :
                        "WINDOW");
                HAL_UART_Transmit(&huart6, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
            }
            else if(modeState == CONTROL)
            {
                //modeState = ALARM;
                if(floorState == 0) //All -> 1F
                {
                    floorState = 1;
                }
            }
            updateTimeFlag = 1;
            LCD_clear();
            break;
        case 2: //FLOOR CHANGE    ||	ALARM SWITCH
            if(modeState == MONITOR)
            {
                floorState = (floorState % FLOOR_SIZE) + 1;
                LCD_clear();
            }
            else if(modeState == CONTROL) //ADD ALL FLOOR
            {
                floorState = (floorState + 1) % (FLOOR_SIZE + 1);
                sprintf(message, "[BMS_SQL]GET@%dF@%s\n",
                        floorState,
                        actuatorState == 0 ? "FAN" :
                        actuatorState == 1 ? "BLIND" :
                        actuatorState == 2 ? "LAMP" :
                        "WINDOW");
                HAL_UART_Transmit(&huart6, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
            }
            else if(modeState == ALARM)
            {
            	alarmState = (alarmState + 1) % 2;
            }
            break;
        case 3: //ACTUATOR CHANGE	||	ALARM TIME CONFIG
            if(modeState == CONTROL)
            {
                actuatorState = (actuatorState + 1) % ACTUATOR_SIZE;
                sprintf(message, "[BMS_SQL]GET@%dF@%s\n",
                        floorState,
                        actuatorState == 0 ? "FAN" :
                        actuatorState == 1 ? "BLIND" :
                        actuatorState == 2 ? "LAMP" :
                        "WINDOW");
                HAL_UART_Transmit(&huart6, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
            }
            else if(modeState == ALARM)
            {
            	if(alarmState == 0)
				{
            		alarm_hour(&openTime);
				}
				else if(alarmState == 1)
				{
					alarm_hour(&closeTime);
				}
            }
            break;
        case 4: //ONOFF TOGGLE	||	ALARM ONOFF TOGGLE
            if(modeState == CONTROL)
            {
                onoffState = !onoffState;
                sprintf(message, "[BMS_SQL]SET@%dF@%s@%s\n",
                        floorState,
                        actuatorState == 0 ? "FAN" :
                        actuatorState == 1 ? "BLIND" :
                        actuatorState == 2 ? "LAMP" :
                        "WINDOW",
                        actuatorState == 1 ? (onoffState == 1 ? "UP" : "DOWN") : // BLIND
                        actuatorState == 3 ? (onoffState == 1 ? "OPEN" : "CLOSE") : // WINDOW
                        (onoffState == 1 ? "ON" : "OFF")); // FAN/LAMP
                HAL_UART_Transmit(&huart6, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
            }
            else if(modeState == ALARM)
			{
				if(alarmState == 0)	alarm_min(&openTime);
				else if(alarmState == 1) alarm_min(&closeTime);
			}
            break;
        default:
            break;
    }
}


void LCD_update(int mode, int floor, int actuator, int onoff) {
    /////////////////////INTIALIZE///////////////////////////
	char modeName[10];
	char floorName[4];
	char actuatorName[7];
	char onoffName[6];
	char buffer[16];

	/////////////////////NAME SETTING///////////////////////////
    if (floor == 0) {
        strcpy(floorName, "ALL");
    } else {
        sprintf(floorName, "%dF ", floor);
    }

    if (actuator == 0) {
        strcpy(actuatorName, "FAN   ");
        if (onoff == 0)
                strcpy(onoffName, "OFF  ");
            else if (onoff == 1)
                strcpy(onoffName, "ON   ");
    } else if (actuator == 1) {
        strcpy(actuatorName, "BLIND ");
        if (onoff == 0)
                strcpy(onoffName, "DOWN ");
            else if (onoff == 1)
                strcpy(onoffName, "UP   ");
    } else if (actuator == 2) {
        strcpy(actuatorName, "LAMP  ");
        if (onoff == 0)
                strcpy(onoffName, "OFF  ");
            else if (onoff == 1)
                strcpy(onoffName, "ON   ");
    } else if (actuator == 3) {
        strcpy(actuatorName, "WINDOW");
        if (onoff == 0)
                strcpy(onoffName, "CLOSE");
            else if (onoff == 1)
                strcpy(onoffName, "OPEN ");
    }

    /////////////////////////////LCD PRINT///////////////////////////////
    if (mode == MONITOR) {  // MONITOR MODE
		strcpy(modeName, "MONITOR");
		snprintf(buffer, sizeof(buffer), "%-8s %-5s", modeName, floorName);
		LCD_writeStringXY(0, 0, buffer);
	} else if (mode == CONTROL) {  // CONTROL MODE
		strcpy(modeName, "CONTROL");
		snprintf(buffer, sizeof(buffer), "%-8s %-5s", modeName, floorName);
		LCD_writeStringXY(0, 0, buffer);
		snprintf(buffer, sizeof(buffer), "%-8s %-5s", actuatorName, onoffName);
		LCD_writeStringXY(1, 0, buffer);
	} else if (mode == ALARM) {
		strcpy(modeName, "ALARM");
		snprintf(buffer, sizeof(buffer), "%s %02d:%02d:%02d", modeName, dateTime.hour, dateTime.min, dateTime.sec);
		LCD_writeStringXY(0, 0, buffer);
		if(alarmState == 0) snprintf(buffer, sizeof(buffer), "%02d:%02d  OPEN ", openTime.hour, openTime.min);
		else if(alarmState == 1) snprintf(buffer, sizeof(buffer), "%02d:%02d  CLOSE", closeTime.hour, closeTime.min);
		LCD_writeStringXY(1, 0, buffer);
	}
}

void bluetooth_Event()
{
    int i = 0;
    char *pToken;
    char *pArray[ARR_CNT] = {0};
    char recvBuf[CMD_SIZE] = {0};
    char sendBuf[CMD_SIZE] = {0};

    strcpy(recvBuf, btData);

    printf("btData : %s\r\n", btData);

    pToken = strtok(recvBuf, "[@]");
    while (pToken != NULL) {
        pArray[i] = pToken;
        if (++i >= ARR_CNT)
            break;
        pToken = strtok(NULL, "[@]");
    }

    ///////////////////////////////// ACTUATOR STATE RECEIVE /////////////////////////////////
    if (!strcmp(pArray[1], "ACTUATOR"))
    {
        int receivedFloor;
        if (!strcmp(pArray[2], "ALL")) {
            receivedFloor = 0;
        } else if (sscanf(pArray[2], "%dF", &receivedFloor) == 1) {
            floorState = receivedFloor;
        }

        if (!strcmp(pArray[3], "FAN")) {
            actuatorState = 0;
        } else if (!strcmp(pArray[3], "BLIND")) {
            actuatorState = 1;
        } else if (!strcmp(pArray[3], "LAMP")) {
            actuatorState = 2;
        } else if (!strcmp(pArray[3], "WINDOW")) {
            actuatorState = 3;
        }

        onoffState = (!strcmp(pArray[4], "ON") || !strcmp(pArray[4], "OPEN") || !strcmp(pArray[4], "UP")) ? 1 : 0;

        LCD_update(modeState, floorState, actuatorState, onoffState);
        return;
    }
    ///////////////////////////////// SENSOR DATA RECEIVE /////////////////////////////////
    else if((modeState == MONITOR) && !strcmp(pArray[1], "SENSOR"))
    {	//SENSOR@FLOOR@CDS@HUMI@TEMP
    	int floor = atoi(pArray[2]);
    	if(floor == floorState)
    	{
    		int cds = atoi(pArray[3]);
			int humi = atoi(pArray[4]);
			int temp = atoi(pArray[5]);
			char buffer[16];

			snprintf(buffer, sizeof(buffer), "C:%2d,H:%2d,T:%2d",cds, humi, temp);
			LCD_writeStringXY(1, 0, buffer);
    	}
    	return;
    }
    ////////////////////////////////// GETTIME /////////////////////////////////////////
    else if(!strcmp(pArray[0],"GETTIME")) {  //GETTIME
      dateTime.year = (pArray[1][0]-0x30) * 10 + pArray[1][1]-0x30 ;
      dateTime.month =  (pArray[1][3]-0x30) * 10 + pArray[1][4]-0x30 ;
      dateTime.day =  (pArray[1][6]-0x30) * 10 + pArray[1][7]-0x30 ;
      dateTime.hour = (pArray[1][9]-0x30) * 10 + pArray[1][10]-0x30 ;
      dateTime.min =  (pArray[1][12]-0x30) * 10 + pArray[1][13]-0x30 ;
      dateTime.sec =  (pArray[1][15]-0x30) * 10 + pArray[1][16]-0x30 ;
      return;
    }
    ////////////////////////////////// ETC /////////////////////////////////////////
    else if (!strncmp(pArray[1], " New conn", sizeof(" New conn"))) {
        return;
    } else if (!strncmp(pArray[1], " Already log", sizeof(" Already log"))) {
        return;
    } else
        return;

    sprintf(sendBuf, "[%s]%s@%s\n", pArray[0], pArray[1], pArray[2]);
    HAL_UART_Transmit(&huart6, (uint8_t *)sendBuf, strlen(sendBuf), HAL_MAX_DELAY);
}


/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART6 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}

void clock_calc(DATETIME *dateTime, DATETIME *openTime, DATETIME *closeTime)
{
  dateTime->sec++;          // 초 증가

  if (dateTime->sec >= 60) // 60초 도달 시
  {
      dateTime->sec = 0;    // 초 초기화
      dateTime->min++;      // 분 증가

      if (dateTime->min >= 60) // 60분 도달 시
      {
          dateTime->min = 0;  // 분 초기화
          dateTime->hour++;   // 시간 증가

          if (dateTime->hour == 24) // 24시간 도달 시
          {
            dateTime->hour = 0;   // 시간 초기화
            updateTimeFlag = 1; // 시간이 갱신되었음을 알리는 플래그
          }
       }
   }

  char message[100];
  if((dateTime->min == openTime->min)&&(dateTime->hour == openTime->hour)&&(dateTime->sec == 0))
  {
		sprintf(message, "[BMS_SQL]OPEN\n");
		HAL_UART_Transmit(&huart6, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);

  }
  else if((dateTime->min == closeTime->min)&&(dateTime->hour == closeTime->hour)&&(dateTime->sec == 0))
  {
		sprintf(message, "[BMS_SQL]CLOSE\n");
		HAL_UART_Transmit(&huart6, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
  }
}

void alarm_min(DATETIME *alarmTime)
{
  alarmTime->min++;          // 초 증가

  if (alarmTime->min >= 60) // 60분 도달 시
  {
	  alarmTime->min = 0;  // 분 초기화
	  alarmTime->hour++;   // 시간 증가

	  if (alarmTime->hour == 24) // 24시간 도달 시
	  {
		  alarmTime->hour = 0;   // 시간 초기화
	  }
   }
}

void alarm_hour(DATETIME *alarmTime)
{
  alarmTime->hour++;          // 초 증가

  if (alarmTime->hour == 24) // 24시간 도달 시
  {
	  alarmTime->hour = 0;   // 시간 초기화
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	uint32_t currentTime = HAL_GetTick();

	if ((currentTime - lastButtonPressTime < 50) && keyNo != 0) {
		return;
	}
	lastButtonPressTime = currentTime;
	switch (GPIO_Pin) {
		case MODE_BTN_Pin:
			keyNo = 1;
			break;
		case FLOOR_BTN_Pin:
			keyNo = 2;
			break;
		case ACTUATOR_BTN_Pin:
			keyNo = 3;
			break;
		case SWITCH_BTN_Pin:
			keyNo = 4;
			break;
		default:
			keyNo = 0;
			return;
	}
}

///////////////////////////TIMER INTERRUPT/////////////////////////////////////
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

	tim3Cnt++;
	if(tim3Cnt >= 1000) //1ms * 1000 = 1Sec
	{
		tim3Flag1Sec = 1;
		tim3Cnt = 0;
		clock_calc(&dateTime, &openTime, &closeTime);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {
    	static int i=0;
    	rx2Data[i] = rx2char;
    	if((rx2Data[i] == '\r')||(btData[i] == '\n'))
    	{
    		rx2Data[i] = '\0';
    		rx2Flag = 1;
    		i = 0;
    	}
    	else
    	{
    		i++;
    	}
    	HAL_UART_Receive_IT(&huart2, &rx2char,1);
    }
    if(huart->Instance == USART6)
    {
    	static int i=0;
    	btData[i] = btchar;
    	if((btData[i] == '\n') || btData[i] == '\r')
    	{
    		btData[i] = '\0';
    		btFlag = 1;
    		i = 0;
    	}
    	else
    	{
    		i++;
    	}
    	HAL_UART_Receive_IT(&huart6, &btchar,1);
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
