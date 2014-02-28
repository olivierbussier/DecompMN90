// ****************************************************************************/
// ComCtls.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include <stdio.h>
#include "resource.h"

#define MAX_LOADSTRING     100
#define ID_TIMER WM_USER + 300

//Menu Bar Height
#define MENU_HEIGHT 26

// Global Variables:
//HINSTANCE	hInst;							// The current instance
HWND		hwndCB;							// The command bar handle
HWND		hwndMain;						// The main window
HWND		g_hwndComCtls;					// The basic dialog for the controls
TCHAR		szTitle[MAX_LOADSTRING];		// The title bar text
TCHAR		szWindowClass[MAX_LOADSTRING];	// The window class name

int ElapsedTime=0;


#define TIMER_VALUE  5000
#define BATTERYGRAPHCLASS       TEXT("BatteryGraph")

unsigned char PowerValues[16384];
int indexPower       =0;
int LastSave		 =0;

void GraphUpdate(HWND hwnd);

/********************************************************************/
LRESULT CALLBACK BatteryGraphWndProc(HWND hwnd, UINT msg, WPARAM wParam,LPARAM lParam)
/********************************************************************/
{
  switch (msg) {
    case WM_CREATE: {
	  break;
	}

    case WM_PAINT:
      GraphUpdate(hwnd);
	  break;

	case WM_DESTROY: {
	  break;
	}

	default:
	  return (DefWindowProc(hwnd, msg, wParam, lParam));
  }

  return ((LONG) TRUE);
}

/********************************************************************/
ATOM BatteryGraphRegisterClass (HINSTANCE hInstance)
/********************************************************************/
{
  WNDCLASS wc;

  wc.style = 0;
  wc.lpfnWndProc = (WNDPROC) BatteryGraphWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 4;
  wc.hInstance = (HINSTANCE) hInstance;
  wc.hIcon = NULL;
  wc.hCursor = NULL;
  wc.hbrBackground = NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = BATTERYGRAPHCLASS;

  return RegisterClass(&wc);
}

/********************************************************************/
void GraphUpdate(HWND hwnd)
/********************************************************************/
{
  PAINTSTRUCT    ps;
  RECT   rect;
  HBRUSH hBrush; //,hOldBrush;
  HPEN   hPen,hPenG,hPenR,   // Handle to the new pen object
         hOldPen;            // Handle to the old pen object
  int    index,
	     Scale;
  HGDIOBJ h1,hOld,hOldBrush;

  HDC hdc = BeginPaint(hwnd, &ps);

  GetClientRect(hwnd,&rect);

  hBrush = CreateSolidBrush (0x0ffffff);
  h1 = GetStockObject(BLACK_PEN);
  hOld      = SelectObject (hdc, h1);
  hOldBrush = SelectObject (hdc, hBrush);
  Rectangle(hdc,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top);
  SelectObject (hdc, hOld);
  SelectObject (hdc, hOldBrush);
  DeleteObject (hBrush);


  // Draw X Axes (10% by 10%)
  // ----------------------

  // Create a dash pen object and select it.
  hPen = CreatePen (PS_SOLID, 1, 0x000000);
  hOldPen = (HPEN)SelectObject (hdc, hPen);

  for (index=1;index<10;index++) {
    MoveToEx (hdc,rect.left,(rect.bottom*index)/10,NULL);
	LineTo (hdc,rect.right-1,(rect.bottom*index)/10);
  }

  // Draw Y Axes (Variable scale)
  // ----------------------------

  // Determine scale

  Scale=12;
  indexPower=60;

  for (index = Scale; index < indexPower; index+=Scale) {
    MoveToEx (hdc,rect.right*index/indexPower,rect.top,NULL);
    LineTo   (hdc,rect.right*index/indexPower,rect.bottom);
  }

  // Select the old pen back into the device context.
  SelectObject (hdc, hOldPen);
  DeleteObject (hPen);

  // Create a solid pen object and select it.
  hPenR = CreatePen (PS_SOLID, 2, 0x000000FF); // Red
  hPenG = CreatePen (PS_SOLID, 2, 0x0000FF00); // Green
  hOldPen = (HPEN)SelectObject (hdc, hPenR);

  MoveToEx (hdc,0,0,NULL);
  LineTo   (hdc,rect.left+10,rect.bottom - 20);
  LineTo   (hdc,rect.right-50,rect.bottom - 20);
  LineTo   (hdc,rect.right-40,rect.top + 40);

/*
  // We set PrevPow with discharging condition, unreachable value
  // This implies PrevPow always different from PowerNov

  unsigned char PowerNow,fCharging,PrevPow=0x7f;
  if (indexPower>0) {
	for (index = 0; index < indexPower; index++) {
	  PowerNow   = PowerValues[index] & 0x7f;
	  fCharging  = PowerValues[index] & 0x80;
      if (((PrevPow & 0x80)^fCharging)!=0) {
		 // Change : We changed power conditions
		if (fCharging) {
		  // Now power supply plugged
		  // Curve is draw in Green
          SelectObject (hdc, hPenG);
		} else {
		  // Now Power supply unplugged
		  // All data are fired and counters reseted
		  indexPower=LastSave=0;
		  ElapsedTime=0;
		  // We memorized last remaining power for next
		  // Initial drawing coordinates
          PowerValues[indexPower++] = PowerNow;
		  break;
		}
	  }
      if (index==0)
		// If Index == 0, then only set initial drawing coordinates
        MoveToEx   (hdc,index * rect.right / indexPower,(rect.bottom - ((rect.bottom * PowerNow)/100)),NULL);
	  else {
		//
        if (PowerNow<=(unsigned char)100) {
          if (((PrevPow != (PowerNow | fCharging)) || (index == indexPower-1))) {
			// Only do something if :
			//  - Power level or charging mode different than previous iteration
			//  - If this is the last iteration
			LineTo (hdc,index * rect.right / indexPower,(rect.bottom - ((rect.bottom * PowerNow)/100)));
			// Memorize charging level and condition for next iteration
			PrevPow = PowerNow | fCharging;
		  }
		}
	  }
	}
  }
*/
  // Select the old pen back into the device context.
  SelectObject (hdc, hOldPen);

  // Delete the pen object and free all resources associated with it.
  DeleteObject (hPenR);
  DeleteObject (hPenG);

  // ReleaseDC (hwnd, hdc);

  EndPaint(hwnd, &ps);
  // Release the device context.
}

/********************************************************************/
void SecondsToString(int temp,TCHAR *szBufW)
/********************************************************************/
{
  int Days  = temp / (24*3600);      temp -= Days * (24*3600);
  int Hours = temp / (   3600);      temp -= Hours* (   3600);
  int Min   = temp / (     60);      temp -= Min  * (     60);
  int Sec   = temp;

  sprintf(szBufW, "%dj %.2dh %.2dm %.2ds", Days,Hours,Min,Sec);
}


/********************************************************************/
void DrawControls(HWND hwnd)
/********************************************************************/
{
  HWND hwBG = GetDlgItem(hwnd, IDC_CUSTOM1);
  InvalidateRect (hwBG,NULL,FALSE);
}
