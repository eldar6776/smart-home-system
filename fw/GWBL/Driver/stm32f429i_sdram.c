/**
  * @file    stm32f429i_sdram.c 
  * @author  WB R&D Team - openmcu666
  * @version V0.1
  * @date    2015.6.25
  * @brief   fmc_sdram Driver
  */
#include "stm32f429i_sdram.h"
#include "stm32f4xx_fmc.h"


#ifndef USE_Delay
static void delay(__IO uint32_t nCount);
#endif /* USE_Delay*/

/**
  * @brief  Configures the FMC and GPIOs to interface with the SDRAM memory.
  *         This function must be called before any read/write operation
  *         on the SDRAM.
  * @param  None
  * @retval None
  */
void SDRAM_Init(void)
{
	FMC_SDRAMInitTypeDef  FMC_SDRAMInitStructure;
	FMC_SDRAMTimingInitTypeDef  FMC_SDRAMTimingInitStructure; 

	SDRAM_GPIOConfig();

	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FMC, ENABLE);
	/****************************************************************************
	* TMRD = 40ns
	* TXSR = 80ns
	* TRAS = 44ns
	* TRC  = 70ns
	* TWR  = 40ns
	* TRP  = 20ns
	* TRCD = 20ns
	***************************************************************************/
	/* TMRD: 2 Clock cycles */	
	FMC_SDRAMTimingInitStructure.FMC_LoadToActiveDelay = 4;
	/* TXSR: min=72ns (6x11.90ns) */      
	FMC_SDRAMTimingInitStructure.FMC_ExitSelfRefreshDelay = 8;  
	/* TRAS: min=42ns (4x11.90ns) max=100k (ns) */
	FMC_SDRAMTimingInitStructure.FMC_SelfRefreshTime = 4; 
	/* TRC:  min=60 (6x11.90ns) */
	FMC_SDRAMTimingInitStructure.FMC_RowCycleDelay = 7; 
	/* TWR:  2 Clock cycles */
	FMC_SDRAMTimingInitStructure.FMC_WriteRecoveryTime = 4;
	/* TRP:  15ns => 2x11.90ns */
	FMC_SDRAMTimingInitStructure.FMC_RPDelay = 2;
	/* TRCD: 15ns => 2x11.90ns */ 
	FMC_SDRAMTimingInitStructure.FMC_RCDDelay = 2; 	

	FMC_SDRAMInitStructure.FMC_Bank = FMC_Bank2_SDRAM;
	FMC_SDRAMInitStructure.FMC_ColumnBitsNumber = FMC_ColumnBits_Number_9b;
	FMC_SDRAMInitStructure.FMC_RowBitsNumber = FMC_RowBits_Number_13b;
	FMC_SDRAMInitStructure.FMC_SDMemoryDataWidth = FMC_SDMemory_Width_16b;
	FMC_SDRAMInitStructure.FMC_InternalBankNumber = FMC_InternalBank_Number_4;
	FMC_SDRAMInitStructure.FMC_CASLatency = FMC_CAS_Latency_3; 
	FMC_SDRAMInitStructure.FMC_WriteProtection = FMC_Write_Protection_Disable;
	FMC_SDRAMInitStructure.FMC_SDClockPeriod = FMC_SDClock_Period_2;  //180/2
	FMC_SDRAMInitStructure.FMC_ReadBurst = FMC_Read_Burst_Enable;
	FMC_SDRAMInitStructure.FMC_ReadPipeDelay = FMC_ReadPipe_Delay_1;
	FMC_SDRAMInitStructure.FMC_SDRAMTimingStruct = &FMC_SDRAMTimingInitStructure;
	FMC_SDRAMInit(&FMC_SDRAMInitStructure);

	SDRAM_InitSequence();
}

/**
  * @brief  Configures all SDRAM memory I/Os pins. 
  * @param  None. 
  * @retval None.
  */
void SDRAM_GPIOConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*
	****************************************************************************
	* PH7 --> SDCKE1??  | PE0 --> NBL0  | PF0 --> A0    | PG0 --> A10            *
	* PH6 --> SDNE1??   | PE1 --> NBL1  | PF1 --> A1    | PG1 --> A11            *
	* ----------------|  PE7 --> D4   | PF2 --> A2    | PG2 --> A12            *
	* PC0 --> SDNWE   |  PE8 --> D5   | PF3 --> A3    | PG4 --> BA0            *
	* ----------------|  PE9 --> D6   | PF4 --> A4    | PG5 --> BA1            *
	* PD0 --> D2      |  PE10--> D7   | PF5 --> A5    | PG8 --> SDCLK          *
	* PD1 --> D3      |   PE11--> D8  | PF11--> SDNRAS| PG15--> SDNCAS         *
	* PD8 --> D13     |  PE12--> D9   | PF12--> A6    |                        *
	* PD9 --> D14     |  PE13--> D10  | PF13--> A7    |                        *
	* PD10--> D15     | PE14--> D11   | PF14--> A8    |                        *
	* PD14--> D0      |---------------| PF15--> A9    |                        *
	* PD15--> D1      |  PE15--> D12                                                       *
	****************************************************************************
	*/


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | 
						 RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOB |
												 RCC_AHB1Periph_GPIOH, ENABLE);

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	GPIO_PinAFConfig(GPIOH, GPIO_PinSource7, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource6, GPIO_AF_FMC);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7  | GPIO_Pin_6;
	GPIO_Init(GPIOH, &GPIO_InitStructure);	

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource0, GPIO_AF_FMC);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FMC);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  |GPIO_Pin_1  |GPIO_Pin_8 |GPIO_Pin_9 |
								GPIO_Pin_10 |GPIO_Pin_14 |GPIO_Pin_15;
	GPIO_Init(GPIOD, &GPIO_InitStructure);	

	GPIO_PinAFConfig(GPIOE, GPIO_PinSource0 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource1 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FMC);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_7 | GPIO_Pin_8  |
								GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11| GPIO_Pin_12 |
								GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOF, GPIO_PinSource0 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource1 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource2 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource3 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource4 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource5 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource11 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource12 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource13 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource14 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource15 , GPIO_AF_FMC);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3  |
								GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_11 | GPIO_Pin_12 |
								GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;      
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOG, GPIO_PinSource0 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource1 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource2 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource4 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource5 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource8 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource15 , GPIO_AF_FMC);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_4 
								| GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_15;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
}

/**
  * @brief  Executes the SDRAM memory initialization sequence. 
  * @param  None. 
  * @retval None.
  */
void SDRAM_InitSequence(void)
{
  FMC_SDRAMCommandTypeDef FMC_SDRAMCommandStructure;
  uint32_t tmpr = 0;
  
/* Step 3 --------------------------------------------------------------------*/
  /* Configure a clock configuration enable command */
  FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_CLK_Enabled;
  FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank2;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 1;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;
  /* Wait until the SDRAM controller is ready */ 
  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET)
  {
  }
  /* Send the command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);  
  
/* Step 4 --------------------------------------------------------------------*/
  /* Insert 100 ms delay */
  delay(10);
    
/* Step 5 --------------------------------------------------------------------*/
  /* Configure a PALL (precharge all) command */ 
  FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_PALL;
  FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank2;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 4;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;
  /* Wait until the SDRAM controller is ready */ 
  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET)
  {
  }
  /* Send the command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);
  
/* Step 6 --------------------------------------------------------------------*/
  /* Configure a Auto-Refresh command */ 
  FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_AutoRefresh;
  FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank2;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 2;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;
  /* Wait until the SDRAM controller is ready */ 
  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET)
  {
  }
  /* Send the  first command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);
  
  /* Wait until the SDRAM controller is ready */ 
  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET)
  {
  }
  /* Send the second command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);
  
/* Step 7 --------------------------------------------------------------------*/
  /* Program the external memory mode register */
  tmpr = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2          |
                   SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                   SDRAM_MODEREG_CAS_LATENCY_3           |
                   SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                   SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
  
  /* Configure a load Mode register command*/ 
  FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_LoadMode;
  FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank2;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 1;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = tmpr;
  /* Wait until the SDRAM controller is ready */ 
  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET)
  {
  }
  /* Send the command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);
  
/* Step 8 --------------------------------------------------------------------*/

  /* Set the refresh rate counter */
  /* (7.81 us x Freq) - 20 */
  /* Set the device refresh counter */
  FMC_SetRefreshCount(33);
  /* Wait until the SDRAM controller is ready */ 
  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET)
  {
  }
}


/**
  * @brief  Writes a Entire-word buffer to the SDRAM memory. 
  * @param  pBuffer: pointer to buffer. 
  * @param  uwWriteAddress: SDRAM memory internal address from which the data will be 
  *         written.
  * @param  uwBufferSize: number of words to write. 
  * @retval None.
  */
void SDRAM_WriteBuffer(uint32_t* pBuffer, uint32_t uwWriteAddress, uint32_t uwBufferSize)
{
  __IO uint32_t write_pointer = (uint32_t)uwWriteAddress;

  /* Disable write protection */
  FMC_SDRAMWriteProtectionConfig(FMC_Bank2_SDRAM, DISABLE);
  
  /* Wait until the SDRAM controller is ready */ 
  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET)
  {
  }

  /* While there is data to write */
  for (; uwBufferSize != 0; uwBufferSize--) 
  {
    /* Transfer data to the memory */
    *(uint32_t *) (SDRAM_BANK_ADDR + write_pointer) = *pBuffer++;

    /* Increment the address*/
    write_pointer += 4;
  }
    
}

/**
  * @brief  Reads data buffer from the SDRAM memory. 
  * @param  pBuffer: pointer to buffer. 
  * @param  ReadAddress: SDRAM memory internal address from which the data will be 
  *         read.
  * @param  uwBufferSize: number of words to write. 
  * @retval None.
  */
void SDRAM_ReadBuffer(uint32_t* pBuffer, uint32_t uwReadAddress, uint32_t uwBufferSize)
{
  __IO uint32_t write_pointer = (uint32_t)uwReadAddress;
  
   
  /* Wait until the SDRAM controller is ready */ 
  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET)
  {
  }
  
  /* Read data */
  for(; uwBufferSize != 0x00; uwBufferSize--)
  {
   *pBuffer++ = *(__IO uint32_t *)(SDRAM_BANK_ADDR + write_pointer );
    
   /* Increment the address*/
    write_pointer += 4;
  } 
}

#ifndef USE_Delay
/**
  * @brief  Inserts a delay time.
  * @param  nCount: specifies the delay time length.
  * @retval None
  */
static void delay(__IO uint32_t nCount)
{
  __IO uint32_t index = 0; 
  for(index = (100000 * nCount); index != 0; index--)
  {
  }
}
#endif /* USE_Delay */


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */  


