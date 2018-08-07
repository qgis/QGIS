/***************************************************************************
    qgswinnative.h - abstracted interface to native Mac objective-c
                             -------------------
    begin                : January 2014
    copyright            : (C) 2014 by Larry Shaffer
    email                : larrys at dakotacarto dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMACNATIVE_H
#define QGSMACNATIVE_H

#include "qgsnative.h"
#include <windows.h>
#include <shlobj.h>
#pragma comment(lib,"Shell32.lib")

class QWinTaskbarButton;
class QWinTaskbarProgress;
class QWindow;

class NATIVE_EXPORT QgsWinNative : public QgsNative
{
  public:
    void initializeMainWindow( QWindow *window ) override;
    void openFileExplorerAndSelectFile( const QString &path ) override;
    void showUndefinedApplicationProgress() override;
    void setApplicationProgress( double progress ) override;
    void hideApplicationProgress() override;
    void onRecentProjectsChanged( const std::vector< RecentProjectProperties > &recentProjects ) override;

  private:

    QWinTaskbarButton *mTaskButton = nullptr;
    QWinTaskbarProgress *mTaskProgress = nullptr;
};

#endif // QGSMACNATIVE_H
