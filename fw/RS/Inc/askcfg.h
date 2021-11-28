/**
 ******************************************************************************
 * Project          : KuceDzevadova
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

#ifndef _ASK_CONFIG_H
#define _ASK_CONFIG_H
/* Exported Define  ----------------------------------------------------------*/
#define _ASK_TIM                                                        htim2 //  10us tick
#define _ASK_MINIMUM_STAY_IN_LOW_STATE_TO_DETECT_NEW_FRAME_IN_MS        8
#define _ASK_TIMEOUT_TO_DETECT_NEW_FRAME_IN_MS                          50
#define _ASK_HOLD_LAST_FRAME_IN_MS                                      500
#define _ASK_MIN_DATA_BYTE                                              5
#define _ASK_MAX_DATA_BYTE                                              5
#define _ASK_TOLERANCE_IN_PERCENT                                       25
#define _PREAMBLE_BIT_PERIOD                                            800     // 800 us pwm preamble period   ž
#define _PREAMBLE_TO_DATA_DUTY                                          400     // 400 us duty cycle of preamble to data delimiter
#define _PREAMBLE_TO_DATA_DELIMITER                                     4400    // 4,4 ms from preamble to first data bit  
#define _DATA_BIT_PERIOD                                                1200    // 1,2 ms data bit period
#define _PREAMBLE_BITS                                                  11
#define _DATA_BITS                                                      65
#define _PACKET_PAUSE                                                   18000   // 18 ms pase betwen two packets
#define _SHORT_DUTY                                                     400     // 400 us short pwm duty time
#define _LONG_DUTY                                                      800     // 800 us long pwm duty cycle time
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
