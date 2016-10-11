//////////////////////////////////////////////////////////////////////////////
//
// 	Filename:	Command.c
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
//-----------------------------------------------------------------------------
//	Required files:
//
//-----------------------------------------------------------------------------
//	Notes:
//
//
//-----------------------------------------------------------------------------
//	Revision History:
//
///////////////////////////////////////////////////////////////////////////////
#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include "Common.h"
#include "Board.h"
#include "Command.h"
#include	"libdso138.h"
#include	"Screen.h"
#include 	"Eeprom.h"

// ===========================================================
// 	File Scope Global variables
// ===========================================================
//
const 	KeyScanCode KScanCodeTab[9] = {
//	scan code	key code		key param
	{0x7FFF, 	KC_SW1,		'1'},		// 1
	{0xBFFF, 	KC_SW2,		'2'},		// 2
	{0xDFFF, 	KC_SW3,		'3'},		// 3
	{0xEFFF, 	KC_SW4,		'4'},		// 4
	{0x7FFE, 	KC_SW1H,	'5'},		// 5
	{0xBFFE, 	KC_SW2H,	'6'},		// 6
	{0xDFFE, 	KC_SW3H,	'7'},		// 7
	{0xEFFE, 	KC_SW4H,	'8'},		// 8
	{0, 			0,			0}		
	};

KEYPAD	Keypad;
U8	VSenPrev, CplPrev;
U16	Flags;


// ===========================================================
//	Function Definitions
// ===========================================================
//
void	AppInit()
{
 U16 tmp0;

// =============================
// Note: DSO_Init() must be executed for proper capture engine running
 
 DSO_Init();

// =============================

 // Check EEPROM for valid settings
 EE_ReadVariable(Addr_SettingStatus, &tmp0);
 if(tmp0 == SettingStatus_Initialized) {
	// Load saved settings
	EE_ReadVariable(VirtAddVarTab[Addr_Vpos], &tmp0);
	SetVPos(tmp0);
	EE_ReadVariable(VirtAddVarTab[Addr_Vsen], &tmp0);
	SetVSen(tmp0);
	EE_ReadVariable(VirtAddVarTab[Addr_Cpl], &tmp0);
	SetCpl(tmp0);
	EE_ReadVariable(VirtAddVarTab[Addr_TimeBase], &tmp0);
	SetTimeBase(tmp0);
	EE_ReadVariable(VirtAddVarTab[Addr_TrigMode], &tmp0);
	SetTrigMode(tmp0);
	EE_ReadVariable(VirtAddVarTab[Addr_TrigEdge], &tmp0);
	SetTrigEdge(tmp0);
	EE_ReadVariable(VirtAddVarTab[Addr_TrigLvl], &tmp0);
	SetTrigLvl(tmp0);
	EE_ReadVariable(VirtAddVarTab[Addr_RecLen], &tmp0);
	SetRecLen(tmp0);
	EE_ReadVariable(VirtAddVarTab[Addr_HPos], &tmp0);
	SetHPos(tmp0);
	EE_ReadVariable(VirtAddVarTab[Addr_VPosOfs], &tmp0);
	SetVPosOfs(tmp0);
 	}
 else {
	// Load default settings and initialize EEPROM
 	SetVPos(0);
	EE_WriteVariable(VirtAddVarTab[Addr_Vpos], 0);
 	SetVSen(VS_05V);
	EE_WriteVariable(VirtAddVarTab[Addr_Vsen], VS_05V);
 	SetCpl(CP_DC);
	EE_WriteVariable(VirtAddVarTab[Addr_Cpl], CP_DC);
 	SetTimeBase(TB_1ms);
	EE_WriteVariable(VirtAddVarTab[Addr_TimeBase], TB_1ms);
 	SetTrigMode(TM_Auto);
	EE_WriteVariable(VirtAddVarTab[Addr_TrigMode], TM_Auto);
 	SetTrigEdge(TE_Falling);
	EE_WriteVariable(VirtAddVarTab[Addr_TrigEdge], TE_Falling);
 	SetTrigLvl(0);
	EE_WriteVariable(VirtAddVarTab[Addr_TrigLvl], 0);
 	SetRecLen(SampleBufSizeMax);
	EE_WriteVariable(VirtAddVarTab[Addr_RecLen], SampleBufSizeMax);
 	SetHPos(GetRecLen()/2 - WDsize/2);
	EE_WriteVariable(VirtAddVarTab[Addr_HPos], GetRecLen()/2 - WDsize/2);
	SetVPosOfs(0);
	EE_WriteVariable(VirtAddVarTab[Addr_VPosOfs], 0);

	// Mark down EEPROM has been initialized
	EE_WriteVariable(VirtAddVarTab[Addr_SettingStatus], SettingStatus_Initialized);

 	}

 OutputTLvl();

 // Misc initialization
 TimerKeyScan = 1;

 Keypad.KDebounceVal = KD_val;
 
}

void	KeyProc(void)
{
 switch(Keypad.KeyCode) {
	case KC_SW1:
		DoKeyOk();
	default:	
		break;
		
	case KC_SW2:
		DoKeyInc();
		break;

	case KC_SW3:
		DoKeyDec();
		break;

	case KC_SW4:
		DoKeySel();
		break;

	case KC_SW1H:
		DoKeyOkH();
		break;

	case KC_SW2H:
		DoKeyIncH();
		break;

	case KC_SW3H:
		DoKeyDecH();
		break;

	case KC_SW4H:
		DoKeySelH();
		break;

 	}
}


void KeyScan(void)
{
 U16	tmp1;
 U8	tmp2, tmp3;

 Keypad.KScanBuf = NoKey;
 // Read buttons
 tmp1 = (PB_Port & PB_Bits) | ~PB_Bits;
 if(tmp1 != NoKey) {
	Keypad.KScanBuf = tmp1;
 	}

 
 // -- Debouncing
 if((Keypad.KScanBuf == NoKey) || (Keypad.KScanBuf != Keypad.KScanCode)) {
 	Keypad.KScanCode = Keypad.KScanBuf;
	Keypad.KCount = 0;
	Keypad.KHCount = 0;
	Keypad.KTimeChk = KH_val;
 	}
 else {
	Keypad.KCount++;
	if(Keypad.KCount > Keypad.KDebounceVal) {
		if(Keypad.KCount == Keypad.KDebounceVal + 3) {
			Keypad.KCount = Keypad.KDebounceVal;
			if(++Keypad.KHCount == Keypad.KTimeChk) {
				// Key hold detected
				Keypad.KScanCode &= 0xFFFE;
				KeyConvert((KeyScanCode *)KScanCodeTab, Keypad.KScanCode);
				// Change KTimeChk for key repeat
				Keypad.KTimeChk += KR_Time;
				}
			}
		}
	else if(Keypad.KCount == Keypad.KDebounceVal) {
		// Key push detected
		KeyConvert((KeyScanCode *)KScanCodeTab, Keypad.KScanCode);
		}
	}

#define	Threshold_High		0x0900
#define	Threshold_Low		0x0300

 // Read switch SEN1
 tmp1 = ADC_Poll(ADC2, 2);
 tmp2 = 0;
 if(tmp1 > Threshold_High) {
 	tmp2 = 2;
 	}
 else if(tmp1 > Threshold_Low) {
 	tmp2 = 1;
 	}

 // Read switch SEN2
 tmp1 = ADC_Poll(ADC2, 1);
 tmp3 = 0;
 if(tmp1 > Threshold_High) {
 	tmp3 = 2;
 	}
 else if(tmp1 > Threshold_Low) {
 	tmp3 = 1;
 	}

 // Determine VSen setting
 tmp2 = 3 * tmp2 + tmp3 + VSenMin;
 if(tmp2 != VSenPrev) {
 	SetVSen(tmp2);
	VSenPrev = tmp2;
 	UpdateDisp(Disp_Param);
 	}
 
 // Read switch Cpl
 tmp1 = ADC_Poll(ADC2, 3);
 tmp2 = 0;
 if(tmp1 > Threshold_High) {
 	tmp2 = 2;
 	}
 else if(tmp1 > Threshold_Low) {
 	tmp2 = 1;
 	}
 tmp2 = 2 - tmp2;
 
 // Determine Cpl setting
 if(tmp2 != CplPrev) {
	SetCpl(tmp2);
	CplPrev = tmp2;
 	UpdateDisp(Disp_Param);
 	}
 }

void	KeyConvert(KeyScanCode *KSCTab, U16 KSCode)
{
 U16 tmp1;
 
 while((tmp1 = *(U16 *)(KSCTab + 0))) {
	if(tmp1 == KSCode) {
		// -- Match found
		Keypad.KeyCode = *(U8 *)((U8 *)KSCTab + 2);
		Keypad.KeyCodeBuf = Keypad.KeyCode;
		Keypad.KeyParam = *(U8 *)((U8 *)KSCTab + 3);
		return;
		}
	else {
		// -- Proceed to next entry
		KSCTab = (KeyScanCode *)((U8 *)KSCTab + sizeof(KeyScanCode));
		}
	
 	}
}

void	DoKeyOk(void)
{
 U16 tmp;
 
 tmp = GetDsoStatus();
 // Toggle HOLD state
 BitXor(tmp, DSO_Hold);

 if(BitTest(tmp, DSO_Hold)) {
 	// Set HOLD 
 	SetHold();
 	// Stop capture
 	StopCapture();
 	}
 else {
 	// Clear HOLD
 	ClrHold();
 	// Start capture at exit of HOLD
	StartCapture();
 	}
 
 UpdateDisp(Disp_Param);
}

void	DoKeyInc(void)
{
 S8	tmp0;
 S16	tmp1;
 
 switch(GetFocus()) {
	case FC_Timebase:
 		tmp0 = GetTimebase();
 		tmp0++;
 		tmp0 = SetTimeBase(tmp0);
		EE_WriteVariable(VirtAddVarTab[Addr_TimeBase], tmp0);
		if(tmp0 >= TB_20ms) {
			// Restart capture
			StartCapture();
			}
		else {
			// Change sampling rate only
			UpdateTimebase();
			}
		// Make key debounce time shorter for these TB's 
		if((tmp0 < TB_20ms) && (tmp0 > TB_1s)) {
			Keypad.KDebounceVal = KD_val1;
			}
		else {
			Keypad.KDebounceVal = KD_val;
			}
	default:	
		break;
		
	case FC_TrigMode:
 		tmp0 = GetTrigMode();
 		tmp0++;
 		tmp0 = SetTrigMode(tmp0);
		EE_WriteVariable(VirtAddVarTab[Addr_TrigMode], tmp0);
		// Restart capture. 
		StartCapture();
		break;

	case FC_TrigEdge:
 		tmp0 = GetTrigEdge();
 		tmp0++;
 		tmp0 = SetTrigEdge(tmp0);
		EE_WriteVariable(VirtAddVarTab[Addr_TrigEdge], tmp0);
		break;
		
	case FC_VPos:
 		tmp1 = GetVPos();
 		tmp1++;
 		tmp1 = SetVPos(tmp1);
		EE_WriteVariable(VirtAddVarTab[Addr_Vpos], tmp1);
 		UpdateDisp(Disp_Trace);
		break;

	case FC_TrigLvl:
 		tmp1 = GetTrigLvl();
 		tmp1++;
 		tmp1 = SetTrigLvl(tmp1);
		EE_WriteVariable(VirtAddVarTab[Addr_TrigLvl], tmp1);
		OutputTLvl();
		break;
		
	case FC_HPos:
		// Move waveform right
 		tmp1 = GetHPos();
 		tmp1--;
 		tmp1 = SetHPos(tmp1);
		EE_WriteVariable(VirtAddVarTab[Addr_HPos], tmp1);
 		UpdateDisp(Disp_Trace);
		break;
		
 	}
 
 UpdateDisp(Disp_Param);
}

void	DoKeyDec(void)
{
 S8	tmp0;
 S16	tmp1;
 
 switch(GetFocus()) {
	case FC_Timebase:
 		tmp0 = GetTimebase();
 		tmp0--;
 		tmp0 = SetTimeBase(tmp0);
		EE_WriteVariable(VirtAddVarTab[Addr_TimeBase], tmp0);
		if(tmp0 >= TB_50ms) {
			// Restart capture
//			UartPutc('7', USART1);
			StartCapture();
//			UartPutc('1', USART1);
			}
		else {
			// Change sampling rate only
			UpdateTimebase();
			
			}
/*		
		// Make key debounce time shorter for these TB's 
		if((tmp0 < TB_20ms) && (tmp0 > TB_1s)) {
			Delay(50000);	// To avoid same keypress repeated
			Keypad.KDebounceVal = KD_val1;
			}
		else {
			Keypad.KDebounceVal = KD_val;
			}
*/		
	default:	
		break;
		
	case FC_TrigMode:
 		tmp0 = GetTrigMode();
 		tmp0--;
 		tmp0 = SetTrigMode(tmp0);
		EE_WriteVariable(VirtAddVarTab[Addr_TrigMode], tmp0);
		// Restart capture. 
		StartCapture();
		break;

	case FC_TrigEdge:
 		tmp0 = GetTrigEdge();
 		tmp0--;
 		tmp0 = SetTrigEdge(tmp0);
		EE_WriteVariable(VirtAddVarTab[Addr_TrigEdge], tmp0);
		break;
		
	case FC_VPos:
 		tmp1 = GetVPos();
 		tmp1--;
 		tmp1 = SetVPos(tmp1);
		EE_WriteVariable(VirtAddVarTab[Addr_Vpos], tmp1);
 		UpdateDisp(Disp_Trace);
		break;

	case FC_TrigLvl:
 		tmp1 = GetTrigLvl();
 		tmp1--;
 		tmp1 = SetTrigLvl(tmp1);
		EE_WriteVariable(VirtAddVarTab[Addr_TrigLvl], tmp1);
		OutputTLvl();
		break;
		
	case FC_HPos:
		// Move waveform left
 		tmp1 = GetHPos();
 		tmp1++;
 		tmp1 = SetHPos(tmp1);
		EE_WriteVariable(VirtAddVarTab[Addr_HPos], tmp1);
 		UpdateDisp(Disp_Trace);
		break;
		
 	}
 
 UpdateDisp(Disp_Param);
}

void	DoKeySel(void)
{
 U8 tmp;

 tmp = GetFocus();
 tmp++;
 if(tmp >= FC_Max) {
 	tmp = 0;
 	}
 SetFocus(tmp);

 UpdateDisp(Disp_Param);
}

void	DoKeyOkH(void)
{
 S16 tmp1;
 
 if(GetFocus() == FC_VPos) {
	// Do VPos alignment
	tmp1 = (S16)(GetAverage() - WWindowMidValue);
	SetVPosOfs(tmp1);
	EE_WriteVariable(VirtAddVarTab[Addr_VPosOfs], tmp1);
 	}
}

void	DoKeyIncH(void)
{
 S16	tmp1;
 
 switch(GetFocus()) {
	case FC_VPos:
 		tmp1 = GetVPos();
 		tmp1 += 10;
 		SetVPos(tmp1);
		break;

	case FC_TrigLvl:
 		tmp1 = GetTrigLvl();
 		tmp1 += 10;
 		SetTrigLvl(tmp1);
		OutputTLvl();
		break;
		
	case FC_HPos:
		// Move waveform right
 		tmp1 = GetHPos();
 		tmp1 -= 20;
 		SetHPos(tmp1);		
		break;
 	}

 UpdateDisp(Disp_Param);
}

void	DoKeyDecH(void)
{
 S16	tmp1;
 
 switch(GetFocus()) {
	case FC_VPos:
 		tmp1 = GetVPos();
 		tmp1 -= 10;
 		SetVPos(tmp1);
		break;

	case FC_TrigLvl:
 		tmp1 = GetTrigLvl();
 		tmp1 -= 10;
 		SetTrigLvl(tmp1);
		OutputTLvl();
		break;
		
	case FC_HPos:
		// Move waveform left
 		tmp1 = GetHPos();
 		tmp1 += 20;
 		SetHPos(tmp1);
		break;
 	}

 UpdateDisp(Disp_Param);
}

void	DoKeySelH(void)
{

}

void	LedBlink(void)
{
 U16	tmp;
 
 // Turn on LED
 Port_BitClr(LED_Base, (1 << LED_Bit));
 tmp = 20;
 while(tmp) {
 	Delay(65000);
 	tmp--;
 	}
 
 // Turn off LED
 Port_BitSet(LED_Base, (1 << LED_Bit));
 tmp = 20;
 while(tmp) {
 	Delay(65000);
 	tmp--;
 	}

 // Turn on LED
 Port_BitClr(LED_Base, (1 << LED_Bit));
 tmp = 20;
 while(tmp) {
 	Delay(65000);
 	tmp--;
 	}
 
 // Turn off LED
 Port_BitSet(LED_Base, (1 << LED_Bit));
}

void	TestMode(void)
{
 U16 tmp;
 
 // Change system clock to HSI
 RCC->CFGR &= ~RCC_CFGR_SW;
 RCC->CFGR |= RCC_CFGR_SW_HSI;

 // Disable JTAG and SWD
 AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG;
 AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_2;

 // Remap PD0 & PD1 to make them available as GPIO
 AFIO->MAPR |= AFIO_MAPR_PD01_REMAP;

 // Set all ports to output mode
 GPIOA->CRL = 0x33333333;
 GPIOA->CRH = 0x33333333;

 GPIOB->CRL = 0x33333333;
 GPIOB->CRH = 0x33333333;

 GPIOC->CRL = 0x33333333;
 GPIOC->CRH = 0x33333333;
 
 GPIOD->CRL = 0x33333333;
 GPIOD->CRH = 0x33333333;

 // Blink LEDs
 while(1) {
	GPIOA->ODR = 0x5555;
	GPIOB->ODR = 0x5555;
	GPIOC->ODR = 0x5555;
	GPIOD->ODR = 0x5555;

	tmp = 5;
	while(tmp) {
		Delay(60000);
		tmp--;
		}

	GPIOA->ODR = 0xAAAA;
	GPIOB->ODR = 0xAAAA;
	GPIOC->ODR = 0xAAAA;
	GPIOD->ODR = 0xAAAA;

	tmp = 5;
	while(tmp) {
		Delay(60000);
		tmp--;
		}
 	}
 
}

