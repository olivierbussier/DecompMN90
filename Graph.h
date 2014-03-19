#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED

#define BATTERYGRAPHCLASS       TEXT("BatteryGraph")

void SetParmsGraph(tGraph *tmp);
LRESULT CALLBACK BatteryGraphWndProc(HWND hwnd, UINT msg, WPARAM wParam,LPARAM lParam);
ATOM BatteryGraphRegisterClass (HINSTANCE hInstance);
void DrawHLine(HDC hdc, RECT rect, int PosY);
void DrawVLine(HDC hdc, RECT rect, int PosX);
void DrawSegment(HDC hdc, RECT rect, int PosX,int PosY);
void SecondsToString(int temp,TCHAR *szBufW);
void DrawControls(HWND hwnd);

// Publics

void GraphUpdate(HWND hwnd);


#endif // GRAPH_H_INCLUDED
