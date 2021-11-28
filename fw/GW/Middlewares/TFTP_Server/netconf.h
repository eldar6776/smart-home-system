#ifndef __NETCONF_H__
#define __NETCONF_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variable----------------------------------------------------------*/
extern struct netif netif;
extern struct ip_addr new_ip;
extern struct ip_addr new_nm;
extern struct ip_addr new_gw;
extern uint8_t iptxt[20];
extern uint8_t sbnttxt[20];
extern uint8_t gtwtxt[20];
extern uint8_t ip_add[4];
extern uint8_t subnet[4];
extern uint8_t gw_add[4];
/* Exported functions ------------------------------------------------------- */
void LwIP_Init(void);
void LwIP_Pkt_Handle(void);
void LwIP_Periodic_Handle(__IO uint32_t localtime);


#ifdef __cplusplus
}
#endif

#endif /* __NETCONF_H__ */

