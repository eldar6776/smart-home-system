/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Receiver modul main fuction code
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
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
#include "dio.h"
#include "pwm.h"
#include "anin.h"
#include "rs485.h"
/* Exported macros  --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
CRC_HandleTypeDef hcrc;
ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart1;
uint8_t sysfl   = 0;
/* Private define ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void RAM_Init(void);
static void MX_CRC_Init(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_UART_Init(void);
static void SystemClock_Config(void);
/* Private function prototypes -----------------------------------------------*/
int main(void){
	HAL_Init();
	SystemClock_Config();
    MX_CRC_Init();
	MX_GPIO_Init();
    MX_ADC1_Init();	
    MX_SPI1_Init();
    MX_I2C1_Init();	
    RAM_Init();
	MX_UART_Init();
    PresentSystem();
    RS485_Init();
	
	while (1){
		DIO_Service();
        ANIN_Service();
		RS485_Service();
		PWM_Service();		
	}
}
/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line){
    HAL_CRC_DeInit(&hcrc);
    HAL_I2C_DeInit(&hi2c1);	
    HAL_ADC_DeInit(&hadc1);	
	HAL_SPI_DeInit(&hspi1);
	HAL_UART_DeInit(&huart1);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15);
#ifndef USE_DEBUGGER
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_13|GPIO_PIN_14);
#endif    
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|
                           GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_9|
                           GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_13);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0|GPIO_PIN_1);
    __HAL_RCC_GPIOA_CLK_DISABLE();
	__HAL_RCC_GPIOB_CLK_DISABLE();    
	__HAL_RCC_GPIOC_CLK_DISABLE();
	__HAL_RCC_GPIOD_CLK_DISABLE();
    HAL_RCC_DeInit();
    HAL_DeInit();
    HAL_NVIC_SystemReset();
}
void PresentSystem(void){
    char buf[64];
    uint16_t AD_value, V25 = 1663;
    ADC_ChannelConfTypeDef sConfig;
    uint32_t romsz = *(__IO uint16_t *)(0x1FFFF7E0);
    float TemperatureC, McuVoltage, Avg_Slope = 5.33f;
    if (WaitForIdleLine(100) == false){
        HAL_Delay(tfifa); // wait for backof time and start again
        _Error_Handler(__FILE__,__LINE__); // after restart 
    }        
    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 5);
    AD_value = HAL_ADC_GetValue(&hadc1);
    TemperatureC = (((V25-AD_value)/Avg_Slope) + 25.0f);
    sConfig.Channel = ADC_CHANNEL_VREFINT;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 5);
    AD_value = HAL_ADC_GetValue(&hadc1);
    McuVoltage = (4095.0f * 1.15f / (float)AD_value);
    HAL_UART_AbortReceive_IT(&huart1);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    sprintf(buf, "\r\nCore Clock Freqency: %d MHz\r\n", HAL_RCC_GetSysClockFreq()/1000000);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "Core Supply Voltage: %.3f mV\r\n", McuVoltage);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "Core Temperature: %.1f 'C\r\n", TemperatureC);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    if (romsz >= 0x400) {
        romsz /= 0x400;
        sprintf(buf, "Flash size: %d MB\r\n", romsz); 
    } else sprintf(buf, "Flash size: %d KB\r\n", romsz); 
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "Device ID: %d\r\n", HAL_GetDEVID());
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "Device Unique ID: %d-%d-%d\r\n", HAL_GetUIDw0(),HAL_GetUIDw1(),HAL_GetUIDw2());
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_UART_Receive_IT(&huart1, &rec, 1);
}

bool WaitForIdleLine(uint8_t loop){
    uint8_t recb, i, sta;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
    while (loop--){// wait for 5 ms idle bus line and set interface address as backof time
        if (HAL_UART_Receive(&huart1, &recb, 1, 5) == HAL_TIMEOUT){ // this will give
            i = tfifa;
            do{// lover addresse number higher pririty to comunicate on bus
                sta = HAL_UART_Receive(&huart1, &recb, 1, 1);
            }while ((sta == HAL_TIMEOUT) && (i--));
            if (sta == HAL_TIMEOUT){ // after another 5 ms of idle line     
                if (HAL_UART_Receive(&huart1, &recb, 1, 5) == HAL_TIMEOUT){
                    return true; // start sending self presentation to bus 
                }// try again
            }// try again
        }// try again
    }// or wait next free bus slot
    return false; 
}
static void SystemClock_Config(void){
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
        _Error_Handler(__FILE__,__LINE__);
    }
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK){
        _Error_Handler(__FILE__,__LINE__);
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
        _Error_Handler(__FILE__,__LINE__);
    }
}
static void MX_CRC_Init(void){
	hcrc.Instance = CRC;
	HAL_CRC_Init(&hcrc);
}
static void MX_ADC1_Init(void){
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 2;
    HAL_ADC_Init(&hadc1);
}
static void MX_I2C1_Init(void){
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	
    if (HAL_I2C_DeInit(&hi2c1) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
    
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

}
static void MX_SPI1_Init(void){
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 10;	
    if (HAL_SPI_Init(&hspi1) != HAL_OK){
		_Error_Handler(__FILE__, __LINE__);
	}
}

static void MX_UART_Init(void){
	huart1.Instance = USART1;
	huart1.Init.BaudRate = tfbps;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;	
    if (HAL_UART_Init(&huart1) != HAL_OK){
		_Error_Handler(__FILE__, __LINE__);
	}
}
static void MX_GPIO_Init(void){

	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();    
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();


	/*Configure GPIO pin Output Level */

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_12 
						  |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);
	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_11, GPIO_PIN_SET);

	/*Configure GPIO pin : PC13 */
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PD0 PD1 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pins : PA3 PA4 PA8 */
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB0 PB10 PB11 PB12 
						   PB4 PB5 PB8 PB9 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12 
						  |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PB1 PB2 PB3 */
	GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
#ifdef USE_DEBUGGER
	GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15;
#else
	GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
#endif

	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure peripheral I/O remapping */
	__HAL_AFIO_REMAP_PD01_ENABLE();
}


static void RAM_Init(void){
    uint8_t ebuf[EE_INIT_ADDR+2],i;
    ZEROFILL(ebuf, COUNTOF(ebuf));
    ebuf[0] = 0;
    ebuf[1] = 0;
    HAL_I2C_Master_Transmit  (&hi2c1,EEADDR, ebuf, 2, EETOUT);
    HAL_I2C_Master_Receive   (&hi2c1,EEADDR, ebuf, COUNTOF(ebuf), EETOUT);
    if (ebuf[EE_INIT_ADDR] != EE_INIT_MARK){        
        ebuf[0] = 0;
        ebuf[1] = EE_TFIFA;
        ebuf[2] = (HAL_GetUIDw2() & 0xff);
        ebuf[3] = DEF_TFGRA;
        ebuf[4] = DEF_TFBRA;
        ebuf[5] = DEF_TFGWA;
        ebuf[6] =((DEF_TFBPS >> 24) & 0xff);
        ebuf[7] =((DEF_TFBPS >> 16) & 0xff);
        ebuf[8] =((DEF_TFBPS >>  8) & 0xff);
        ebuf[9] = (DEF_TFBPS & 0xff);
        ebuf[10]=((DEF_SYSID >> 8) & 0xff);
        ebuf[11]= (DEF_SYSID & 0xff);
        HAL_I2C_Master_Transmit  (&hi2c1, EEADDR, ebuf, 12, EETOUT);
        if (HAL_I2C_IsDeviceReady(&hi2c1, EEADDR, EETIME, EETOUT) != HAL_OK){
            _Error_Handler(__FILE__, __LINE__);
        }    
        ebuf[0] = 0;
        ebuf[1] = EE_NTC_BYPASS_VOLTAGE_ADDRESS;
        ebuf[2] = ((NTC_BYPASS_VOLTAGE_DEFAULT >> 8) & 0xff);
        ebuf[3] = (NTC_BYPASS_VOLTAGE_DEFAULT & 0xff);
        HAL_I2C_Master_Transmit  (&hi2c1, EEADDR, ebuf, 4, EETOUT);
        if (HAL_I2C_IsDeviceReady(&hi2c1, EEADDR, EETIME, EETOUT) != HAL_OK){
            _Error_Handler(__FILE__, __LINE__);
        }
        ebuf[0] = 0;
        ebuf[1] = EE_NTC_OPEN_VOLTAGE_ADDRESS;
        ebuf[2] = ((NTC_OPEN_VOLTAGE_DEFAULT >> 8) & 0xff);
        ebuf[3] = (NTC_OPEN_VOLTAGE_DEFAULT & 0xff);
        HAL_I2C_Master_Transmit  (&hi2c1, EEADDR, ebuf, 4, EETOUT);
        if (HAL_I2C_IsDeviceReady(&hi2c1, EEADDR, EETIME, EETOUT) != HAL_OK){
            _Error_Handler(__FILE__, __LINE__);
        }        
        ebuf[0] = 0;
        ebuf[1] = EE_NTC_WARNING_TEMPERATURE_ADDRESS;
        ebuf[2] = ((NTC_WARNING_TEMPERATURE_DEFAULT >> 8) & 0xff);
        ebuf[3] = (NTC_WARNING_TEMPERATURE_DEFAULT & 0xff);
        HAL_I2C_Master_Transmit  (&hi2c1, EEADDR, ebuf, 4, EETOUT);
        if (HAL_I2C_IsDeviceReady(&hi2c1, EEADDR, EETIME, EETOUT) != HAL_OK){
            _Error_Handler(__FILE__, __LINE__);
        }        
        ebuf[0] = 0;
        ebuf[1] = EE_NTC_SHUTDOWN_TEMPERATURE_ADDRESS;
        ebuf[2] = ((NTC_SHUTDOWN_TEMPERATURE_DEFAULT >> 8) & 0xff);
        ebuf[3] = (NTC_SHUTDOWN_TEMPERATURE_DEFAULT & 0xff);
        HAL_I2C_Master_Transmit  (&hi2c1, EEADDR, ebuf, 4, EETOUT);
        if (HAL_I2C_IsDeviceReady(&hi2c1, EEADDR, EETIME, EETOUT) != HAL_OK){
            _Error_Handler(__FILE__, __LINE__);
        }        
        ebuf[0] = 0;
        ebuf[1] = EE_SUPPLY_VOLTAGE_SHUTDOWN_ADDRESS;
        ebuf[2] = ((SUPPLY_VOLTAGE_SHUTDOWN_DEFAULT >> 8) & 0xff);
        ebuf[3] = (SUPPLY_VOLTAGE_SHUTDOWN_DEFAULT & 0xff);
        HAL_I2C_Master_Transmit  (&hi2c1, EEADDR, ebuf, 4, EETOUT);
        if (HAL_I2C_IsDeviceReady(&hi2c1, EEADDR, EETIME, EETOUT) != HAL_OK){
            _Error_Handler(__FILE__, __LINE__);
        }
        ebuf[0] = 0;
        ebuf[1] = EE_PWM_0_15_FREQUENCY_ADDRESS;
        ebuf[2] = ((PWM_0_15_FREQUENCY_DEFAULT >> 8) & 0xff);
        ebuf[3] = (PWM_0_15_FREQUENCY_DEFAULT & 0xff);
        HAL_I2C_Master_Transmit  (&hi2c1, EEADDR, ebuf, 4, EETOUT);
        if (HAL_I2C_IsDeviceReady(&hi2c1, EEADDR, EETIME, EETOUT) != HAL_OK){
            _Error_Handler(__FILE__, __LINE__);
        }        
        ebuf[0] = 0;
        ebuf[1] = EE_PWM_16_31_FREQUENCY_ADDRESS;
        ebuf[2] = ((PWM_16_31_FREQUENCY_DEFAULT >> 8) & 0xff);
        ebuf[3] = (PWM_16_31_FREQUENCY_DEFAULT & 0xff);
        HAL_I2C_Master_Transmit  (&hi2c1, EEADDR, ebuf, 4, EETOUT);
        if (HAL_I2C_IsDeviceReady(&hi2c1, EEADDR, EETIME, EETOUT) != HAL_OK){
            _Error_Handler(__FILE__, __LINE__);
        }
        ZEROFILL(ebuf, COUNTOF(ebuf));
        ebuf[0] = 0;
        ebuf[1] = EE_TFTYPE1_ADDR;
        ebuf[2] = S_BINARY;
        ebuf[3] = S_LIGHT;
        HAL_I2C_Master_Transmit  (&hi2c1, EEADDR, ebuf, 18, EETOUT);
        if (HAL_I2C_IsDeviceReady(&hi2c1, EEADDR, EETIME, EETOUT) != HAL_OK){
            _Error_Handler(__FILE__, __LINE__);
        }
        ebuf[0] = 0;
        ebuf[1] = EE_INIT_ADDR;
        ebuf[2] = EE_INIT_MARK;
        HAL_I2C_Master_Transmit  (&hi2c1, EEADDR, ebuf, 3, EETOUT);
        if (HAL_I2C_IsDeviceReady(&hi2c1, EEADDR, EETIME, EETOUT) != HAL_OK){
            _Error_Handler(__FILE__, __LINE__);
        }       
        ZEROFILL(ebuf, COUNTOF(ebuf));
        ebuf[0] = 0;
        ebuf[1] = 0;
        HAL_I2C_Master_Transmit  (&hi2c1,EEADDR, ebuf, 2, EETOUT);
        HAL_I2C_Master_Receive   (&hi2c1,EEADDR, ebuf, COUNTOF(ebuf), EETOUT);
    }    

    sysfl   = ebuf[EE_SYS_STATE];
    tfifa   = ebuf[EE_TFIFA];   // device address
    tfgra   = ebuf[EE_TFGRA];   // group address
    tfbra   = ebuf[EE_TFBRA];   // broadcast address
    tfgwa   = ebuf[EE_TFGWA];   // gateway address
    tfbps   =(ebuf[EE_TFBPS]<<24)
            |(ebuf[EE_TFBPS+1]<<16)
            |(ebuf[EE_TFBPS+2]<<8)
            | ebuf[EE_TFBPS+3]; // interface baudrate
    sysid   =(ebuf[EE_SYSID]<<8)
            | ebuf[EE_SYSID+1]; // system id (system unique number)
	ntc_bypass_voltage          = ((ebuf[EE_NTC_BYPASS_VOLTAGE_ADDRESS]<<8) + ebuf[EE_NTC_BYPASS_VOLTAGE_ADDRESS+1]);
	ntc_open_voltage            = ((ebuf[EE_NTC_OPEN_VOLTAGE_ADDRESS]<<8) + ebuf[EE_NTC_OPEN_VOLTAGE_ADDRESS+1]);
	ntc_warning_temperature     = ((ebuf[EE_NTC_WARNING_TEMPERATURE_ADDRESS]<<8) + ebuf[EE_NTC_WARNING_TEMPERATURE_ADDRESS+1]);
	ntc_shutdown_temperature    = ((ebuf[EE_NTC_SHUTDOWN_TEMPERATURE_ADDRESS]<<8) + ebuf[EE_NTC_SHUTDOWN_TEMPERATURE_ADDRESS+1]);
	psu_shutdown_voltage        = ((ebuf[EE_SUPPLY_VOLTAGE_SHUTDOWN_ADDRESS]<<8) + ebuf[EE_SUPPLY_VOLTAGE_SHUTDOWN_ADDRESS+1]);
	pwm_0_15_freq               = ((ebuf[EE_PWM_0_15_FREQUENCY_ADDRESS]<<8) + ebuf[EE_PWM_0_15_FREQUENCY_ADDRESS+1]);
	pwm_16_31_freq              = ((ebuf[EE_PWM_16_31_FREQUENCY_ADDRESS]<<8) + ebuf[EE_PWM_16_31_FREQUENCY_ADDRESS+1]);
    for (i = 0; i < 16; i++){
        tftype[i] = ebuf[EE_TFTYPE1_ADDR+i];
        if (tftype[i] == 0xff) tftype[i] = 0;
    }
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
