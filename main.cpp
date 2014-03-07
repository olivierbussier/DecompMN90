#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "resource.h"
#include "uutil.h"
#include "version.h"

#include "DecompMN90.h"

HINSTANCE hInst;
char *bufstr;

void GraphUpdate(HWND hwnd);

int    GetValueInt(HWND hwndDlg,int ID);
void   SetValueInt(HWND hwndDlg,int ID, int Value);
double GetValueFloat(HWND hwndDlg,int ID);
void   SetValueFloat(HWND hwndDlg,int ID, double Value);
BOOL   CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL   CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
/*
	//Date Version Types
	static const char DATE[] = "04";
	static const char MONTH[] = "03";
	static const char YEAR[] = "2014";
	static const char UBUNTU_VERSION_STYLE[] =  "14.03";

	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";

	//Standard Version Type
	static const long MAJOR  = 0;
	static const long MINOR  = 4;
	static const long BUILD  = 9;
	static const long REVISION  = 42;

	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 21;
	#define RC_FILEVERSION 0,4,9,42
	#define RC_FILEVERSION_STRING "0, 4, 9, 42\0"
	static const char FULLVERSION_STRING [] = "0.4.9.42";

	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 9;
*/

/**************************************************************************/
BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
/**************************************************************************/
{
  char buffer[2048];

  switch(Message) {
    case WM_INITDIALOG:
      sprintf(buffer,"Version %li.%li.%li révision %li du %s/%s/%s\r\nBuild No %li",AutoVersion::MAJOR,
                                                                                    AutoVersion::MINOR,
                                                                                    AutoVersion::BUILD,
                                                                                    AutoVersion::REVISION,
                                                                                    AutoVersion::DATE,
                                                                                    AutoVersion::MONTH,
                                                                                    AutoVersion::YEAR,
                                                                                    AutoVersion::BUILDS_COUNT);
      SetWindowText(GetDlgItem (hwnd,ID_VERSION),buffer);
      return TRUE;
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
          EndDialog(hwnd, IDOK);
          break;
      }
      break;
    default:
      return FALSE;
  }
  return TRUE;
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
        SetValueInt  (hwndDlg,ID_VDESC , 20);
        SetValueInt  (hwndDlg,ID_VASCA , 15);
        SetValueInt  (hwndDlg,ID_VASCP,   6);
        SetValueFloat(hwndDlg,ID_VAZOTSURF,0.7808);

        // Set the font for EDIT
        hFont=CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS, "Courier New");
        // Set the new font for the control:
        SendMessage (GetDlgItem (hwndDlg,ID_TEXTRESULT), WM_SETFONT, WPARAM (hFont), TRUE);
        GraphUpdate(GetDlgItem (hwndDlg,IDC_CUSTOM1));
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
            bufstr  = Decomp(Prof,Duree,verbose,vdesc,vasca,vascp,vazotsurf);
            hwndtemp= GetDlgItem (hwndDlg,ID_TEXTRESULT);
            SetWindowText(hwndtemp,bufstr);
            strend(bufstr);
            break;
          case ID_HELP_ABOUT:
            DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(DLG_ABOUT), hwndDlg, AboutDlgProc);
            break;
        }
        return TRUE;
    }
    return FALSE;
}

ATOM BatteryGraphRegisterClass (HINSTANCE hInstance);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    hInst=hInstance;

    BatteryGraphRegisterClass (hInstance);

    InitCommonControls();

    bufstr=(char *)malloc(100000);
    return DialogBox(hInst, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgMain);
}
