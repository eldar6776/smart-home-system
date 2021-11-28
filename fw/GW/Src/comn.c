/**
 ******************************************************************************
 * File Name          : common.c
 * Date               : 21/08/2016 20:59:16
 * Description        : usefull function and macros set 
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "stm32f4xx_it.h"


/* Imported Type  ------------------------------------------------------------*/
/* Imported Variable  --------------------------------------------------------*/
/* Imported Function  --------------------------------------------------------*/
/* Private Type --------------------------------------------------------------*/
/* Private Define ------------------------------------------------------------*/
/* Private Variable ----------------------------------------------------------*/
/* Private Macro -------------------------------------------------------------*/
/* Private Function Prototype ------------------------------------------------*/
/* Program Code  -------------------------------------------------------------*/
void DelayMs(__IO uint32_t delay_time)
{
    __IO uint32_t init_time = Get_SysTick();
    
    while(delay_time)
    {
        if ((Get_SysTick() - init_time) >= delay_time)
        {
            delay_time = 0U;
        }
    }
}


uint8_t Bcd2Dec(uint8_t val)
{
	return(((val >> 4U) * 10U) + (val & 0x0fU));
}


uint8_t Dec2Bcd(uint8_t val)
{
	uint32_t r = 0U;
    
	while(val > 9U)
	{
		val -= 10U;
		++r;
	}

	r <<= 4U;
	r += val;
	return (r);
}


void CharToBinStr(char c, char *pstr) 
{
	*(unsigned long long*)pstr = 3472328296227680304ULL +
    (((c * 9241421688590303745ULL) / 128) & 72340172838076673ULL);
}


uint32_t GetSize(const uint8_t *pbuf) 
{
    uint32_t size;
    
	for (size = 0U; *pbuf++; size++)
    {
    }
    
	return size;
}


uint32_t BaseToPower(uint16_t base, uint8_t power)
{   
	uint32_t result = 1U;
    
	while(power--) result *= (uint32_t)base;
	return (result);
}


uint8_t CalcCRC(const uint8_t *pbuf, uint16_t size)
{
	uint32_t g = 0U;
 
    do
    {
        g += *pbuf;
        ++pbuf;
        --size;
        
    }while(size != 0U);

	return(((~g) & 0xffU) + 1U);
}


int32_t Str2Int(const char *pstr, uint8_t str_size)
{
    int32_t result = 0; //--------------------------------------store the result
    int32_t sign = 1;   //--------------------------------------init sign is positive
    uint32_t scnt = 10U;

    if((*pstr) == '-')  //--------------------------------------write sign for negative number
    {
        sign = -1;
        ++pstr;
    }

    if(str_size != 0U)
    {
        scnt = str_size;
    }
      
    while(IS09(pstr) && (scnt != 0U)) //------------------------check is string char valid decimal number
    {
        result = ((result * 10U) + (*pstr - '0'));
        ++pstr;
        --scnt;
    }

    return (sign * result); //----------------------------------get signed value
}


uint8_t Int2Str(char *pstr, int32_t val, uint8_t str_size)
{
    char tmp[12];
    uint8_t strcnt = str_size;
    uint32_t i, t = 0U, s = 0U; //------------------------------init sign is positive
    uint32_t div = 1000000000U;

    
    if(val < 0) //----------------------------------------------check is value negative
    {
        val = -val; //------------------------------------------make value positive
        s = 1U; //----------------------------------------------record sign
    }

  
    for (i = 0U; i < 10U; i++)
    {
        tmp[i] = ((val / div) + '0');
        val = val % div;
        div /= 10U;
        
        if(str_size == 0U)  //----------------------------------supress leading zeros conversion
        {
            pstr[t] = tmp[i];
            
            if(t == 0U) //--------------------------------------check is writing enabled
            {
                if(tmp[i] != '0')   //--------------------------wait first non zero value to start writing
                {
                    if(s == 1U) 
                    {
                        pstr[0] = '-';  //----------------------write minus sign
                        pstr[1] = tmp[i];  //-------------------0x00U terminate string
                        t = 1U;    //---------------------------enable writing and select next char position
                    }
                    
                    ++t;
                }
            }
            else    //------------------------------------------select next char position
            {
                ++t;
            }
        }
        strcnt = t & 0xFFU;
    } 
    
    
    if(str_size != 0)   //--------------------------------------if sized result requested 
    {
        if(s == 1U) 
        {
            pstr[0] = '-';  //----------------------------------write minus sign
            pstr[str_size + 1U] = NUL;   //---------------------0x00U terminate string
        }
        else pstr[str_size] = NUL;   //-------------------------0x00U terminate string
        
        while(str_size && i)    //------------------------------resulted string limited to 10 digit number or requested size
        {
            --i;
            --str_size;
            pstr[str_size] = tmp[i];    //----------------------copy required size from result converted in local bufer 
        }
    }
    else pstr[t] = NUL;     //----------------------------------0x00U terminate string
    return (strcnt);
}


void Str2Hex(const char *pstr, uint8_t *phex, uint16_t str_size)
{
    uint32_t scnt = 0U;
    
    if ((pstr[0]=='0')&&((pstr[1]=='x')||(pstr[1]=='X'))) //----check if input string is hex in 0x or 0X format 
    {
        pstr += 2U;
    }
    
	while(str_size) //------------------------------------------convert requested hex chars from string to integer value
	{
        if(!ISVALIDHEX(pstr)) return;
       
        if(scnt == 0U) 
        {
            *phex = CONVERTHEX(pstr);
            ++scnt;
        }
        else
        {
            scnt = 0U;
            *phex <<= 4U;
            *phex += CONVERTHEX(pstr);
            ++phex;
        }
        
        ++pstr;
        --str_size;
	}
}


void Hex2Str(char *pstr, const uint8_t *phex, uint16_t str_size)
{
    uint32_t scnt = 0U;
    
	while(str_size) //------------------------------------------convert requested hex chars from string to integer value
	{
        if(scnt == 0U) 
        {
            uint32_t tmp = ((*phex >> 4) & 0x0FU);  //----------select high nibble
            if(tmp > 0x09U) *pstr = tmp + 0x37U;    //----------convert to upper ascii letter
            else *pstr = tmp + 0x30U;   //----------------------convert to char digit
            ++scnt;
        }
        else
        {
            uint32_t tmp = (*phex & 0x0FU); //------------------select low nibble
            if(tmp > 0x09U) *pstr = tmp + 0x37U;    //----------convert to upper ascii letter
            else *pstr = tmp + 0x30U;   //----------------------convert to char digit
            scnt = 0U;
            ++phex;
        }
        
        ++pstr;
        *pstr = 0x00U; //----------------------------------------0x00U terminate string
        --str_size;
	}
}
/**
  * @brief  Convert from Binary to 2 digit BCD.
  * @param  Value: Binary value to be converted.
  * @retval Converted word
  */
uint8_t RTC_ByteToBcd2(uint8_t Value)
{
  uint8_t bcdhigh = 0;
  
  while (Value >= 10)
  {
    bcdhigh++;
    Value -= 10;
  }
  
  return  ((uint8_t)(bcdhigh << 4) | Value);
}
/**
  * @brief  Convert from 2 digit BCD to Binary.
  * @param  Value: BCD value to be converted.
  * @retval Converted word
  */
uint8_t RTC_Bcd2ToByte(uint8_t Value)
{
  uint8_t tmp = 0;
  tmp = ((uint8_t)(Value & (uint8_t)0xF0) >> (uint8_t)0x4) * 10;
  return (tmp + (Value & (uint8_t)0x0F));
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

