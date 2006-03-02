/* g4wihelp.c - NSIS Helper DLL used with gpg4win. -*- coding: latin-1; -*-
 * Copyright (C) 2005 g10 Code GmbH
 * Copyright (C) 2001 Justin Frankel
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must
 *    not claim that you wrote the original software. If you use this
 *    software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 * 
 * 2. Altered source versions must be plainly marked as such, and must
 *    not be misrepresented as being the original software.
 * 
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 ************************************************************
 * The code for the splash screen has been taken from the Splash
 * plugin of the NSIS 2.04 distribution.  That code comes without
 * explicit copyright notices in tyhe source files or author names, it
 * seems that it has been written by Justin Frankel; not sure about
 * the year, though. [wk 2005-11-28]
 */

#include <windows.h>
#include "exdll.h"

static HINSTANCE g_hInstance; /* Our Instance. */
static HWND g_hwndParent;     /* Handle of parent window or NULL. */
static HBITMAP g_hbm;         /* Handle of the splash image. */ 
static int sleepint;          /* Milliseconds to show the spals image. */


/* Standard entry point for DLLs. */
int WINAPI
DllMain (HANDLE hinst, DWORD reason, LPVOID reserved)
{
   if (reason == DLL_PROCESS_ATTACH)
     g_hInstance = hinst;
   return TRUE;
}



/* Dummy function for testing. */
void __declspec(dllexport) 
dummy (HWND hwndParent, int string_size, char *variables, 
       stack_t **stacktop, extra_parameters_t *extra)
{
  g_hwndParent = hwndParent;

  EXDLL_INIT();

  // note if you want parameters from the stack, pop them off in order.
  // i.e. if you are called via exdll::myFunction file.dat poop.dat
  // calling popstring() the first time would give you file.dat,
  // and the second time would give you poop.dat. 
  // you should empty the stack of your parameters, and ONLY your
  // parameters.

  // do your stuff here
  {
    char buf[1024];
    sprintf(buf,"$R0=%s\r\n$R1=%s\r\n",
            getuservariable(INST_R0),
            getuservariable(INST_R1));
    MessageBox(g_hwndParent,buf,0,MB_OK);

    sprintf (buf,
             "autoclose    =%d\r\n"
             "all_user_var =%d\r\n"
             "exec_error   =%d\r\n"
             "abort        =%d\r\n"
             "exec_reboot  =%d\r\n"
             "reboot_called=%d\r\n"
             "silent       =%d\r\n"
             "instdir_error=%d\r\n"
             "rtl          =%d\r\n"
             "errlvl       =%d\r\n",
             extra->exec_flags->autoclose,
             extra->exec_flags->all_user_var,
             extra->exec_flags->exec_error,
             extra->exec_flags->abort,
             extra->exec_flags->exec_reboot,
             extra->exec_flags->reboot_called,
             extra->exec_flags->silent,
             extra->exec_flags->instdir_error,
             extra->exec_flags->rtl,
             extra->exec_flags->errlvl);      
    MessageBox(g_hwndParent,buf,0,MB_OK);
  }
}



void __declspec(dllexport) 
runonce (HWND hwndParent, int string_size, char *variables, 
         stack_t **stacktop, extra_parameters_t *extra)
{
  const char *result;

  g_hwndParent = hwndParent;
  EXDLL_INIT();

  CreateMutexA (NULL, 0, getuservariable(INST_R0));
  result = GetLastError ()? "1":"0";
  setuservariable (INST_R0, result);
}


void __declspec(dllexport) 
playsound (HWND hwndParent, int string_size, char *variables, 
           stack_t **stacktop, extra_parameters_t *extra)
{
  char fname[MAX_PATH];

  g_hwndParent = hwndParent;
  EXDLL_INIT();

  if (popstring(fname, sizeof fname))
    return;
  PlaySound (fname, NULL, SND_ASYNC|SND_FILENAME|SND_NODEFAULT);
}


void __declspec(dllexport) 
stopsound (HWND hwndParent, int string_size, char *variables, 
           stack_t **stacktop, extra_parameters_t *extra)
{
  g_hwndParent = hwndParent;
  EXDLL_INIT();
  PlaySound (NULL, NULL, 0);
}


/* Windows procedure to control the splashimage.  This one pauses the
   execution until the sleep time is over or the user closes this
   windows. */
static LRESULT CALLBACK 
splash_wndproc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  LRESULT result = 0;

  switch (uMsg)
    {
    case WM_CREATE:
      {
        BITMAP bm;
        RECT vp;

        GetObject(g_hbm, sizeof(bm), (LPSTR)&bm);
        SystemParametersInfo(SPI_GETWORKAREA, 0, &vp, 0);
        SetWindowLong(hwnd,GWL_STYLE,0);
        SetWindowPos(hwnd,NULL,
                     vp.left+(vp.right-vp.left-bm.bmWidth)/2,
                     vp.top+(vp.bottom-vp.top-bm.bmHeight)/2,
                     bm.bmWidth,bm.bmHeight,
                     SWP_NOZORDER);
        ShowWindow(hwnd,SW_SHOW);
        SetTimer(hwnd,1,sleepint,NULL);
      }
      break;
  
    case WM_PAINT:
      {
        PAINTSTRUCT ps;
        RECT r;
        HDC curdc=BeginPaint(hwnd,&ps);
        HDC hdc=CreateCompatibleDC(curdc);
        HBITMAP oldbm;
        GetClientRect(hwnd,&r);
        oldbm=(HBITMAP)SelectObject(hdc,g_hbm);
        BitBlt(curdc,r.left,r.top,r.right-r.left,r.bottom-r.top,
               hdc,0,0,SRCCOPY);
        SelectObject(hdc,oldbm);
        DeleteDC(hdc);
        EndPaint(hwnd,&ps);
      }
      break;

    case WM_CLOSE:
      break;

    case WM_TIMER:
    case WM_LBUTTONDOWN:
      DestroyWindow(hwnd);
      /*(fall through)*/
    default:
      result =  DefWindowProc (hwnd, uMsg, wParam, lParam);
    }

  return result;
}


/* Display a splash screen.  Call as

     g4wihelp::showsplash SLEEP FNAME

   With SLEEP being the time in milliseconds to show the splashscreen
   and FNAME the complete filename of the image.  As of now only BMP
   is supported.
*/
void __declspec(dllexport) 
showsplash (HWND hwndParent, int string_size, char *variables, 
           stack_t **stacktop, extra_parameters_t *extra)
{
  static WNDCLASS wc;
  char sleepstr[30];
  char fname[MAX_PATH];
  int err = 0;
  char *p;
  char classname[] = "_sp";

  g_hwndParent = hwndParent;
  EXDLL_INIT();
  if (popstring(sleepstr, sizeof sleepstr))
    err = 1;
  if (popstring(fname, sizeof fname))
    err = 1;
  if (err)
    return;

  if (!*fname)
    return; /* Nothing to do. */

  for (sleepint=0, p=sleepstr; *p >= '0' && *p <= '9'; p++)
    {
      sleepint *= 10;
      sleepint += *p - '0';
    }
  if (sleepint <= 0)
    return; /* Nothing to do. */

  wc.lpfnWndProc = splash_wndproc;
  wc.hInstance = g_hInstance;
  wc.hCursor = LoadCursor(NULL,IDC_ARROW);
  wc.lpszClassName = classname;
  if (!RegisterClass(&wc)) 
    return; /* Error. */

  g_hbm = LoadImage (NULL, fname, IMAGE_BITMAP,
                     0, 0 , LR_CREATEDIBSECTION|LR_LOADFROMFILE);
  if (g_hbm) 
    {
      MSG msg;
      HWND hwnd;

      hwnd = CreateWindowEx (WS_EX_TOOLWINDOW, classname, classname,
                             0, 0, 0, 0, 0, (HWND)hwndParent, NULL,
                             g_hInstance, NULL);
      
      while (IsWindow(hwnd) && GetMessage ( &msg, hwnd, 0, 0))
        {
          DispatchMessage (&msg);
        }

      DeleteObject (g_hbm);
      g_hbm = NULL;
    }
  UnregisterClass (classname, g_hInstance);
}
