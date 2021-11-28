#ifndef __NETCONF_H__
#define __NETCONF_H__

#ifdef __cplusplus
extern "C" {
#endif
	
extern uint8_t iptxt[20];
extern uint8_t sbnttxt[20];
extern uint8_t gtwtxt[20];
/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void LwIP_Init(void);
void LwIP_Pkt_Handle(void);
void LwIP_Periodic_Handle(__IO uint32_t localtime);

#ifdef __cplusplus
}
#endif

#endif /* __NETCONF_H__ */

