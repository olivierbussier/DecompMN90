#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "resource.h"
#include "uutil.h"
#include "version.h"

#include "DecompMN90.h"
#include "Graph.h"

HINSTANCE hInst;
char *bufstr;

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
    int vdesc,vasca,vascp,verbose,result;

    switch(uMsg) {
      case WM_INITDIALOG:
        SetValueInt  (hwndDlg,ID_VDESC , 20000);
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
            result  = Decomp(Prof,Duree,verbose,vdesc,vasca,vascp,vazotsurf);
            hwndtemp= GetDlgItem (hwndDlg,ID_TEXTRESULT);
            bufstr  = strget();
            SetWindowText(hwndtemp,bufstr);
            strend();
            break;
          case ID_HELP_ABOUT:
            DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(DLG_ABOUT), hwndDlg, AboutDlgProc);
            break;
          case ID_OPEN_DIVE:
            OPENFILENAME ofn;       // common dialog box structure
            char szFile[512];
            //HANDLE hf;

            // Initialize OPENFILENAME
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwndDlg;
            ofn.lpstrFile = szFile;
            // Set lpstrFile[0] to '\0' so that GetOpenFileName does not
            // use the contents of szFile to initialize itself.
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "xml files\0*.xml\0text files\0*.txt\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

// Display the Open dialog box.

            result = GetOpenFileName(&ofn);
            if(!result)
              break;
            vazotsurf   = GetValueFloat(hwndDlg,ID_VAZOTSURF);
            verbose = (IsDlgButtonChecked  (hwndDlg,ID_VERBOSE)==BST_CHECKED);
            // Lecture XML
            result = DiveXML(ofn.lpstrFile,verbose,vazotsurf);
            if (!result)
              break;
            hwndtemp= GetDlgItem (hwndDlg,ID_TEXTRESULT);
            bufstr  = strget();
            SetWindowText(hwndtemp,bufstr);
            strend();
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
