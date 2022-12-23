/***************************************************************************
                             qgsappwindowmanager.cpp
                             -----------------------
    Date                 : September 2018
    Copyright            : (C) 2018 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappwindowmanager.h"
#include "qgsstylemanagerdialog.h"
#include "qgsstyle.h"
#include "qgisapp.h"
#include "qgslayoutmanagerdialog.h"
#include "qgsrasterlayer.h"

#ifdef HAVE_3D
#include "qgs3dviewsmanagerdialog.h"
#endif

QgsAppWindowManager::~QgsAppWindowManager()
{
  if ( mStyleManagerDialog )
    delete mStyleManagerDialog;
  if ( mLayoutManagerDialog )
    delete mLayoutManagerDialog;
}

QWidget *QgsAppWindowManager::openStandardDialog( QgsWindowManagerInterface::StandardDialog dialog )
{
  switch ( dialog )
  {
    case QgsWindowManagerInterface::DialogStyleManager:
    {
      if ( !mStyleManagerDialog )
      {
        mStyleManagerDialog = new QgsStyleManagerDialog( QgisApp::instance(), Qt::Window );
        mStyleManagerDialog->setAttribute( Qt::WA_DeleteOnClose );
      }
      mStyleManagerDialog->show();
      mStyleManagerDialog->activate();
      return mStyleManagerDialog;
    }
  }
  return nullptr;
}

QWidget *QgsAppWindowManager::openApplicationDialog( QgsAppWindowManager::ApplicationDialog dialog )
{
  switch ( dialog )
  {
    case DialogLayoutManager:
    {
      if ( !mLayoutManagerDialog )
      {
        mLayoutManagerDialog = new QgsLayoutManagerDialog( QgisApp::instance(), Qt::Window );
        mLayoutManagerDialog->setAttribute( Qt::WA_DeleteOnClose );
      }
      mLayoutManagerDialog->show();
      mLayoutManagerDialog->activate();
      return mLayoutManagerDialog;
    }
    case Dialog3DMapViewsManager:
    {
#ifdef HAVE_3D
      if ( !m3DMapViewsManagerDialog )
      {
        m3DMapViewsManagerDialog = new Qgs3DViewsManagerDialog( QgisApp::instance(), Qt::Window );
        m3DMapViewsManagerDialog->setAttribute( Qt::WA_DeleteOnClose );
      }
      m3DMapViewsManagerDialog->show();
      m3DMapViewsManagerDialog->reload();
      m3DMapViewsManagerDialog->raise();
      m3DMapViewsManagerDialog->setWindowState( m3DMapViewsManagerDialog->windowState() & ~Qt::WindowMinimized );
      m3DMapViewsManagerDialog->activateWindow();
      return m3DMapViewsManagerDialog;
#endif
    }
  }
  return nullptr;
}
