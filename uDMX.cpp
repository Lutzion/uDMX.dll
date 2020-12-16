// uDMX.cpp : Definiert den Einsprungpunkt fuer die DLL-Anwendung
//
// Copyright (C) 2011 Lutz (ilLU[TZ]minator)
// 

#include "stdafx.h"

//hashlib++ includes
#include <hashlibpp.h>	
//misc. includes
#include <iostream>
#include <string.h>


#include "resource.h"

#include <process.h>
#include <Commctrl.h>  // TBM_GETPOS


#include "uDMX.h"
#include "uDMX_fkt.h"



// globals
const char cCryptKey[]  = {(char)0x3F, 
                           (char)0x89, 
													 (char)0xF0, 
													 (char)0x14, 
													 (char)0x00} ;

int   iProcCnt    = 0 ;
int   iThreadCnt  = 0 ;
int   iDevCnt     = 0 ;
DWORD dwBytesMax  = 1 ;
int   iCracked	  = 0 ;
int   iConfigured = 0 ;
bool  bExtern     = false ;

HANDLE  hDLL      = NULL ;
HWND	hDlgInfo		= NULL ;
HWND	hDlgTest		= NULL ;

HANDLE pThread		= NULL ;
UIParams Par ;

const char cDevDomain[] = CONFIG_DEVDOM ;
const char cDevDevice[] = CONFIG_DEVDEV ;
const char cRegistry[]  = CONFIG_REGKEY ;
const char cDlgSFirm[]  = DLG_SETUPFIRM ;
const char cDlgCopyR[]  = DLG_INFOCOPYR ;


// DLL Main function
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  dwReason , 
                       LPVOID lpReserved )
{
	//HANDLE hEvent ;

  switch (dwReason)
  {
  case (DLL_PROCESS_ATTACH):
    iProcCnt++ ;

		/*
		hEvent = CreateEvent(NULL, TRUE, FALSE, "ilLUTZmination_uDMX_dll_Event");
		if (GetLastError() == ERROR_ALREADY_EXISTS)
			hEvent = NULL ;
		*/

    if (iProcCnt && 
        !pThread /*&&
				hEvent*/)
    {
      // when attached for the first time ->
      // open the device
      hDLL    = hModule ;

      Par.bRestart  = false ;
      Par.bRunning  = true ;
			Par.Connected = false ;
			Par.ConnPrev	= true ;
      memset(&Par.DMX, 0, sizeof(Par.DMX)) ;
      Par.DmxLen    = 0 ;

      // read configuration
      ConfigRead((DWORD *) &Par.iDMXChans, 
                 (DWORD *) &Par.iUDMXBs,
                   (int *) &Par.iUDMXTb,
                   (int *) &Par.iUDMXTm,
                   (int *) &Par.iUDMXTg) ;
      Par.DmxLen  = Par.iDMXChans ;

			if (!iCracked)
				pThread = (HANDLE) _beginthread(uDMXthread, 0, (void *)&Par) ;
    }

    break ;

  case (DLL_THREAD_ATTACH):
    iThreadCnt++ ;
    break ;

  case (DLL_THREAD_DETACH):
    iThreadCnt-- ;
    break ;

  case (DLL_PROCESS_DETACH):
    iProcCnt-- ;
    if (iProcCnt < 0)
      iProcCnt = 0 ;

    if (!iProcCnt)
    {
      // when detached the last time ->
      // close device handle
      Par.bRunning = false ;
			Par.Connected= false ;

      //if (hDlgHWErr)
      //  EndDialog(hDlgHWErr, true) ;
    }

    break ;
  }

  return TRUE ;
}



/*
// close the device
bool uDMXClose(usb_dev_handle * hUSB)
{
  if (hUSB)
    usb_close(hUSB) ;

  return true ;
}
*/

// set ONE value (_stdcall for use with VisualBasic)
bool __stdcall ChannelSet(long Channel, long Value)
{
  return (ChannelsSet(1, Channel, &Value)) ;
}

// set value(s) for the given channel(s)
bool __stdcall ChannelsSet(long ChannelCnt, long Channel, long* Value) 
{
  DWORD iRest, iToSend, iChan ;
  bool  bRes = true ;
  char *cVal = (char *) Value ;

  if (!iConfigured)
    // read configuration
    ConfigRead((DWORD *) &Par.iDMXChans, 
               (DWORD *) &Par.iUDMXBs,
                 (int *) &Par.iUDMXTb,
                 (int *) &Par.iUDMXTm,
                 (int *) &Par.iUDMXTg) ;

  // "Normale" DMX Numerierung anpassen
  Channel-- ;
  
  // how many channels to set
  iRest   = ChannelCnt - 1 ;
  // how many channels to set
  iToSend = min(512, iRest + Channel) ;

	if (iCracked)
		return true ;

  for (iChan = Channel; iChan <= iToSend; iChan++)
  {
    Par.DMX[iChan] = cVal[iChan - Channel] ;
  }

  return bRes ;
}

bool __stdcall Connected()
{
  return Par.Connected ;
}

void __stdcall ConfigureModal(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	DialogBox((HINSTANCE) hDLL,      
                      MAKEINTRESOURCE(IDD_CONFIG),
                      (HWND)NULL ,
                      (DLGPROC)ConfigDlgProc) ;
}


// Configuration Dialog
bool __stdcall Configure()
{
  HWND  hDlg ;
  hDlg = CreateDialog((HINSTANCE) hDLL,      
                      MAKEINTRESOURCE(IDD_CONFIG),
                      (HWND)NULL ,
                      (DLGPROC)ConfigDlgProc) ;

  return true ;
}

INT_PTR CALLBACK  ConfigDlgProc(  HWND hwndDlg,
                                  UINT uMsg,
                                  WPARAM wParam,
                                  LPARAM lParam)
{
  DWORD dwCnt = 512, dwBytes = 512 ;
  int   iTimBreak = 88, iTimMark = 8, iTimGap = 0 ;
  BOOL  bRes ;
	HICON hIcon ;
	HWND	hWnd ;

  switch (uMsg)
  {
  case WM_INITDIALOG:
    ConfigRead(&dwCnt, &dwBytes, &iTimBreak, &iTimMark, &iTimGap) ;
		::SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 
			            (LPARAM) LoadIcon((HINSTANCE)hDLL, MAKEINTRESOURCE(IDI_uDMX)));

    ::SetDlgItemText(hwndDlg, IDC_FIRMWARE, cDlgSFirm) ;

    // Show information
    ::SetDlgItemInt(hwndDlg, IDC_CHANCNT,  dwCnt,   0) ;  
    ::SetDlgItemInt(hwndDlg, IDC_BYTESMAX, dwBytes, 0) ;  
    ::SetDlgItemInt(hwndDlg, IDC_TIMBREAK, iTimBreak, 0) ;  
    ::SetDlgItemInt(hwndDlg, IDC_TIMMARK,  iTimMark, 0) ;  
    ::SetDlgItemInt(hwndDlg, IDC_TIMGAP,   iTimGap, 0) ;  

		::SetTimer(hwndDlg, 1, 500, NULL) ;

    return TRUE ;

  case WM_COMMAND: 
    switch(LOWORD(wParam))
    {
    case IDOK:
			// End Dialog + save
      dwCnt = ::GetDlgItemInt(hwndDlg, IDC_CHANCNT, &bRes, false) ;
      dwCnt = max(1, min(512, dwCnt)) ;


      dwBytes = ::GetDlgItemInt(hwndDlg, IDC_BYTESMAX, &bRes, false) ;
      dwBytes = max(1, min(512, dwBytes)) ;

      iTimBreak = ::GetDlgItemInt(hwndDlg, IDC_TIMBREAK, &bRes, false) ;
      iTimBreak = max(88, min(170, iTimBreak)) ;

      iTimMark  = ::GetDlgItemInt(hwndDlg, IDC_TIMMARK, &bRes, false) ;
      iTimMark  = max(8, min(170, iTimMark)) ;

      iTimGap = ::GetDlgItemInt(hwndDlg, IDC_TIMGAP, &bRes, false) ;
      iTimGap = max(0, min(170, iTimGap)) ;

      ConfigWrite(dwCnt, dwBytes, iTimBreak, iTimMark, iTimGap) ;

      Par.iUDMXTb = iTimBreak ;
      Par.iUDMXTm = iTimMark ;
      Par.iUDMXTg = iTimGap ;

			::KillTimer(hwndDlg, 1) ;
			EndDialog (hwndDlg, TRUE) ;
			return TRUE ;

    case IDCANCEL:
			// End Dialog
			::KillTimer(hwndDlg, 1) ;
			EndDialog (hwndDlg, TRUE) ;
			return TRUE ;

    case IDINFO:
      Info() ;
      break ;

    case IDTEST:
      Test() ;
      break ;

    } // switch(LOWORD(wParam))
		break ;

	case WM_TIMER:
		if (Par.ConnPrev != Par.Connected)
		{
			Par.ConnPrev = Par.Connected ;
			hIcon = LoadIcon((HINSTANCE)hDLL, //GetModuleHandle(NULL), 
											(Par.Connected ? MAKEINTRESOURCE(IDI_uDMX) : 
											 MAKEINTRESOURCE(IDI_ERR))) ;
			//hDlgIco = GetDlgItem(hwndDlg, IDC_DLGCFGICO) ;
			::SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM) hIcon); 
			hWnd = GetDlgItem(hwndDlg, IDC_HWERROR) ;
			ShowWindow(hWnd, (Par.Connected ? SW_HIDE : SW_SHOW)) ;
		}
		break ;

  default:
    break ;
  } // switch (uMsg)

  return FALSE ;
}

// Info Dialog
bool __stdcall Info()
{
	if (!hDlgInfo)
	{
		hDlgInfo = CreateDialog((HINSTANCE) hDLL,      
														MAKEINTRESOURCE(IDD_INFO),
														(HWND)NULL ,
														(DLGPROC)InfoDlgProc) ;
		::SetDlgItemText(hDlgInfo, IDC_COPYRIGHT, cDlgCopyR) ;
	}
	else
		SetForegroundWindow(hDlgInfo) ;

  return true ;
}


INT_PTR CALLBACK  InfoDlgProc(  HWND hwndDlg,
                                  UINT uMsg,
                                  WPARAM wParam,
                                  LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_INITDIALOG:
		::SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 
			            (LPARAM) LoadIcon((HINSTANCE)hDLL, MAKEINTRESOURCE(IDI_uDMX)));
    return TRUE ;

  case WM_COMMAND: 
    switch(LOWORD(wParam))
    {
    case IDOK:
			// End Dialog
			EndDialog (hwndDlg, TRUE) ;
			hDlgInfo = NULL ;
			return TRUE ;
    }
  default:
    break ;
  }

  return FALSE ;
}

// Test Dialog
bool __stdcall Test()
{
	if (!hDlgTest)
		hDlgTest = CreateDialog((HINSTANCE) hDLL,      
														MAKEINTRESOURCE(IDD_TEST),
														(HWND)NULL ,
														(DLGPROC)TestDlgProc) ;
	else
		SetForegroundWindow(hDlgTest) ;

  return true ;
}


INT_PTR CALLBACK  TestDlgProc( HWND hwndDlg,
                               UINT uMsg,
                               WPARAM wParam,
                               LPARAM lParam)
{
	int i ;
	static unsigned char iChan[6] ;

  switch (uMsg)
  {
	// Initialisierung
  case WM_INITDIALOG:
		::SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 
			            (LPARAM) LoadIcon((HINSTANCE)hDLL, MAKEINTRESOURCE(IDI_uDMX)));

		// Fuer alle Slider ...
		for (i = IDC_SLIDER1; i <= IDC_SLIDER6; i++)
		{
			// Bereich von 0..255 setzen
			SendDlgItemMessage(hwndDlg, i, TBM_SETRANGE, 
												(WPARAM) TRUE,               // redraw flag 
												(LPARAM) MAKELONG(0, 255));  // min. & max. positions
			// Eine Linie alle 16 Stufen
			SendDlgItemMessage(hwndDlg, i, TBM_SETTICFREQ, 
												(WPARAM) 16,								// redraw flag 
												(LPARAM) 0);								// min. & max. positions
			// Die aktuelle Position setzen (Richtungsumkehr Oben 255 / Unten 0)
			SendDlgItemMessage(hwndDlg, i, TBM_SETPOS, 
													true, 255 - iChan[i - IDC_SLIDER1]);
		}

		// Werte auch in Text-Anzeigen
		for (i = IDC_CH1; i <= IDC_CH6; i++)
		{
			SetDlgItemInt(hwndDlg, i,  iChan[i - IDC_CH1],   0) ;
		}

    return TRUE ;

	case WM_HSCROLL:
	case WM_VSCROLL:
		// Es wurde gescrollt
		// Welcher Slider wurde geaendert ?
		i = GetWindowLong((HWND) lParam, GWL_ID);

		// Wert des Sliders auslesen
		iChan[i - IDC_SLIDER1] = 255 - SendDlgItemMessage(hwndDlg, i, TBM_GETPOS, 0, 0); 

		// Wertanzeige des Sliders aktualisieren
		SetDlgItemInt(hwndDlg, i - IDC_SLIDER1 + IDC_CH1, iChan[i - IDC_SLIDER1],   0) ;

		// Werte uebertragen
		ChannelsSet(6, 1, (long *)&iChan[0]) ;
		break ;

  case WM_COMMAND: 
    switch(LOWORD(wParam))
    {
    case IDOK:
    case IDCANCEL:
			// End Dialog
			EndDialog (hwndDlg, TRUE) ;
			hDlgTest = NULL ;
			return TRUE ;
    }
  default:
    break ;
  }

  return FALSE ;
}


// Read configuration
bool ConfigRead(DWORD * dwChannelCnt, 
                DWORD * dwBytesMax, 
                int   * iTimBreak,
                int   * iTimMark,
                int   * iTimGap)
{
  HKEY    hKey ;
  DWORD   dwType, dwValue, dwSize ;
	std::string strReg		= cRegistry ;
	std::string strCrypt	= cCryptKey ;

  // Default
  *dwChannelCnt   = 512 ;
  *dwBytesMax     = 512 ;
  *iTimBreak      = 88 ;
  *iTimMark       = 8 ;
  *iTimGap        = 10 ;

	// CrackCheck
	hashwrapper* hw = new sha512wrapper();
	if ((hw->getHashFromString(strCrypt + strReg).compare(CHECK_CONFIG_KEY) != 0) ||
		  (hw->getHashFromString(strCrypt + cDevDomain).compare(CHECK_CONFIG_DOM) != 0) ||
			(hw->getHashFromString(strCrypt + cDevDevice).compare(CHECK_CONFIG_DEV) != 0))
		iCracked = 1 ;
  if ((hw->getHashFromString(strCrypt + cDlgSFirm).compare(CHECK_DLG_SFIRM) != 0) || 
		  (hw->getHashFromString(cDlgCopyR + strCrypt).compare(CHECK_DLG_COPYR) != 0))
		iCracked = 1 ;
	delete hw ;

  // open registry
	if (RegOpenKeyEx(HKEY_CURRENT_USER, 
                   cRegistry, 
                   0,
				           KEY_QUERY_VALUE, 
                   &hKey) != ERROR_SUCCESS)
    return false ;

	if (RegQueryValueEx(hKey, 
                      CONFIG_CHANCNT, 
                      0, 
							        &dwType, 
                      (LPBYTE) &dwValue,
							        &dwSize) == ERROR_SUCCESS)
  {
    *dwChannelCnt  = dwValue ;
  }

	if (RegQueryValueEx(hKey, 
                      CONFIG_BYTESMAX, 
                      0, 
							        &dwType, 
                      (LPBYTE) &dwValue,
							        &dwSize) == ERROR_SUCCESS)
  {
    *dwBytesMax  = dwValue ;
  }

	if (RegQueryValueEx(hKey, 
                      CONFIG_TIMEBREAK, 
                      0, 
							        &dwType, 
                      (LPBYTE) &dwValue,
							        &dwSize) == ERROR_SUCCESS)
  {
    *iTimBreak  = dwValue ;
  }
	if (RegQueryValueEx(hKey, 
                      CONFIG_TIMEMARK, 
                      0, 
							        &dwType, 
                      (LPBYTE) &dwValue,
							        &dwSize) == ERROR_SUCCESS)
  {
    *iTimMark  = dwValue ;
  }
	if (RegQueryValueEx(hKey, 
                      CONFIG_TIMEGAP, 
                      0, 
							        &dwType, 
                      (LPBYTE) &dwValue,
							        &dwSize) == ERROR_SUCCESS)
  {
    *iTimGap  = dwValue ;
  }

  RegCloseKey(hKey) ;

	iConfigured = true ;

  return true ;
}

bool ConfigWrite(DWORD ChannelCnt, DWORD BytesMax, 
                 int iTimBreak, int iTimMark, int iTimGap)
{
  HKEY    hKey ;
  DWORD   dwCreated ;

  // open (or create) registry
	if (RegCreateKeyEx(HKEY_CURRENT_USER, 
                     cRegistry, 
                     0, 
                     "REG_SZ", 
                     0, 
                     KEY_ALL_ACCESS, 
                     0, 
                     &hKey, 
                     &dwCreated) != ERROR_SUCCESS)
    return false ;

	if (RegSetValueEx(hKey, 
                    CONFIG_CHANCNT, 
                    0, 
							      REG_DWORD,
                    (LPBYTE) &ChannelCnt,
							      sizeof(DWORD)) != ERROR_SUCCESS)
    return false ;

	if (RegSetValueEx(hKey, 
                    CONFIG_BYTESMAX, 
                    0, 
							      REG_DWORD,
                    (LPBYTE) &BytesMax,
							      sizeof(DWORD)) != ERROR_SUCCESS)
    return false ;

	if (RegSetValueEx(hKey, 
                    CONFIG_TIMEBREAK, 
                    0, 
							      REG_DWORD,
                    (LPBYTE) &iTimBreak,
							      sizeof(DWORD)) != ERROR_SUCCESS)
    return false ;
	
  if (RegSetValueEx(hKey, 
                    CONFIG_TIMEMARK, 
                    0, 
							      REG_DWORD,
                    (LPBYTE) &iTimMark,
							      sizeof(DWORD)) != ERROR_SUCCESS)
    return false ;

  if (RegSetValueEx(hKey, 
                    CONFIG_TIMEGAP, 
                    0, 
							      REG_DWORD,
                    (LPBYTE) &iTimGap,
							      sizeof(DWORD)) != ERROR_SUCCESS)
    return false ;


  RegCloseKey(hKey) ;

  return true ;
}
