// uDMX.h

#ifndef __UDMX_H__
#define __UDMX_H__


#define DllExport   __declspec( dllexport )



/* Use obdev's generic shared VID/PID pair and follow the rules outlined
 * in firmware/usbdrv/USBID-License.txt.
 */
#define USBDEV_SHARED_VENDOR    0x16C0  /* VOTI */
#define USBDEV_SHARED_PRODUCT   0x05DC  /* Obdev's free shared PID */


#define cmd_SetChannelRange 2



/* this is libusb, see http://libusb.sourceforge.net/ */
#include "usb.h"

// defines for configuration
#define CONFIG_REGKEY			"SOFTWARE\\ilLUTZminator\\uDMX.dll"
#define CONFIG_CHANCNT		"ChannelCount"
#define CONFIG_BYTESMAX		"BytesMax"
#define CONFIG_TIMEBREAK	"TimeBreak"
#define CONFIG_TIMEMARK		"TimeMark"
#define CONFIG_TIMEGAP		"TimeIBGap"

#define CONFIG_DEVDOM			"www.anyma.ch"
#define CONFIG_DEVDEV			"uDMX"

#define DLG_SETUPFIRM     "Use uDMX firmware >= 1.4 made by ilLU[TZ]minator (www.ilLUTZminator.de) to enable setting of times"
#define DLG_INFOCOPYR     "DLL for uDMX Transmitter\nwritten by ilLU[TZ]minator, Copyright 2010"

// sha512 CHECKS
//#define CHECK_CONFIG_KEY	"e0e27ae42a5f0f1dd904e17f8daa453f288aeae27236b6b2f9be8e36ebac7807d7dc1e0b7409e15b12159ecdb5739306edf8ff36c3678538004efda3b2da56e9"
//#define CHECK_CONFIG_DOM  "1ba65178a1cbfcc321a328cd314b375406de553e721a8bcfad4826d391f0cb2cb098bcd43dbd28ea29cd39aeb1bb1967a0cab9e024ef5d659a190338dd386191"
//#define CHECK_CONFIG_DEV  "28da5e90c5473b15f0f2b0560dad4b108b255c642f91e1f099fe34190e898ed50b08154ac36144d1f82983063fa09a6c77998364a9500968dd8916ddfe1f8f95"
//#define CHECK_DLG_SFIRM		"b5f20c162ef088898b2fc9ce2479c4f17a7df13251b6b257882041b07ab26e1b1b58effa627c6bd74f8feadb4626ef077a734e4f8bee0d8ce74c03df578518eb"
//#define CHECK_DLG_COPYR		"49ee44a56f19bc7547ad2ee64f2f5e9d44808884a3036d4ecdb4b3b042a0910d606154dce191da1c1ee4a2bc14f51afc77ddc18bd0f5201d43d4590444e74cc2"

#define CHECK_CONFIG_KEY	"729424225af86ff97e455259f0550df0dd6a135af9fee49b426beaead22f61fb37cd6d656dc113ceede788635a0f2a96c24f79bf0a062eca910d813f386b2d23"
#define CHECK_CONFIG_DOM  "872193426f920d39ebd21857da8f8fe1f6fc0a144de08146bdafe0f2e415ba45c71f0a4e807decac31daa88709d85753fca7ef15f55089af0a86ca4fce1743de"
#define CHECK_CONFIG_DEV  "af2270cfae2b445c4e22c3e9a8e6294a28539b6683a25ff5617375ce8f1f4174ee06142ef927377d02d1bec7f8831f3c9c294d1bdf6f8bcd2d018c340e3ae690"
#define CHECK_DLG_SFIRM		"665380ed077cac22d68e96a433123ea911965b5d6b6071cfee2847f08dc59980f74cfb42739cd24491bd2d66b38bd73688e0cf7ea2f0062fdefd587fb5d513c1"
#define CHECK_DLG_COPYR		"230d02078649d490eae5194bbfaa634be4a454c558f15641260a1cc366dea771d4b626e72462c69cdc2ff30662e48a759a8f59011ed0cc894745c0c0430e9053"

typedef unsigned char byte ;

typedef struct
{
  // === Data
  int   iVerbose ;
  int   iDMXChans ;
  int   iUDMXBs ;
  int   iUDMXTb ;
  int   iUDMXTm ;
  int   iUDMXTg ; 
  
	byte  DMX[513] ;
  int   DmxLen ;  // used by Thread ! 

	bool  bRunning ;
  bool  bRestart ;
	bool  Connected ;
	bool  ConnPrev ;

} UIParams, *pUIParams ;



// functions
usb_dev_handle  * uDMXOpen() ;
bool              uDMXClose(usb_dev_handle *) ;
bool _stdcall     ChannelsSet(long ChannelCnt, long Channel, 
                             long* Value) ;
bool _stdcall     ChannelSet(long Channel, long Value) ;
bool _stdcall     Configure() ;
bool _stdcall     Info() ;
bool _stdcall     Test() ;
//bool _stdcall     HwError() ;

bool              ConfigRead(DWORD *, DWORD *, int *, int *, int *) ;
bool              ConfigWrite(DWORD, DWORD, int, int, int) ;


INT_PTR CALLBACK  ConfigDlgProc(  HWND hwndDlg,
                                  UINT uMsg,
                                  WPARAM wParam,
                                  LPARAM lParam) ;
INT_PTR CALLBACK  InfoDlgProc  (  HWND hwndDlg,
                                  UINT uMsg,
                                  WPARAM wParam,
                                  LPARAM lParam) ;
INT_PTR CALLBACK  TestDlgProc  (  HWND hwndDlg,
                                  UINT uMsg,
                                  WPARAM wParam,
                                  LPARAM lParam) ;
INT_PTR CALLBACK  HwErrDlgProc  ( HWND hwndDlg,
                                  UINT uMsg,
                                  WPARAM wParam,
                                  LPARAM lParam) ;

#endif 