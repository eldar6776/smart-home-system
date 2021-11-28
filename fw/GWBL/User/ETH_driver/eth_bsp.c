/**
  * @file    eth_bsp.c 
  * @author  WB R&D Team - openmcu666
  * @version V0.1
  * @date    2015.6.24
  * @brief   ETH Driver
  */
#include "stm32f4x7_eth.h"
#include "main.h"
#include "eth_bsp.h"

__IO uint32_t  EthInitStatus = 0;
/**
  * @brief  ETH���ų�ʼ��
  * @param  None
  * @retval None
  */
static void eth_gpio_init(void)
{
  /*
   ****************************************************************************
   * ETH ������RMIIģʽ
   * PA1  -->  RMII_REF_CLK        |  PG14 -->  ETH_RMII_TXD1
   * PA2  -->  ETH_MDIO            |  PC1  -->  ETH_MDC
   * PA7  -->  ETH_RMII_CRS_DV     |  PC4  -->  ETH_RMII_RXD0
   * PB11 -->  ETH_RMII_TX_EN      |  PC5  -->  ETH_RMII_RXD1
   * PG13 -->  ETH_RMII_TXD0       |  PH3  -->  NRST
   ****************************************************************************
   */
  volatile uint32_t i;
  GPIO_InitTypeDef GPIO_InitStructure;
	
 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB
                       | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE
	                     | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOH , ENABLE); //ʹ��GPIOʱ�� 

 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);                         //ʹ��SYSCFGʱ�� 
  
  
  SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);               //����ΪRMIIģʽ


  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);


  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 ;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_ETH);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14 ;
  GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource13, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_ETH);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);
	

  /* ����LAN8720��λpin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOH, &GPIO_InitStructure);

  GPIO_ResetBits(GPIOH, GPIO_Pin_3);	
  for (i = 0; i < 200000; i++);
  GPIO_SetBits(GPIOH, GPIO_Pin_3);
  for (i = 0; i < 200000; i++);
}

/**
  * @brief  ETH DMA��ʼ��
  * @param  None
  * @retval None
  */
static void eth_macdma_init(void)
{
  ETH_InitTypeDef ETH_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx |
                        RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);
                        
  ETH_DeInit();                                                                  //Ӳ����λeth 
	
  ETH_SoftwareReset();
 
  while (ETH_GetSoftwareResetStatus() == SET);                                   //�ȴ�eth�����λ��� 
  
  ETH_StructInit(&ETH_InitStructure);                                             //Ĭ��ETH��ʼ������ 

  /* Fill ETH_InitStructure parametrs */
  /*------------------------   MAC   -----------------------------------*/
  ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
  ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
  ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
  ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
  ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
  ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
  ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
  ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
  ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
#ifdef CHECKSUM_BY_HARDWARE
  ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif

  /*------------------------   DMA   -----------------------------------*/  
  
  /* When we use the Checksum offload feature, we need to enable the Store and Forward mode: 
  the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum, 
  if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
  ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; 
  ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;         
  ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;     
 
  ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;       
  ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;   
  ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
  ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;      
  ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;                
  ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;          
  ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
  ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

  /* Ethernet��ʼ */
  EthInitStatus = ETH_Init(&ETH_InitStructure, LAN8720_PHY_ADDRESS);
}

/**
  * @brief  ETH���ź� DMA��ʼ��
  * @param  None
  * @retval None
  */
void ETHERNET_Init(void)
{
 
  eth_gpio_init();            //����ETH��GPIO 
  
  eth_macdma_init();          //mac��dma���� 

  if (EthInitStatus == 0) {
    while (1);
  }
}

