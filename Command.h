//////////////////////////////////////////////////////////////////////////////
//
// 	Filename:	Command.h
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
//
//
#ifndef	Command_h
#define	Command_h

#include	"Common.h"

// ============== Key Analysis Definitions =====================

typedef	struct {
	// Keypad processing variables
	U16	Flags;
	U16	KScanBuf;
	U16 	KScanCode;
	U16	KeyCode;
	U16	KeyParam;
	U16	KeyCodeBuf;
	U16	KCount;			// Key debounce counter
	U16	KDebounceVal;	// Debounce timing length (number of scans)
	U16	KHCount;		// Key hold counter
	U16	KTimeChk;		// Key time check 

	} KEYPAD;

enum	KeypadFlags {
	KF_DoKeyScan		= 0,

	};

// -- Key-Event mapping 
typedef	struct	{
	U8	Keycode;
	U8	Event;
	}KeyEvent ;

// -- Key code conversion 
typedef	struct	{
	U16 	ScanCode;
	U8	Keycode;
	}KeyMap ;

// --------------------------------------
// Keypad 
typedef	struct	{
	U16	ScanCode;
	U8	KeyCode;
	U8	KeyParam;
	}KeyScanCode;

// -- Key Code Definitions 
enum	KeyCodes {
	KC_void = 0,				
	KC_SW1,				
	KC_SW2,				
	KC_SW3,				
	KC_SW4,				
	KC_SW1H,				
	KC_SW2H,				
	KC_SW3H,				
	KC_SW4H,		
	};

// Pushbutton processing parameters
#define	NoKey			0xFFFF	

#define	KD_val			2		// 
#define	KD_val1			5		// 
#define	KH_val			20		// 
#define	KR_Time		1	

// Setting status
#define	SettingStatus_Initialized		0xF3C5


// ===========================================================
//	Export variables
// ===========================================================
//
extern	KEYPAD	Keypad;
extern	U16	Flags;

// ===========================================================
//	Function Declarations
// ===========================================================
//
void	AppInit();
void KeyScan(void);
void	KeyConvert(KeyScanCode *KSCTab, U16 KSCode);
U8	KeyEventMap(U8 keycode, KeyEvent *kvmap);
void	KeyProc(void);
void	DoKeyOk(void);
void	DoKeyInc(void);
void	DoKeyDec(void);
void	DoKeySel(void);
void	DoKeyOkH(void);
void	DoKeyIncH(void);
void	DoKeyDecH(void);
void	DoKeySelH(void);
void	LedBlink(void);
void	TestMode(void);


#endif

