/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define LoRa_SPI4_SCK_Pin GPIO_PIN_2
#define LoRa_SPI4_SCK_GPIO_Port GPIOE
#define LoRa_RST_Pin GPIO_PIN_3
#define LoRa_RST_GPIO_Port GPIOE
#define LoRa_SPI4_NSS_Pin GPIO_PIN_4
#define LoRa_SPI4_NSS_GPIO_Port GPIOE
#define LoRa_SPI4_MISO_Pin GPIO_PIN_5
#define LoRa_SPI4_MISO_GPIO_Port GPIOE
#define LoRa_SPI4_MOSI_Pin GPIO_PIN_6
#define LoRa_SPI4_MOSI_GPIO_Port GPIOE
#define HC_SET_Pin GPIO_PIN_3
#define HC_SET_GPIO_Port GPIOC
#define STM_USART2_CTS_Pin GPIO_PIN_0
#define STM_USART2_CTS_GPIO_Port GPIOA
#define STM_USART2_RTS_Pin GPIO_PIN_1
#define STM_USART2_RTS_GPIO_Port GPIOA
#define STM_USART2_TX_Pin GPIO_PIN_2
#define STM_USART2_TX_GPIO_Port GPIOA
#define STM_USART2_RX_Pin GPIO_PIN_3
#define STM_USART2_RX_GPIO_Port GPIOA
#define M_STAT_Pin GPIO_PIN_4
#define M_STAT_GPIO_Port GPIOA
#define M_RI_Pin GPIO_PIN_5
#define M_RI_GPIO_Port GPIOA
#define M_PWR_Pin GPIO_PIN_6
#define M_PWR_GPIO_Port GPIOA
#define ESP_UART7_RX_Pin GPIO_PIN_7
#define ESP_UART7_RX_GPIO_Port GPIOE
#define ESP_UART7_TX_Pin GPIO_PIN_8
#define ESP_UART7_TX_GPIO_Port GPIOE
#define M_I2C2_SCL_Pin GPIO_PIN_10
#define M_I2C2_SCL_GPIO_Port GPIOB
#define M_I2C2_SDA_Pin GPIO_PIN_11
#define M_I2C2_SDA_GPIO_Port GPIOB
#define M_SPI2_NSS_Pin GPIO_PIN_12
#define M_SPI2_NSS_GPIO_Port GPIOB
#define M_SPI2_SCK_Pin GPIO_PIN_13
#define M_SPI2_SCK_GPIO_Port GPIOB
#define M_SPI2_MISO_Pin GPIO_PIN_14
#define M_SPI2_MISO_GPIO_Port GPIOB
#define M_SPI2_MOSI_Pin GPIO_PIN_15
#define M_SPI2_MOSI_GPIO_Port GPIOB
#define M_USART3_TX_Pin GPIO_PIN_8
#define M_USART3_TX_GPIO_Port GPIOD
#define M_USART3_RX_Pin GPIO_PIN_9
#define M_USART3_RX_GPIO_Port GPIOD
#define M_USART3_CTS_Pin GPIO_PIN_11
#define M_USART3_CTS_GPIO_Port GPIOD
#define M_USART3_RTS_Pin GPIO_PIN_12
#define M_USART3_RTS_GPIO_Port GPIOD
#define LED4_Pin GPIO_PIN_13
#define LED4_GPIO_Port GPIOD
#define LED3_Pin GPIO_PIN_14
#define LED3_GPIO_Port GPIOD
#define LED2_Pin GPIO_PIN_15
#define LED2_GPIO_Port GPIOD
#define LED1_Pin GPIO_PIN_6
#define LED1_GPIO_Port GPIOC
#define GPS_UART1_TX_Pin GPIO_PIN_9
#define GPS_UART1_TX_GPIO_Port GPIOA
#define GPS_UART1_RX_Pin GPIO_PIN_10
#define GPS_UART1_RX_GPIO_Port GPIOA
#define SD_SPI3_NSS_Pin GPIO_PIN_15
#define SD_SPI3_NSS_GPIO_Port GPIOA
#define SD_SPI3_SCK_Pin GPIO_PIN_10
#define SD_SPI3_SCK_GPIO_Port GPIOC
#define SD_SPI_MISO_Pin GPIO_PIN_11
#define SD_SPI_MISO_GPIO_Port GPIOC
#define SD_SPI3_MOSI_Pin GPIO_PIN_12
#define SD_SPI3_MOSI_GPIO_Port GPIOC
#define CAN_STB_Pin GPIO_PIN_2
#define CAN_STB_GPIO_Port GPIOD
#define GPS_I2C1_SCL_Pin GPIO_PIN_6
#define GPS_I2C1_SCL_GPIO_Port GPIOB
#define GPS_I2C2_SCL_Pin GPIO_PIN_7
#define GPS_I2C2_SCL_GPIO_Port GPIOB
#define HC_UART8_RX_Pin GPIO_PIN_0
#define HC_UART8_RX_GPIO_Port GPIOE
#define HC_UART8_TX_Pin GPIO_PIN_1
#define HC_UART8_TX_GPIO_Port GPIOE

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */
#define LORA_DEBUG	// na vypis debug informacii pouzi funkciu vypis();
/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
