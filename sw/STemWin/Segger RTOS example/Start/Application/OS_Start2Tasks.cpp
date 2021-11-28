/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 1995 - 2016 SEGGER Microcontroller GmbH & Co. KG         *
*                                                                    *
*       Internet: segger.com  Support: support_embos@segger.com      *
*                                                                    *
**********************************************************************
*                                                                    *
*       embOS * Real time operating system for microcontrollers      *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product or a real-time            *
*       operating system for in-house use.                           *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       OS version: Internal                                         *
*                                                                    *
*       Current version number will be inserted here                 *
*       when shipment is built.                                      *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File    : OS_Start2Tasks.cpp
Purpose : embOS C++ sample program running two simple tasks.
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "RTOS.h"
#include "stdlib.h"

/*********************************************************************
*
*       Class definition
*
**********************************************************************
*/
class CThread:private OS_TASK {
public:
  CThread(char* s, unsigned int p, unsigned int t):sName(s), Priority(p), Timeout(t) {
    void* pTaskStack = malloc(256u);
    OS_CreateTaskEx(this, sName, p, CThread::run, pTaskStack, 256u, 2u, reinterpret_cast<void*>(Timeout));
  }

private:
  char*        sName;
  unsigned int Priority;
  unsigned int Timeout;

  static void run(void* t) {
    while (1) {
      OS_Delay(reinterpret_cast<unsigned int>(t));
    }
  }
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
CThread *HPTask, *LPTask;

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
int main(void) {
  OS_IncDI();                      /* Initially disable interrupts  */
  OS_InitKern();                   /* Initialize OS                 */
  OS_InitHW();                     /* Initialize Hardware for OS    */
  /* You need to create at least one task before calling OS_Start() */
  HPTask = new CThread(const_cast<char*>("HPTask"), 100u, 50u);
  LPTask = new CThread(const_cast<char*>("LPTask"), 50u, 200u);
  OS_Start();                      /* Start multitasking            */
  return 0;
}

/****** End Of File *************************************************/
