#include "stm32f4xx.h"
#include "lwip/opt.h"
#include "stm32f4xx_it.h"

uint32_t sys_now(void)
{
  return  Get_SysTick();
}
