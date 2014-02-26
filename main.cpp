#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "resource.h"
#include "decomp.h"
#include "util.h"

HINSTANCE hInst;
char *buffer;

/**************************************************************************/
int GetValueInt(HWND hwndDlg,int ID)
/**************************************************************************/
{
  char result[1024];

  if (GetWindowText(GetDlgItem (hwndDlg,ID),result,sizeof(result))==0)
    return 0;
  return atoi(result);
}

/**************************************************************************/
void SetValueInt(HWND hwndDlg,int ID, int Value)
/**************************************************************************/
{
  char result[1024];

  sprintf (result,"%i",Value);
  SetWindowText(GetDlgItem (hwndDlg,ID),result);
}

/**************************************************************************/
double GetValueFloat(HWND hwndDlg,int ID)
/**************************************************************************/
{
  char result[1024];

  if (GetWindowText(GetDlgItem (hwndDlg,ID),result,sizeof(result))==0)
    return 0;
  return atof(result);
}

/**************************************************************************/
void SetValueFloat(HWND hwndDlg,int ID, double Value)
/**************************************************************************/
{
  char result[1024];

  sprintf (result,"%6.4f",Value);
  SetWindowText(GetDlgItem (hwndDlg,ID),result);
}

/**************************************************************************/
BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
/**************************************************************************/
{
    HWND hwndtemp;
    HFONT hFont;
    double Prof,Duree,vazotsurf;
    int vdesc,vasca,vascp,verbose;

    switch(uMsg) {
      case WM_INITDIALOG:
        SetValueInt  (hwndDlg,ID_VDESC ,200);
        SetValueInt  (hwndDlg,ID_VASCA , 15);
        SetValueInt  (hwndDlg,ID_VASCP,   6);
        SetValueFloat(hwndDlg,ID_VAZOTSURF,0.7808);

        // Set the font for EDIT
        hFont=CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS, "Courier New");
        // Set the new font for the control:
        SendMessage (GetDlgItem (hwndDlg,ID_TEXTRESULT), WM_SETFONT, WPARAM (hFont), TRUE);

        return TRUE;

      case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        return TRUE;

      case WM_COMMAND:
        switch(LOWORD(wParam)) {
          case ID_BT_CALC:
            Prof    = GetValueInt  (hwndDlg,ID_PROF);
            Duree   = GetValueInt  (hwndDlg,ID_DUREE);
            vdesc   = GetValueInt  (hwndDlg,ID_VDESC);
            vasca   = GetValueInt  (hwndDlg,ID_VASCA);
            vascp   = GetValueInt  (hwndDlg,ID_VASCP);
            vazotsurf   = GetValueFloat(hwndDlg,ID_VAZOTSURF);
            verbose = (IsDlgButtonChecked  (hwndDlg,ID_VERBOSE)==BST_CHECKED);
            buffer  = Decomp(Prof,Duree,verbose,vdesc,vasca,vascp,vazotsurf);
            hwndtemp= GetDlgItem (hwndDlg,ID_TEXTRESULT);
            SetWindowText(hwndtemp,buffer);
            strend(buffer);
            break;
        }
        return TRUE;
    }
    return FALSE;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    hInst=hInstance;
    InitCommonControls();
    buffer=(char *)malloc(100000);
    return DialogBox(hInst, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgMain);
}
