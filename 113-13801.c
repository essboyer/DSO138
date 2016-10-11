//////////////////////////////////////////////////////////////////////////////
//
// 	Filename:	113-13801.c
//	Version:		
//	Data:		
//
//	Author:		Liu, Zemin
//	Company:	JYE Tech
//
//-----------------------------------------------------------------------------
//
// 	Target: 		STM32F103C8 
// 	Tool chain: 	CodeSourcery G++
//
//	Descriptions: 	Main firmware for low-cost DSO kit
//	PCB: 		109-13800-00C
//-----------------------------------------------------------------------------
//	Required files:
//
//-----------------------------------------------------------------------------
//	ATTENTION: 
//-----------------------------------------------------------------------------
//	Revision History:
//
//
///////////////////////////////////////////////////////////////////////////////

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include "Common.h"
#include "Board.h"
#include	"Screen.h"
#include	"Command.h"
#include 	"Eeprom.h"

#include	"libdso138.h"


int main (void)
{
 U16 tmp1, tmp2;
 
 Clock_Init();
 
 Port_Init();

 // Check SW4 pin. If the pin is read LOW enter TEST mode
 tmp1 = (PB_Port & PB_Bits) | ~PB_Bits;
 if(!BitTest(tmp1, 0x8000)) {
	// Enter TEST mode
	TestMode();
 	}
 
 /* Unlock the Flash Program Erase controller */
 FLASH_Unlock();

 /* EEPROM Init */
 EE_Init();

 TFT_Init_Ili9341();
 
 USART1_Init();
 uputs((U8 *)"Uart1 Ok\n\r", USART1);

 SysTick_Init();
 TIM3_Init();
 TIM4_Init();

 ADC2_Init();
 
 AppInit();  
 
 tmp1 = clBlack;
 PutsGenic(24, 120, (U8 *)"FW: 113-13801-037", clWhite, tmp1, &ASC8X16);
 PutsGenic(24, 140, (U8 *)LibVersion, clWhite, tmp1, &ASC8X16);
 PutsGenic(24, 180, (U8 *)"Booting...", clWhite, tmp1, &ASC8X16);

 // LED blink twice
 LedBlink();

 NVIC_Configuration();

 Flags = 0;
 
// Uncomment the following line to disable display of scope panel
// NoScopePanel = 1;
 
 while(1) {
// ============================
// Note: 	Keep this section in the main loop for proper capture engine running
//		even scope panel display is disabled.

 	DsoDisplay();

	tmp1 = GetDsoStatus();

// --------------------------------------------------------	
	if(BitTest(tmp1, DSO_NormalDisplay)) {

		// Add your diaplay codes here

		
		}
// --------------------------------------------------------

 	if(GTimeout) {
		GTimeout = 0;
		StartCapture();
 		}
	
// ============================	


	if(BitTest(Keypad.Flags, (1 << KF_DoKeyScan))) {
		BitClr(Keypad.Flags, (1 << KF_DoKeyScan));
		// Do key scan
		KeyScan();
		}

	if(Keypad.KeyCode) {
		// Process key code
		KeyProc();
		Keypad.KeyCode = 0;
		}

	if(GTimer == 0) {
		if(BitTest(tmp1, DSO_CaptureDone)) {
			UpdateDisp(Disp_Trace);
			tmp2 = GetTimebase();
			if(tmp2 <= TB_1ms) {
				// Start next capture		
				StartCapture();
				}
			else {
				// Lower capture rate
				GTimer = 40;
				}
			}
		}

	
	if(BitTest(tmp1, DSO_Rolling)) {
		Rolling();
		}

 	}
 
}

