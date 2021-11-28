/* USER CODE BEGIN Header */
/**
******************************************************************************
* @file           : main.c
* @brief          : Stunning teen brunette rubs her clit until she cums
******************************************************************************
* @attention
*
* <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
* All rights reserved.</center></h2>
*
* This software component is licensed by ST under BSD 3-Clause license,
* the "License"; You may not use this file except in compliance with the
* License. You may obtain a copy of the License at:
*                        opensource.org/licenses/BSD-3-Clause
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define I2CEXP0_WRADD       0x4A
#define I2CEXP0_RDADD       0x4B
#define I2CEXP1_WRADD       0x4C
#define I2CEXP1_RDADD       0x4D
#define I2CEXP2_WRADD       0x4E
#define I2CEXP2_RDADD       0x4F
#define I2CPWM0_WRADD       0x90
#define I2CPWM0_RDADD       0x91
#define I2CPWM1_WRADD       0x92
#define I2CPWM1_RDADD       0x93
#define I2CPWM_TOUT         15

/* Private macro -------------------------------------------------------------*/
#define PWM_CalculatePrescale(FREQUNCY)			(PCA9685_PRE_SCALE_REGISTER = ((25000000U / (4096U * FREQUNCY)) - 1U))
#define PWM_OuputEnable()						(HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET))
#define PWM_OuputDisable()						(HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET))
/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;
I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart1;
CRC_HandleTypeDef hcrc;
TinyFrame tfapp;
uint16_t sysid;
uint8_t  tftype[16];
bool init_tf = false;
bool pwminit = true;
uint16_t pwm_0_15_freq;
uint16_t pwm_16_31_freq;
uint8_t pwm[32], rel[3] = {0,0,0};
uint8_t pca9685_register[PCA9685_REGISTER_SIZE];
uint32_t rstmr, rsflg, tfbps;
uint8_t  tfifa, tfgra, tfbra, tfgwa, rec;
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_CRC_Init(void);
void MX_ADC_Init(void);
void MX_GPIO_Init(void);
void MX_I2C1_Init(void);
void MX_UART_Init(void);
void MX_SPI1_Init(void);
void RS485_Init(void);
void PCA9685_Init(void);
void PCA9685_Reset(void);
void PCA9685_OutputUpdate(void);
void PCA9685_SetOutputFrequency(uint16_t frequency);
TF_Result ID_Listener(TinyFrame *tf, TF_Msg *msg);
TF_Result GEN_Listener(TinyFrame *tf, TF_Msg *msg);
TF_Result TYPE_Listener(TinyFrame *tf, TF_Msg *msg);
/* Private user code ---------------------------------------------------------*/
/**
* @brief  The application entry point.
* @retval int
*/
int main(void){
    HAL_Init();
    SystemClock_Config();
    MX_CRC_Init();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_UART_Init();
    RS485_Init();
    ZEROFILL(pwm, 32);
    ZEROFILL(pca9685_register, PCA9685_REGISTER_SIZE);
    PCA9685_Reset();
    PCA9685_Init();
    if(pwminit) PCA9685_SetOutputFrequency(PWM_0_15_FREQUENCY_DEFAULT);		
    if(pwminit) PWM_OuputEnable();
    HAL_I2C_Master_Transmit(&hi2c1, I2CEXP0_WRADD, &rel[0], 1, I2CPWM_TOUT);
    HAL_I2C_Master_Transmit(&hi2c1, I2CEXP1_WRADD, &rel[1], 1, I2CPWM_TOUT);
    HAL_I2C_Master_Transmit(&hi2c1, I2CEXP2_WRADD, &rel[2], 1, I2CPWM_TOUT);
    PresentSystem();
    while (1){
    }
}
void PresentSystem(void){
    char buf[64];
    int32_t x;
    uint16_t AD_value, V25 = 1663;
    float TemperatureC, McuVoltage, Avg_Slope = 5.33f;  
    ADC_ChannelConfTypeDef sConfig;
    __HAL_RCC_ADC1_CLK_ENABLE();
    hadc.Instance = ADC1;
	hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc.Init.Resolution = ADC_RESOLUTION_12B;
	hadc.Init.ScanConvMode = DISABLE;
	hadc.Init.ContinuousConvMode = DISABLE;
	hadc.Init.DiscontinuousConvMode = DISABLE;
	hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc.Init.DMAContinuousRequests = DISABLE;
	hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	HAL_ADC_Init(&hadc);
    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc, &sConfig);
    HAL_ADC_Start(&hadc);
    HAL_ADC_PollForConversion(&hadc, 5);
    AD_value = HAL_ADC_GetValue(&hadc);
    TemperatureC = (((V25-AD_value)/Avg_Slope) + 25.0f);
    sConfig.Channel = ADC_CHANNEL_VREFINT;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc, &sConfig);
    HAL_ADC_Start(&hadc);
    HAL_ADC_PollForConversion(&hadc, 5);
    AD_value = HAL_ADC_GetValue(&hadc);
    McuVoltage = (4095.0f * 1.35f / (float)AD_value);
    x = HAL_GetUIDw0();
    while (x>10000) x /= 10;
    HAL_Delay(x);
    strcpy(buf, "\r\n\r\n");
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    strcpy(buf, "--------------------------------------------------------\r\n");
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    strcpy(buf, "LuxHOME Intergrated Controller\r\n");
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    strcpy(buf, "--------------------------------------------------------\r\n");
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    strcpy(buf, "Flash size: 16 KB\r\n"); 
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "Core Clock Freqency: %d MHz\r\n", HAL_RCC_GetSysClockFreq()/1000000);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "Core Supply Voltage: %.3f mV\r\n", McuVoltage);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "Core Temperature: %.1f 'C\r\n", TemperatureC);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "ID Device: %d\r\n", HAL_GetDEVID());
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "ID Unique: %d-%d-%d\r\n", HAL_GetUIDw0(),HAL_GetUIDw1(),HAL_GetUIDw2());
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    sprintf(buf, "Version Firmware: RE%X\r\n", *(__IO uint32_t*)(0x08002008)&0x00FFFFFF);
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    strcpy(buf, "--------------------------------------------------------\r\n\r\n");
    HAL_UART_Transmit(&huart1,(uint8_t*)buf, strlen(buf), RESP_TOUT);
    HAL_UART_Receive_IT(&huart1, &rec, 1);
}
/**
  * @brief
  * @param
  * @retval
  */
TF_Result ID_Listener(TinyFrame *tf, TF_Msg *msg){
    return TF_CLOSE;
}
/**
  * @brief
  * @param
  * @retval
  */
TF_Result BINARY_Listener(TinyFrame *tf, TF_Msg *msg){
    uint8_t idx, pos, cnt = 0;    
    while((cnt < (msg->len/2))&&(msg->data[cnt*2])){
        idx = (msg->data[cnt*2]-1)/8;
        pos = (msg->data[cnt*2]-1)-(idx*8);
        if   (!msg->data[cnt*2+1])  rel[idx] &= ~(1U<<pos);
        else                        rel[idx] |=  (1U<<pos);
        HAL_I2C_Master_Transmit(&hi2c1, I2CEXP0_WRADD+(idx*2), &rel[idx], 1, I2CPWM_TOUT);
        ++cnt;
    }
    TF_Respond(tf, msg);
    return TF_STAY;
}
/**
  * @brief
  * @param
  * @retval
  */
TF_Result DIMMER_Listener(TinyFrame *tf, TF_Msg *msg){  
    uint8_t cnt = 0;
    while((cnt < (msg->len/2))&&(msg->data[cnt*2])){
        pwm[msg->data[cnt*2]-1] = msg->data[(cnt*2)+1];
        ++cnt;
    }
    if (pwminit) PCA9685_OutputUpdate();
    TF_Respond(tf, msg);
    return TF_STAY;
}
/**
  * @brief
  * @param
  * @retval
  */
TF_Result GEN_Listener(TinyFrame *tf, TF_Msg *msg){    
    return TF_STAY;
}
/**
  * @brief
  * @param
  * @retval
  */
void RS485_Init(void){
    if(!init_tf){
        init_tf = TF_InitStatic(&tfapp, TF_SLAVE);
        TF_AddGenericListener(&tfapp, GEN_Listener);
        TF_AddTypeListener(&tfapp, S_BINARY, BINARY_Listener);
        TF_AddTypeListener(&tfapp, S_DIMMER, DIMMER_Listener);
    }
	HAL_UART_Receive_IT(&huart1, &rec, 1);
}
/**
  * @brief
  * @param
  * @retval
  */
void RS485_Tick(void){
    if (init_tf == true) {
        TF_Tick(&tfapp);
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len){
    HAL_UART_Transmit(&huart1,(uint8_t*)buff, len, RESP_TOUT);
    HAL_UART_Receive_IT(&huart1, &rec, 1);
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    TF_AcceptChar(&tfapp, rec);
    HAL_UART_Receive_IT(&huart1, &rec, 1);
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
    __HAL_UART_CLEAR_PEFLAG(&huart1);
    __HAL_UART_CLEAR_FEFLAG(&huart1);
    __HAL_UART_CLEAR_NEFLAG(&huart1);
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    HAL_UART_AbortReceive(&huart1);
	HAL_UART_Receive_IT(&huart1, &rec, 1);
}
/**
* @brief System Clock Configuration
* @retval None
*/
void SystemClock_Config(void){
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    /** Initializes the CPU, AHB and APB busses clocks 
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.HSI14CalibrationValue = 16;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
    RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB busses clocks 
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
              |RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK){
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
        Error_Handler();
    }
}
/**
* @brief ADC Initialization Function
* @param None
* @retval None
*/
void MX_CRC_Init(void){
	hcrc.Instance = CRC;
	HAL_CRC_Init(&hcrc);
}
/**
* @brief ADC Initialization Function
* @param None
* @retval None
*/
void MX_ADC_Init(void){
    ADC_ChannelConfTypeDef sConfig = {0};
    hadc.Instance = ADC1;
    hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
    hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc.Init.LowPowerAutoWait = DISABLE;
    hadc.Init.LowPowerAutoPowerOff = DISABLE;
    hadc.Init.ContinuousConvMode = DISABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc.Init.DMAContinuousRequests = DISABLE;
    hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    if (HAL_ADC_Init(&hadc) != HAL_OK){
        Error_Handler();
    }
    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK){
        Error_Handler();
    }
    sConfig.Channel = ADC_CHANNEL_VREFINT;
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK){
        Error_Handler();
    }
}

/**
* @brief I2C1 Initialization Function
* @param None
* @retval None
*/
void MX_I2C1_Init(void){
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x20303E5D;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK){
        Error_Handler();
    }
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK){
        Error_Handler();
    }
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK){
        Error_Handler();
    }
}
/**
* @brief SPI1 Initialization Function
* @param None
* @retval None
*/
void MX_SPI1_Init(void){
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 7;
    hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    if (HAL_SPI_Init(&hspi1) != HAL_OK){
        Error_Handler();
    }
}

/**
* @brief USART1 Initialization Function
* @param None
* @retval None
*/
void MX_UART_Init(void){
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_RS485Ex_Init(&huart1, UART_DE_POLARITY_HIGH, 0, 0) != HAL_OK){
        Error_Handler();
    }
    
}
/**
* @brief GPIO Initialization Function
* @param None
* @retval None
*/
void MX_GPIO_Init(void){
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(EEPROM_WR_GPIO_Port, EEPROM_WR_Pin, GPIO_PIN_RESET);
    /*Configure GPIO pin : EEPROM_WR_Pin */
    GPIO_InitStruct.Pin = EEPROM_WR_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(EEPROM_WR_GPIO_Port, &GPIO_InitStruct);
    /*Configure GPIO pin : HWADDR2_Pin */
    GPIO_InitStruct.Pin = HWADDR2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(HWADDR2_GPIO_Port, &GPIO_InitStruct);
    /*Configure GPIO pin : HWADDR0_Pin */
    GPIO_InitStruct.Pin = HWADDR0_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(HWADDR0_GPIO_Port, &GPIO_InitStruct);
    /*Configure GPIO pin : HWADDR1_Pin */
    GPIO_InitStruct.Pin = HWADDR1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(HWADDR1_GPIO_Port, &GPIO_InitStruct);
}
/**
* @brief  This function is executed in case of error occurrence.
* @retval None
*/
void Error_Handler(void){
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
tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

void PCA9685_Init(void){
	uint8_t buf[2];
	buf[0] = 0x00U;
	buf[1]= 0x00U;	
	if(HAL_I2C_Master_Transmit(&hi2c1, I2CPWM0_WRADD, buf, 2, I2CPWM_TOUT) != HAL_OK){
		pwminit = false;
	}
	
	HAL_Delay(5);
	buf[0] = 0x01U;
	buf[1]= 0x04U;	
	if(HAL_I2C_Master_Transmit(&hi2c1, I2CPWM0_WRADD, buf, 2, I2CPWM_TOUT) != HAL_OK){
        pwminit = false;
	}	
	HAL_Delay(2);
    buf[0] = 0x00U;
	buf[1]= 0x00U;	
	if(HAL_I2C_Master_Transmit(&hi2c1, I2CPWM0_WRADD, buf, 2, I2CPWM_TOUT) != HAL_OK){
		pwminit = false;
	}
	
	HAL_Delay(5);
	buf[0] = 0x01U;
	buf[1]= 0x04U;	
	if(HAL_I2C_Master_Transmit(&hi2c1, I2CPWM1_WRADD, buf, 2, I2CPWM_TOUT) != HAL_OK){
        pwminit = false;
	}	
	HAL_Delay(2);
}


void PCA9685_Reset(void){	
	uint8_t cmd = PCA9685_SW_RESET_COMMAND;	
	if(HAL_I2C_Master_Transmit(&hi2c1, PCA9685_GENERAL_CALL_ACK, &cmd, 1, I2CPWM_TOUT) != HAL_OK){
		pwminit = false;
	}
}


void PCA9685_SetOutputFrequency(uint16_t frequency){
	uint8_t buf[2];	
	buf[0] = 0x00U;
	buf[1]= 0x10U;	
	if(HAL_I2C_Master_Transmit(&hi2c1, I2CPWM0_WRADD, buf, 2, I2CPWM_TOUT) != HAL_OK){
		Error_Handler();	
	}	
	PWM_CalculatePrescale(frequency);	
	buf[0] = PCA9685_PRE_SCALE_REG_ADDRESS;
	buf[1]= PCA9685_PRE_SCALE_REGISTER;	
	if(HAL_I2C_Master_Transmit(&hi2c1, I2CPWM0_WRADD, buf, 2, I2CPWM_TOUT) != HAL_OK){
		Error_Handler();	
	}
	buf[0] = 0x00U;
	buf[1]= 0xa0U;	
	if(HAL_I2C_Master_Transmit(&hi2c1, I2CPWM0_WRADD, buf, 2, I2CPWM_TOUT) != HAL_OK){
		Error_Handler();
	}
	HAL_Delay(5);
	buf[0] = 0x01U;
	buf[1]= 0x04U;	
	if(HAL_I2C_Master_Transmit(&hi2c1, I2CPWM0_WRADD, buf, 2, I2CPWM_TOUT) != HAL_OK){
		Error_Handler();
	}
    buf[0] = 0x00U;
	buf[1]= 0x10U;	
	if(HAL_I2C_Master_Transmit(&hi2c1, I2CPWM1_WRADD, buf, 2, I2CPWM_TOUT) != HAL_OK){
		Error_Handler();	
	}
    HAL_Delay(5);
	PWM_CalculatePrescale(frequency);	
	buf[0] = PCA9685_PRE_SCALE_REG_ADDRESS;
	buf[1]= PCA9685_PRE_SCALE_REGISTER;	
	if(HAL_I2C_Master_Transmit(&hi2c1, I2CPWM1_WRADD, buf, 2, I2CPWM_TOUT) != HAL_OK){
		Error_Handler();	
	}
	buf[0] = 0x00U;
	buf[1]= 0xa0U;	
	if(HAL_I2C_Master_Transmit(&hi2c1, I2CPWM1_WRADD, buf, 2, I2CPWM_TOUT) != HAL_OK){
		Error_Handler();
	}
	HAL_Delay(5);
	buf[0] = 0x01U;
	buf[1]= 0x04U;	
	if(HAL_I2C_Master_Transmit(&hi2c1, I2CPWM1_WRADD, buf, 2, I2CPWM_TOUT) != HAL_OK){
		Error_Handler();
	}
}


void PCA9685_OutputUpdate(void){
	uint16_t pwm_out;
    uint8_t i,j,buf[70];	
    j = 6;
    for(i = 0; i < 16; i++){
        pca9685_register[j]= 0;
        j += 1;
        pca9685_register[j] = 0;
        j += 3;
	}
	j = 8;
    for(i = 0; i < 16; i++){
		pwm_out = pwm[i] * 16U;
        pca9685_register[j]= (pwm_out & 0xffU);
        j += 1;
        pca9685_register[j] = (pwm_out >> 8U);
        j += 3;
	}
    buf[0] = PCA9685_LED_0_ON_L_REG_ADDRESS;
    memcpy(&buf[1],&pca9685_register[6], 64);

	if(HAL_I2C_Master_Transmit(&hi2c1, I2CPWM0_WRADD, buf, 65, PWM_UPDATE_TIMEOUT) != HAL_OK){
		Error_Handler();
	}    
	j = 8;
    for(i = 16; i < 32; i++){
		pwm_out= pwm[i] * 16U;
        pca9685_register[j]= (pwm_out & 0xffU);
        j += 1;
        pca9685_register[j] = (pwm_out >> 8U);
        j += 3;
	}	    
    buf[0] = PCA9685_LED_0_ON_L_REG_ADDRESS;
    memcpy(&buf[1],&pca9685_register[6], 64);
	if(HAL_I2C_Master_Transmit(&hi2c1, I2CPWM1_WRADD, buf, 65, PWM_UPDATE_TIMEOUT) != HAL_OK){
		Error_Handler();
	}
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
