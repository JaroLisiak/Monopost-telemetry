/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 ** This notice applies to any and all portions of this file
 * that are not between comment pairs USER CODE BEGIN and
 * USER CODE END. Other portions of this file, whether
 * inserted by the user or by software development tools
 * are owned by their respective copyright owners.
 *
 * COPYRIGHT(c) 2019 STMicroelectronics
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"

/* USER CODE BEGIN Includes */
#include "gps.h"
#include "CAN2018.h"
#include <string.h>

#define GPSSENTENCE 100
#define ESP32SENTENCE 2
#define HC12SENTENCE 2


/*
 // LORA return codes
 #define LORA_OK                            0
 #define LORA_CRC_ERROR                     1
 #define LORA_TIMEOUT                       2
 #define LORA_INVALID_HEADER                3
 #define LORA_ERROR                         4
 #define LORA_BUSY                          5
 #define LORA_EMPTY                         6
 */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;
SPI_HandleTypeDef hspi4;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart7;
UART_HandleTypeDef huart8;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

uint16_t unAvailableTime = 3000;

// Timers flags
uint8_t Tim1Flag = 0;
uint8_t Tim2Flag = 0;
uint8_t Tim3Flag = 0;
uint8_t Tim4Flag = 0;

// Availible states
uint8_t hcAvailible = 1;


// HC-12
uint8_t hcRxChar[2];	// character from HC-12
uint8_t hcSentence[HC12SENTENCE];
uint16_t hcSentenceLength = 0;
uint8_t hcSentenceReady = 0;
uint32_t hcLastOK = 0;

// GPS
uint8_t RxFromGpsBuffer[2];
uint8_t Gps_Sentence[GPSSENTENCE];
int Gps_Sentence_length = 0;

// ESP32 - WiFi
uint8_t esp32RxChar[2];
uint8_t Esp32_Sentence[ESP32SENTENCE];
int Esp32_Sentence_length = 0;
uint8_t esp32Availible = 0;
uint32_t esp32LastOK = 0;


// ID of package
uint16_t msgID = 1;	// packet ID
char* sendWiFiMsg(uint8_t *msg, uint16_t *msg_len);
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_SPI2_Init(void);
static void MX_SPI3_Init(void);
static void MX_SPI4_Init(void);
static void MX_UART7_Init(void);
static void MX_UART8_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/


uint16_t getCharCount(uint8_t *msg, uint8_t *msg_len, uint8_t ch) {
	int count = 0;
	for (int i = 0; i < *msg_len; i++) {
		if (*(msg + i) == ch) {
			count++;
		}
	}
	return count;
}

uint8_t sendHc12(uint8_t *msg, uint8_t *msg_len) {
	// send via HC-12 (434 MHz)

	hcAvailible = 0;
	uint16_t specCharCount = getCharCount(msg, msg_len, 0xff)
			+ getCharCount(msg, msg_len, 0xfe);
	uint8_t * new_msg = (uint8_t*) malloc(*msg_len + specCharCount + 6);// original size + doubled chars + header & footer
	new_msg[0] = 0x00;
	new_msg[1] = 0xfe;
	new_msg[2] = 0x00;

	uint16_t oldIndex = 0;
	uint16_t newIndex = 3;
	for (oldIndex = 0; oldIndex < *msg_len; oldIndex++) {
		if ((*(msg + oldIndex) == 0xff) || (*(msg + oldIndex) == 0xfe)) {
			new_msg[newIndex++] = *(msg + oldIndex);
			new_msg[newIndex++] = *(msg + oldIndex);
		} else {
			new_msg[newIndex++] = *(msg + oldIndex);
		}
	}
	new_msg[newIndex++] = 0x00;
	new_msg[newIndex++] = 0xff;
	new_msg[newIndex++] = 0x00;

	uint8_t res = HAL_UART_Transmit(&huart8, new_msg, newIndex, 300);
	HAL_Delay(100);	
	vypis("\r\n");
	free(new_msg);
	hcAvailible = 1;
	return res;
}
uint8_t sendEsp32(uint8_t *msg, uint8_t *msg_len) {
	HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
	// send via HC-12 (434 MHz)

	uint16_t specCharCount = getCharCount(msg, msg_len, 0xff)
			+ getCharCount(msg, msg_len, 0xfe);
	uint8_t * new_msg = (uint8_t*) malloc(*msg_len + specCharCount + 6);// original size + doubled chars + header & footer
	new_msg[0] = 0x00;
	new_msg[1] = 0xfe;
	new_msg[2] = 0x00;

	uint16_t oldIndex = 0;
	uint16_t newIndex = 3;
	for (oldIndex = 0; oldIndex < *msg_len; oldIndex++) {
		if ((*(msg + oldIndex) == 0xff) || (*(msg + oldIndex) == 0xfe)) {
			new_msg[newIndex++] = *(msg + oldIndex);
			new_msg[newIndex++] = *(msg + oldIndex);
		} else {
			new_msg[newIndex++] = *(msg + oldIndex);
		}
	}
	new_msg[newIndex++] = 0x00;
	new_msg[newIndex++] = 0xff;
	new_msg[newIndex++] = 0x00;

	sendWiFiMsg(new_msg, newIndex);
	HAL_Delay(100);	
	free(new_msg);
}

void send(uint8_t *msg, uint8_t *msg_len) {
	uint8_t res = sendWiFiMsg(msg, msg_len);
	uint8_t res2 = sendHc12(msg, msg_len);
}

uint8_t * append_message_ID(uint8_t *msg, uint16_t *msg_len, uint16_t ID) {
	uint8_t dataSize = 2; // ID is uint16 (2 bytes)
	msg = (uint8_t *) realloc(msg, *msg_len + dataSize * sizeof(uint8_t));

	int index = *msg_len;
	msg[0] = ID & 0xff;
	msg[1] = ID >> 8;
	*msg_len = *msg_len + dataSize;
	return msg;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart8) {
		// HC-12

		if ((hcRxChar[0] == 'O')) {
			hcSentence[0] = 'O';
		} else if ((hcRxChar[0] == 'K') && (hcSentence[0] == 'O')) {
			hcLastOK = HAL_GetTick();
			hcSentence[0] = 0x00;
			hcSentence[1] = 0x00;
		} else {
			hcSentence[0] = 0x00;
			hcSentence[1] = 0x00;
		}
		HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
		HAL_UART_Receive_IT(&huart8, hcRxChar, 1);
	}
	if (huart == &huart7) {
		// wifi module

		if ((esp32RxChar[0] == 'O')) {
			Esp32_Sentence[0] = 'O';
		} else if ((esp32RxChar[0] == 'K') && (Esp32_Sentence[0] == 'O')) {
			
			esp32LastOK = HAL_GetTick();
			
			Esp32_Sentence[0] = 0x00;
			Esp32_Sentence[1] = 0x00;
		} else {
			Esp32_Sentence[0] = 0x00;
			Esp32_Sentence[1] = 0x00;
		}

		HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
		
		HAL_UART_Receive_IT(&huart7, esp32RxChar, 1);

	}
	if (huart == &huart1) {
		// GPS
		if (RxFromGpsBuffer[0] == '$') {
			Gps_Sentence_length = 0;
			Gps_Sentence[Gps_Sentence_length++] = RxFromGpsBuffer[0];
		} else {
			if (RxFromGpsBuffer[0] == '\n') {
				Gps_Sentence[Gps_Sentence_length++] = RxFromGpsBuffer[0];
				GPS_FindSentence(&GPS_msg, Gps_Sentence, 0, 100);
				if (GPS_msg.type != NMEA_BAD) {
					GPS_sentences_parsed++;
					GPS_ParseSentence(Gps_Sentence, &GPS_msg);
				} else
					GPS_sentences_unknown++;
			} else {
				Gps_Sentence[Gps_Sentence_length++] = RxFromGpsBuffer[0];
			}
		}
		HAL_UART_Receive_IT(&huart1, RxFromGpsBuffer, 1);
	}
}

void vypis(char str[]) {
	// uncomment to debug
	// comment to real conditions

	//HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str), 20);

	//HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
}

char * addToMsg(char dst[], const char src[]) {
	int sizeNewArray = strlen(dst) + strlen(src) + 1;
	char * newJSON = (char *) malloc(sizeNewArray);
	strcpy(newJSON, dst);
	strcpy(newJSON + strlen(dst), src);
	free(dst);	// free old memory
	return newJSON;
}

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 *
 * @retval None
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration----------------------------------------------------------*/

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
	MX_CAN1_Init();
	MX_I2C1_Init();
	MX_I2C2_Init();
	MX_SPI2_Init();
	MX_SPI3_Init();
	MX_SPI4_Init();
	MX_UART7_Init();
	MX_UART8_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_USART3_UART_Init();
	MX_TIM1_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM4_Init();
	/* USER CODE BEGIN 2 */

	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_Base_Start_IT(&htim4);

	GPS_InitData();

	if (HAL_UART_Receive_IT(&huart8, hcRxChar, 1) != HAL_OK) {
		// HC-12 error
		hcAvailible = 0;
		vypis("\n\nHC-12 ERROR!!  \r\n\n");
	}

	if (HAL_UART_Receive_IT(&huart1, RxFromGpsBuffer, 1) != HAL_OK) {
		// GPS error
	}

	if (HAL_UART_Receive_IT(&huart7, esp32RxChar, 1) != HAL_OK) {
		// ESP32 error
		vypis("\n\nESP32 ERROR!!  \r\n\n");
	}
	if (HAL_CAN_Start(&hcan1) != HAL_OK) {
		// can error
	}

	/*
	 * 				  RX - TX
	 * USART 1 - GPS - PA10, PA9
	 * GPS: 9600 Baud, 8 bits, no parity bit, 1 stop bit,
	 *
	 * USART 2 - SERIAL USB - PA3, PA2
	 * USART 3 - MICROBUS CLICK2 - PD9, PD8 + PD11 + PD12
	 * UART 7 - ESP32 - PE7, PE8
	 * UART 8 - HC-12 - PE0, PE1, PC3 (set)
	 * SET PIN = low (reset) -> setting mode, AT
	 * SET PIN = high (set) -> communication mode
	 *
	 * SPI2 - LORA - PB12, PB13, PB14, PB15
	 * USART3
	 *
	 * I2C2 - GPS
	 *
	 */

	/*
	 * HC-12 config
	 */
	HAL_GPIO_WritePin(HC_SET_GPIO_Port, HC_SET_Pin, GPIO_PIN_RESET); // config mode
	HAL_Delay(100);


	// +20 dBm boost transmit
	HAL_UART_Transmit(&huart8, "AT+P8\0", 6, 100);
	HAL_Delay(100);

	// set HC-12 to normal (sending) mode
	HAL_GPIO_WritePin(HC_SET_GPIO_Port, HC_SET_Pin, GPIO_PIN_SET); // send mode
	HAL_Delay(100);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	while (1) {

		uint8_t* msg = malloc(sizeof(uint8_t));
		uint16_t msg_len = 0;
		msg = append_message_ID(msg, &msg_len, msgID++); // append packet ID to message


		if (Tim1Flag) {
			// send CAN ID 10, 15, 20

			msg = append_BBOX_power_Data(msg, &msg_len);
			msg = append_wheel_RPM_Data(msg, &msg_len);
			msg = append_BBOX_status_Data(msg, &msg_len);

			Tim1Flag = 0;
		}
		if (Tim2Flag) {
			//send CAN iD 25,30,40,50

			msg = append_FU_Values_1(msg, &msg_len);
			msg = append_BBOX_command(msg, &msg_len);
			msg = append_BMS_Command(msg, &msg_len);
			msg = append_BMS_State(msg, &msg_len);

			Tim2Flag = 0;
		}
		if (Tim3Flag) {
			// send CAN ID 60, 70, 80, 90, 95 + GPS data

			msg = append_ECU_State(msg, &msg_len);
			msg = append_FU_Values_2(msg, &msg_len);

			msg = append_GPS_Data(msg, &msg_len);

			Tim3Flag = 0;
		}
		if (Tim4Flag) {
			// For CAN ID 184 - 290
			msg = append_Interconnect(msg, &msg_len);
			msg = append_BMS_Voltages(msg, &msg_len);
			msg = append_BMS_Temps(msg, &msg_len);

		}

		send(msg, &msg_len); // sendData(msg, &msg_len);
		free(msg);

		HAL_Delay(500);

		HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

	}
	/* USER CODE END 3 */

}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

	/**Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE()
	;

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/**Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = 16;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 180;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Activate the Over-Drive mode
	 */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Configure the Systick interrupt time
	 */
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

	/**Configure the Systick
	 */
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* CAN1 init function */
static void MX_CAN1_Init(void) {

	hcan1.Instance = CAN1;
	hcan1.Init.Prescaler = 16;
	hcan1.Init.Mode = CAN_MODE_NORMAL;
	hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hcan1.Init.TimeSeg1 = CAN_BS1_1TQ;
	hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
	hcan1.Init.TimeTriggeredMode = DISABLE;
	hcan1.Init.AutoBusOff = DISABLE;
	hcan1.Init.AutoWakeUp = DISABLE;
	hcan1.Init.AutoRetransmission = DISABLE;
	hcan1.Init.ReceiveFifoLocked = DISABLE;
	hcan1.Init.TransmitFifoPriority = DISABLE;
	if (HAL_CAN_Init(&hcan1) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/* I2C1 init function */
static void MX_I2C1_Init(void) {

	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/* I2C2 init function */
static void MX_I2C2_Init(void) {

	hi2c2.Instance = I2C2;
	hi2c2.Init.ClockSpeed = 100000;
	hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c2.Init.OwnAddress1 = 0;
	hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c2.Init.OwnAddress2 = 0;
	hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c2) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/* SPI2 init function */
static void MX_SPI2_Init(void) {

	/* SPI2 parameter configuration*/
	hspi2.Instance = SPI2;
	hspi2.Init.Mode = SPI_MODE_MASTER;
	hspi2.Init.Direction = SPI_DIRECTION_2LINES;
	hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi2.Init.NSS = SPI_NSS_SOFT;
	hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi2.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi2) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/* SPI3 init function */
static void MX_SPI3_Init(void) {

	/* SPI3 parameter configuration*/
	hspi3.Instance = SPI3;
	hspi3.Init.Mode = SPI_MODE_MASTER;
	hspi3.Init.Direction = SPI_DIRECTION_2LINES;
	hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi3.Init.NSS = SPI_NSS_HARD_OUTPUT;
	hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi3.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi3) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/* SPI4 init function */
static void MX_SPI4_Init(void) {

	/* SPI4 parameter configuration*/
	hspi4.Instance = SPI4;
	hspi4.Init.Mode = SPI_MODE_MASTER;
	hspi4.Init.Direction = SPI_DIRECTION_2LINES;
	hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi4.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi4.Init.NSS = SPI_NSS_SOFT;
	hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi4.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi4.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi4) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/* TIM1 init function */
static void MX_TIM1_Init(void) {

	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;

	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 2879;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 62499;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
			!= HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/* TIM2 init function */
static void MX_TIM2_Init(void) {

	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 8639;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 62499;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig)
			!= HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/* TIM3 init function */
static void MX_TIM3_Init(void) {

	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 14399;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 62499;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig)
			!= HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/* TIM4 init function */
static void MX_TIM4_Init(void) {

	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;

	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 28124;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 63999;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htim4) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig)
			!= HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/* UART7 init function */
static void MX_UART7_Init(void) {

	huart7.Instance = UART7;
	huart7.Init.BaudRate = 115200;
	huart7.Init.WordLength = UART_WORDLENGTH_8B;
	huart7.Init.StopBits = UART_STOPBITS_1;
	huart7.Init.Parity = UART_PARITY_NONE;
	huart7.Init.Mode = UART_MODE_TX_RX;
	huart7.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart7.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart7) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/* UART8 init function */
static void MX_UART8_Init(void) {

	huart8.Instance = UART8;
	huart8.Init.BaudRate = 9600;
	huart8.Init.WordLength = UART_WORDLENGTH_8B;
	huart8.Init.StopBits = UART_STOPBITS_1;
	huart8.Init.Parity = UART_PARITY_NONE;
	huart8.Init.Mode = UART_MODE_TX_RX;
	huart8.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart8.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart8) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/* USART1 init function */
static void MX_USART1_UART_Init(void) {

	huart1.Instance = USART1;
	huart1.Init.BaudRate = 9600;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/* USART2 init function */
static void MX_USART2_UART_Init(void) {

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/* USART3 init function */
static void MX_USART3_UART_Init(void) {

	huart3.Instance = USART3;
	huart3.Init.BaudRate = 115200;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart3) != HAL_OK) {
		_Error_Handler(__FILE__, __LINE__);
	}

}

/** Configure pins as 
 * Analog
 * Input
 * Output
 * EVENT_OUT
 * EXTI
 */
static void MX_GPIO_Init(void) {

	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE()
	;
	__HAL_RCC_GPIOC_CLK_ENABLE()
	;
	__HAL_RCC_GPIOA_CLK_ENABLE()
	;
	__HAL_RCC_GPIOB_CLK_ENABLE()
	;
	__HAL_RCC_GPIOD_CLK_ENABLE()
	;

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE, LoRa_RST_Pin | LoRa_SPI4_NSS_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, HC_SET_Pin | LED1_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(M_PWR_GPIO_Port, M_PWR_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, LED4_Pin | LED3_Pin | LED2_Pin | CAN_STB_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pins : LoRa_RST_Pin LoRa_SPI4_NSS_Pin */
	GPIO_InitStruct.Pin = LoRa_RST_Pin | LoRa_SPI4_NSS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : HC_SET_Pin LED1_Pin */
	GPIO_InitStruct.Pin = HC_SET_Pin | LED1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : M_STAT_Pin M_RI_Pin */
	GPIO_InitStruct.Pin = M_STAT_Pin | M_RI_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : M_PWR_Pin */
	GPIO_InitStruct.Pin = M_PWR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(M_PWR_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LED4_Pin LED3_Pin LED2_Pin CAN_STB_Pin */
	GPIO_InitStruct.Pin = LED4_Pin | LED3_Pin | LED2_Pin | CAN_STB_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/*
	 * Intervals:
	 * 1: 100ms - 287 - 62499
	 * 2: 500ms - 719 - 62499
	 * 3: 1s - 1439 - 62499
	 * 4: 2s - 5624 - 63999
	 */
	if (htim->Instance == htim1.Instance) {
		//  100 ms
		Tim1Flag = 1;
	}
	if (htim->Instance == htim2.Instance) {
		//  500 ms
		Tim2Flag = 1;
	}
	if (htim->Instance == htim3.Instance) {
		//  1000 ms
		Tim3Flag = 1;
	}
	if (htim->Instance == htim4.Instance) {
		//  2000 ms
		Tim4Flag = 1;
	}
}


uint8_t * removeDoubles(uint8_t *msg, uint16_t *msg_len) {
	uint16_t specCharCount = getCharCount(msg, msg_len, 0xfe)
			+ getCharCount(msg, msg_len, 0xff);
	uint16_t new_msg_size = (*msg_len) - 6 - ((specCharCount - 2) / 2);	// old size - 2 border chars - doubled
	uint8_t * new_msg = (uint8_t *) malloc(new_msg_size);
	uint16_t newIndex = 0;
	uint16_t oldIndex = 0;
	if ((*(msg) == 0x00) && (*(msg + 1) == 0xfe) && (*(msg + 2) == 0x00)) {
		oldIndex = 3;
		while (oldIndex < (*msg_len) - 3) {
			if ((*(msg + oldIndex) == 0xff)
					&& (*(msg + oldIndex + 1) == 0xff)) {
				new_msg[newIndex++] = *(msg + oldIndex);
				oldIndex += 2;
			} else if ((*(msg + oldIndex) == 0xfe)
					&& (*(msg + oldIndex + 1) == 0xfe)) {
				new_msg[newIndex++] = *(msg + oldIndex);
				oldIndex += 2;
			} else {
				new_msg[newIndex++] = *(msg + oldIndex);
				oldIndex++;
			}
		}
	}

	*msg_len = new_msg_size;
	// TODO: free?
	free(msg);
	return new_msg;
}

char* sendWiFiMsg(uint8_t *msg, uint16_t *msg_len) {

	uint16_t index = 2;	// first 2 bytes are packet ID
	uint16_t ramecID = ((uint16_t) *(msg + 1) << 8) | *(msg);	// packet ID

	char buff[260];	// buffer for internal JSON - { } - appended tu main JSON - [ { \"%d\":[ -
	char* JSON = malloc(sizeof(char));		// main (final JSON)
	JSON[0] = 0;
	uint16_t JSON_len = 0;

	// start main JSON with [ { \"%d\":[
	char buffer[13];
	sprintf(buffer, "{\"Id\":%d,\"Data\":{", ramecID); // new [{\"Id\":%d,\"Data\":[ 		old: [{\"%d\":[
	JSON = addToMsg(JSON, buffer);
	uint8_t parse_error = 0;
	while (index < *msg_len && parse_error == 0) {
		uint16_t key = ((uint16_t) *(msg + index + 1) << 8) | *(msg + index);
		index++;
		switch (key) {
		case 5:
			// GPS data
			if (JSON_len > 0) {
				JSON = addToMsg(JSON, ",\0");
			}
			sprintf(buff,
					"\"GPS_data\":{\"latitude\":%u,\"latitude_char\":\"%c\",\"longitude\":%u,\"longitude_char\":\"%c\",\"speed\":%u,\"course\":%u,\"altitude\":%ld}",
					(uint32_t) (*(msg + index + 1) << 24)
							| (uint32_t) (*(msg + index + 2) << 16)
							| (uint32_t) (*(msg + index + 3) << 8)
							| (uint32_t) (*(msg + index + 4)),
					*(msg + index + 5),
					(uint32_t) (*(msg + index + 6) << 24)
							| (uint32_t) (*(msg + index + 7) << 16)
							| (uint32_t) (*(msg + index + 8) << 8)
							| (uint32_t) (*(msg + index + 9)),
					*(msg + index + 10),
					(uint32_t) (*(msg + index + 11) << 24)
							| (uint32_t) (*(msg + index + 12) << 16)
							| (uint32_t) (*(msg + index + 13) << 8)
							| (uint32_t) (*(msg + index + 15)),
					(uint32_t) (*(msg + index + 15) << 24)
							| (uint32_t) (*(msg + index + 16) << 16)
							| (uint32_t) (*(msg + index + 17) << 8)
							| (uint32_t) (*(msg + index + 18)),
					(int32_t) (*(msg + index + 19) << 24)
							| (int32_t) (*(msg + index + 20) << 16)
							| (int32_t) (*(msg + index + 21) << 8)
							| (int32_t) (*(msg + index + 22)));
			JSON = addToMsg(JSON, buff);
			JSON_len++;
			index = index + 23;
			break;
		case 10:
			// BBOX_power_data
			if (JSON_len > 0) {
				JSON = addToMsg(JSON, ",\0");
			}
			sprintf(buff,
					"\"BBOX_power\":{\"power\":%d,\"current\":%d,\"voltage\":%d}",
					((int16_t) *(msg + index + 2) << 8) | *(msg + index + 1),
					((int16_t) *(msg + index + 4) << 8) | *(msg + index + 3),
					((uint16_t) *(msg + index + 6) << 8) | *(msg + index + 5));
			JSON = addToMsg(JSON, buff);
			JSON_len++;
			index = index + 7;
			break;
		case 15:
			// wheel RPM
			if (JSON_len > 0) {
				JSON = addToMsg(JSON, ",\0");
			}
			sprintf(buff,
					"\"wheel_RPM\":{\"front_right\":%d,\"front_left\":%d,\"rear_right\":%d,\"rear_left\":%d}",
					((uint16_t) *(msg + index + 2) << 8) | *(msg + index + 1),
					((uint16_t) *(msg + index + 4) << 8) | *(msg + index + 3),
					((uint16_t) *(msg + index + 6) << 8) | *(msg + index + 5),
					((uint16_t) *(msg + index + 8) << 8) | *(msg + index + 7));
			JSON = addToMsg(JSON, buff);
			JSON_len++;
			index = index + 9;
			break;
		case 20:
			// BBOX status data
			if (JSON_len > 0) {
				JSON = addToMsg(JSON, ",\0");
			}
			sprintf(buff,
					"\"BBOX_status\":{\"SHD_IN\":%d,\"SHD_OUT\":%d,\"TSMS\":%d,\"AIR_N\":%d,\"AIR_P\":%d,\"PRECH_60V\":%d,\"IMD_OK\":%d,\"BMS_OK\":%d,\"SIGNAL_ERROR\":%d,\"SHD_RESET\":%d,\"SHD_EN\":%d,\"POLARITY\":%d,\"FANS\":%d,\"STM_temp\":%d}",
					(uint8_t) (*(msg + index + 2) & 0b10000000) >> 7,
					(uint8_t) (*(msg + index + 2) & 0b01000000) >> 6,
					(uint8_t) (*(msg + index + 2) & 0b00001000) >> 3,
					(uint8_t) (*(msg + index + 1) & 0b10000000) >> 7,
					(uint8_t) (*(msg + index + 1) & 0b01000000) >> 6,
					(uint8_t) (*(msg + index + 1) & 0b00000010) >> 1,
					(uint8_t) (*(msg + index + 1) & 0b00001000) >> 3,
					(uint8_t) (*(msg + index + 1) & 0b00100000) >> 5,
					(uint8_t) (*(msg + index + 2) & 0b00010000) >> 4,
					(uint8_t) (*(msg + index + 2) & 0b00100000) >> 5,
					(uint8_t) (*(msg + index + 1) & 0b00000001) >> 0,
					(uint8_t) (*(msg + index + 1) & 0b00000100) >> 2,
					(uint8_t) (*(msg + index + 1) & 0b00010000) >> 4,
					(int8_t) *(msg + index + 3));
			JSON = addToMsg(JSON, buff);
			JSON_len++;

			index = index + 4;
			break;
		case 25:
			// FU_Values_1
			if (JSON_len > 0) {
				JSON = addToMsg(JSON, ",\0");
			}
			sprintf(buff,
					"\"FU_Values_1\":{\"apps1\":%d,\"apps2\":%d,\"brake1\":%d,\"brake2\":%d,\"error\":%d}",
					(uint8_t) *(msg + index + 1), (uint8_t) *(msg + index + 2),
					(uint8_t) *(msg + index + 3), (uint8_t) *(msg + index + 4),
					((uint16_t) *(msg + index + 6) << 8) | *(msg + index + 5)

					);
			JSON = addToMsg(JSON, buff);
			JSON_len++;
			index = index + 7;
			break;
		case 30:
			// BBOX_command
			if (JSON_len > 0) {
				JSON = addToMsg(JSON, ",\0");
			}
			sprintf(buff, "\"BBOX_command\":{\"FANS\":%d,\"SHD_EN\":%d}",
					(uint8_t) (*(msg + index + 1) & 0b10000000) >> 7,
					(uint8_t) (*(msg + index + 1) & 0b01000000) >> 6);
			JSON = addToMsg(JSON, buff);
			JSON_len++;
			index = index + 2;
			break;
		case 40:
			// BMS_Command
			if (JSON_len > 0) {
				JSON = addToMsg(JSON, ",\0");
			}
			sprintf(buff,
					"\"BMS_Command\":{\"BMS_Balanc\":%d,\"BMS_FullMode\":%d,\"BMS_OK\":%d,\"BMS_ONOFF\":%d,\"BMS_CAN\":%d}",
					(uint8_t) (*(msg + index + 1) & 0b11000000) >> 6,
					(uint8_t) (*(msg + index + 2) & 0b10000000) >> 7,
					(uint8_t) (*(msg + index + 1) & 0b00110000) >> 4,
					(uint8_t) (*(msg + index + 1) & 0b00001100) >> 2,
					(uint8_t) (*(msg + index + 1) & 0b00000011));
			JSON = addToMsg(JSON, buff);
			JSON_len++;
			index = index + 3;
			break;
		case 50:
			// BMS_State
			if (JSON_len > 0) {
				JSON = addToMsg(JSON, ",\0");
			}
			sprintf(buff,
					"\"BMS_State\":{\"BMS_Mode\":%d,\"BMS_Faults\":%d,\"CellVolt_L\":%d,\"CellVolt_H\":%d,\"CellTemp_L\":%d,\"CellTemp_H\":%d,\"BMS_Ident\":%d}",
					(uint8_t) *(msg + index + 1),
					((uint16_t) *(msg + index + 3) << 8) | *(msg + index + 2),
					(uint8_t) *(msg + index + 4), (uint8_t) *(msg + index + 5),
					(uint8_t) *(msg + index + 6), (uint8_t) *(msg + index + 7),
					(uint8_t) *(msg + index + 8));
			JSON = addToMsg(JSON, buff);
			JSON_len++;
			index = index + 9;
			break;
		case 60:
			// ECU_State
			if (JSON_len > 0) {
				JSON = addToMsg(JSON, ",\0");
			}
			sprintf(buff,
					"\"ECU_State\":{\"ECU_Status\":%d,\"FL_AMK_Status\":%d,\"FR_AMK_Status\":%d,\"RL_AMK_Status\":%d,\"RR_AMK_Status\":%d,\"TempMotor_H\":%d,\"TempInverter_H\":%d,\"TempIGBT_H\":%d}",
					(uint8_t) *(msg + index + 1), (uint8_t) *(msg + index + 2),
					(uint8_t) *(msg + index + 3), (uint8_t) *(msg + index + 4),
					(uint8_t) *(msg + index + 5), (uint8_t) *(msg + index + 6),
					(uint8_t) *(msg + index + 7), (uint8_t) *(msg + index + 8));
			JSON = addToMsg(JSON, buff);
			JSON_len++;
			index = index + 9;
			break;
		case 70:
			// FU_Values_2
			if (JSON_len > 0) {
				JSON = addToMsg(JSON, ",\0");
			}
			sprintf(buff,
					"\"FU_Values_2\":{\"steer\":%d,\"susp_FL\":%d,\"susp_FR\":%d,\"brake_pos\":%d,\"RTD\":%d,\"BOTS\":%d,\"SHDB\":%d,\"INERTIA_SW\":%d,\"reserve\":%d}",
					(int8_t) *(msg + index + 1),
					((uint16_t) *(msg + index + 3) << 8) | *(msg + index + 2),
					((uint16_t) *(msg + index + 5) << 8) | *(msg + index + 4),
					(uint8_t) *(msg + index + 6),
					(uint8_t) (*(msg + index + 7) & 0b10000000) >> 7,
					(uint8_t) (*(msg + index + 7) & 0b01000000) >> 6,
					(uint8_t) (*(msg + index + 7) & 0b00100000) >> 5,
					(uint8_t) (*(msg + index + 7) & 0b00010000) >> 4,
					(uint8_t) *(msg + index + 8));
			JSON = addToMsg(JSON, buff);
			JSON_len++;
			index = index + 9;
			break;
		case 80:
			// Interconnect
			if (JSON_len > 0) {
				JSON = addToMsg(JSON, ",\0");
			}
			sprintf(buff,
					"\"Interconnect\":{\"car_state\":%d,\"left_w_pump\":%d,\"right_w_pump\":%d,\"brake_red\":%d,\"brake_white\":%d,\"tsas\":%d,\"killswitch_R\":%d,\"killswitch_L\":%d,\"reserve\":%d,\"susp_RR\":%d,\"susp_RL\":%d}",
					(uint8_t) *(msg + index + 1),
					(uint8_t) (*(msg + index + 2) & 0b10000000) >> 7,
					(uint8_t) (*(msg + index + 2) & 0b01000000) >> 6,
					(uint8_t) (*(msg + index + 2) & 0b00100000) >> 5,
					(uint8_t) (*(msg + index + 2) & 0b00010000) >> 4,
					(uint8_t) (*(msg + index + 2) & 0b00001000) >> 3,
					(uint8_t) (*(msg + index + 2) & 0b00000100) >> 2,
					(uint8_t) (*(msg + index + 2) & 0b00000010) >> 1,
					(uint8_t) *(msg + index + 3),
					((uint16_t) *(msg + index + 5) << 8) | *(msg + index + 4),
					((uint16_t) *(msg + index + 7) << 8) | *(msg + index + 6));
			JSON = addToMsg(JSON, buff);
			JSON_len++;
			index = index + 8;
			break;
		case 90:
			// BMS_Voltages
			if (JSON_len > 0) {
				JSON = addToMsg(JSON, ",\0");
			}
			sprintf(buff,
					"\"BMS_Voltages\":{\"BMS_VoltIdent\":%d,\"BMS_Volt1\":%d,\"BMS_Volt2\":%d,\"BMS_Volt3\":%d,\"BMS_Volt4\":%d,\"BMS_Volt5\":%d,\"BMS_Volt6\":%d,\"BMS_Volt7\":%d}",
					(uint8_t) *(msg + index + 1), (uint8_t) *(msg + index + 2),
					(uint8_t) *(msg + index + 3), (uint8_t) *(msg + index + 4),
					(uint8_t) *(msg + index + 5), (uint8_t) *(msg + index + 6),
					(uint8_t) *(msg + index + 7), (uint8_t) *(msg + index + 8));
			JSON = addToMsg(JSON, buff);
			JSON_len++;
			index = index + 9;
			break;
		case 95:
			// BMS_Voltages
			if (JSON_len > 0) {
				JSON = addToMsg(JSON, ",\0");
			}
			sprintf(buff,
					"\"BMS_Temps\":{\"BMS_TempIdent\":%d,\"BMS_Temp1\":%d,\"BMS_Temp2\":%d,\"BMS_Temp3\":%d,\"BMS_Temp4\":%d,\"BMS_Temp5\":%d,\"BMS_Temp6\":%d,\"BMS_Temp7\":%d}",
					(uint8_t) *(msg + index + 1), (uint8_t) *(msg + index + 2),
					(uint8_t) *(msg + index + 3), (uint8_t) *(msg + index + 4),
					(uint8_t) *(msg + index + 5), (uint8_t) *(msg + index + 6),
					(uint8_t) *(msg + index + 7), (uint8_t) *(msg + index + 8));
			JSON = addToMsg(JSON, buff);
			JSON_len++;
			index = index + 9;
			break;
		default:
			parse_error = 1;
			break;
		}
	}
	JSON = addToMsg(JSON, "}}");	// append last chars to final JSON

	uint8_t res = HAL_UART_Transmit(&huart7, JSON, strlen(JSON), 1500);
	HAL_Delay(50);
	free(JSON);
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  file: The file name as string.
 * @param  line: The line in file as a number.
 * @retval None
 */
void _Error_Handler(char *file, int line) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while (1) {
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
void assert_failed(uint8_t* file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	 tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/