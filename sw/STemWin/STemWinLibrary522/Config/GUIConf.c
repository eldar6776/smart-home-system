
#include "GUI.h"
#include "stm32f429i_lcd.h"

#define LCD_GUI_RAM_BASE  (U32)(LCD_FRAME_BUFFER+(1024*1024*2))
#define GUI_BLOCKSIZE 0x80
/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

//
// Define the available number of bytes available for the GUI
//

#define GUI_NUMBYTES  (1024 * 30)    // x KByte

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
#if defined ( __CC_ARM   ) 
  U32 HeapMem[1024 * 2048] __attribute__((at(0xD003FC00)));//0xD0400000
//#elif defined ( __ICCARM__ ) 
//  #pragma location=0xD003FC00//0xD0100000
//  static __no_init U32 HeapMem[1024 * 2048];
//#elif defined   (  __GNUC__  ) 
//  U32 HeapMem[1024 * 2048] __attribute__((section(".HeapMemSection")));
#endif

U32 extMem[GUI_NUMBYTES / 4] __attribute__((at(0xD003FC00)));//0xD0400000

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       Get_ExtMemHeap
*
* Purpose:
*   Allocate heap from external memory
*/
U32* Get_ExtMemHeap (void)
{
  return HeapMem;
}

/*********************************************************************
*
*       GUI_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   available memory for the GUI.
*/
void GUI_X_Config(void)
{
 //GUI_ALLOC_AssignMemory(extMem, GUI_NUMBYTES);//缓存在内部RAM
	// GUI_ALLOC_AssignMemory((void*)LCD_GUI_RAM_BASE, GUI_NUMBYTES);//缓存在外部SDRAM
//	GUI_ALLOC_SetAvBlockSize(GUI_BLOCKSIZE);
	GUI_ALLOC_AssignMemory(extMem, GUI_NUMBYTES);
	GUI_SetDefaultFont(GUI_FONT_6X8);
}

/*************************** End of file ****************************/
