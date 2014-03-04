// ****************************************************************************/
// ComCtls.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include "DecompMN90.h"

#define MAX_LOADSTRING     100
#define ID_TIMER WM_USER + 300

//Menu Bar Height
#define MENU_HEIGHT 26

// Global Variables:
TCHAR		szTitle[MAX_LOADSTRING];		// The title bar text
TCHAR		szWindowClass[MAX_LOADSTRING];	// The window class name
HWND        GraphHwnd=0;

int ElapsedTime=0;

#define BATTERYGRAPHCLASS       TEXT("BatteryGraph")

tGraph ResultGraph;
  //int EchX,EchY;
  //int DivX,DivY;
  //bool ok;
  //std::list<tCaract> *G;

void SetParmsGraph(tGraph *tmp);
LRESULT CALLBACK BatteryGraphWndProc(HWND hwnd, UINT msg, WPARAM wParam,LPARAM lParam);
ATOM BatteryGraphRegisterClass (HINSTANCE hInstance);

void DrawHLine(HDC hdc, RECT rect, int PosY);
void DrawVLine(HDC hdc, RECT rect, int PosX);
void DrawSegment(HDC hdc, RECT rect, int PosX,int PosY);
void SecondsToString(int temp,TCHAR *szBufW);
void DrawControls(HWND hwnd);
void GraphUpdate(HWND hwnd);

/********************************************************************/
void SetParmsGraph(tGraph *tmp)
/********************************************************************/
{
  RECT rect;
  HWND hwnd;

  ResultGraph= *tmp;
  ResultGraph.ok=1;

  if (GraphHwnd!=0) {
    hwnd = GetDlgItem(GraphHwnd, IDC_CUSTOM1);
    GetClientRect(hwnd,&rect);
    InvalidateRect (hwnd,&rect,true);
  }
}

/********************************************************************/
LRESULT CALLBACK BatteryGraphWndProc(HWND hwnd, UINT msg, WPARAM wParam,LPARAM lParam)
/********************************************************************/
{
  switch (msg) {
    case WM_CREATE: {
      GraphHwnd = hwnd;
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
void DrawHLine(HDC hdc, RECT rect, int PosY)
/********************************************************************/
{
  MoveToEx (hdc,rect.left   ,(rect.bottom*PosY)/10000,NULL);
  LineTo   (hdc,rect.right-1,(rect.bottom*PosY)/10000);
}

/********************************************************************/
void DrawVLine(HDC hdc, RECT rect, int PosX)
/********************************************************************/
{
  MoveToEx (hdc,(rect.right*PosX)/10000,rect.top,NULL);
  LineTo   (hdc,(rect.right*PosX)/10000,rect.bottom);
}

/********************************************************************/
void DrawSegment(HDC hdc, RECT rect, int PosX,int PosY)
/********************************************************************/
{
  LineTo   (hdc,(rect.right*PosX)/10000,(rect.bottom*PosY)/10000);
}

/********************************************************************/
void GraphUpdate(HWND hwnd)
/********************************************************************/
{
  PAINTSTRUCT    ps;
  RECT   rect;
  HBRUSH hBrush; //,hOldBrush;
  HPEN   hPen,hPenG,hPenR,hPenB,  // Handle to the new pen object
         hOldPen;                 // Handle to the old pen object
  int    index,i;
  int    x,px=0,y,py=0;
  HGDIOBJ h1,hOld,hOldBrush;

  if (!ResultGraph.ok)
    return;

  HDC hdc = BeginPaint(hwnd, &ps);

  GetClientRect(hwnd,&rect);

  hBrush = CreateSolidBrush (0x0ffeeee);
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

  for (index=1;index<10000;index+=1000) {
    DrawHLine(hdc,rect,index);
    DrawVLine(hdc,rect,index);
  }

  // Select the old pen back into the device context.
  SelectObject (hdc, hOldPen);
  DeleteObject (hPen);

  // Create a solid pen object and select it.

  hPenR = CreatePen (PS_SOLID, 1, 0x000000FF); // Red
  hPenG = CreatePen (PS_SOLID, 1, 0x0000FF00); // Green
  hPenB = CreatePen (PS_SOLID, 1, 0x00FF0000); // Green

  hOldPen = (HPEN)SelectObject (hdc, hPenR);

  MoveToEx (hdc,0,0,NULL);

  // Analyse des points

  std::list<tCaract> *tmp=ResultGraph.G;
  std::list<tCaract>::iterator ix;
  tCaract toto;

  for (ix=tmp->begin();ix != tmp->end();ix++) {
    toto = *ix;
    // Tracé de la courbe de profondeur
    x = toto.Temps*10000/ResultGraph.EchX;
    y = toto.Profondeur*10000/ResultGraph.EchY;
    if (x!=px || y!=py) {
      DrawSegment(hdc,rect,x,y);
      px=x;
      py=y;
    }
  }

  int colpen[12] = {
    0x00aa00,
    0x00bb00,
    0x00cc00,
    0x00dd00,
    0x00ee00,
    0x00ff00,
    0xaa0000,
    0xbb0000,
    0xcc0000,
    0xdd0000,
    0xee0000,
    0xff0000
  };
  HPEN tabpen[12];
  hOldPen = (HPEN)SelectObject (hdc, hPenB);

  for (i=0;i<12;i++) {
    tabpen[i] = CreatePen(PS_SOLID,1,colpen[i]);
  }

  for (i=0;i<12;i++) {
    MoveToEx (hdc,0,0,NULL);
    SelectObject (hdc, tabpen[i]);
    for (ix=tmp->begin();ix != tmp->end();ix++) {
      toto = *ix;
      x=toto.Temps*10000/ResultGraph.EchX;
      y=toto.profMin[i]*10000/ResultGraph.EchY;
      if ((px!=x || py!=y) && (y>0)) {
        // Tracé de la courbe de profondeur
        DrawSegment(hdc,rect,x,y);
        px=x; py=y;
	  }
    }
  }

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
