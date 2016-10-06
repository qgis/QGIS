/***************************************************************************
                          qgsgrasswin.cpp
                             -------------------
    begin                : October, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGlobal>

#include "qgslogger.h"
#include "qgsgrasswin.h"

#ifdef Q_OS_WIN
#include "Windows.h"
#include "WinDef.h"
#include "Winuser.h"
#endif

#ifdef Q_OS_WIN
// Ideas/code from http://stackoverflow.com/questions/20162359/c-best-way-to-get-window-handle-of-the-only-window-from-a-process-by-process
// Get window for pid
struct EnumData
{
  DWORD dwProcessId;
  HWND hWnd;
};
BOOL CALLBACK EnumProc( HWND hWnd, LPARAM lParam )
{
  EnumData& ed = *( EnumData* )lParam;
  DWORD dwProcessId = 0x0;
  GetWindowThreadProcessId( hWnd, &dwProcessId );
  if ( ed.dwProcessId == dwProcessId )
  {
    ed.hWnd = hWnd;
    SetLastError( ERROR_SUCCESS );
    return FALSE;
  }
  return TRUE;
}
HWND FindWindowFromProcessId( DWORD dwProcessId )
{
  EnumData ed = { dwProcessId };
  if ( !EnumWindows( EnumProc, ( LPARAM )&ed ) &&
       ( GetLastError() == ERROR_SUCCESS ) )
  {
    return ed.hWnd;
  }
  return NULL;
}
#endif

void QgsGrassWin::hideWindow( int pid )
{
  Q_UNUSED( pid )
  QgsDebugMsg( QString( "pid = %1" ).arg( pid ) );
#ifdef Q_OS_WIN
  HWND hWnd = FindWindowFromProcessId(( DWORD )pid );
  if ( hWnd )
  {
    QgsDebugMsg( "driver window found -> minimize" );
  }
  else
  {
    QgsDebugMsg( "cannot find driver window" );
  }
  // Unfortunately the window opens first for a moment
  ShowWindow( hWnd, SW_HIDE );
#endif
}

