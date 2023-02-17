/***************************************************************************
     qgsscreenhelper.cpp
     ---------------
    Date                 : August 2022
    Copyright            : (C) 2022 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsscreenhelper.h"
#include "qgis.h"
#include <QWidget>
#include <QEvent>
#include <QWindow>
#include <QScreen>


QgsScreenHelper::QgsScreenHelper( QWidget *parent )
  : QObject( parent )
  , mWidget( parent )
{
  mWidget->installEventFilter( this );
}

QScreen *QgsScreenHelper::screen()
{
  if ( QWindow *windowHandle = QgsScreenHelper::windowHandle() )
  {
    return windowHandle->screen();
  }
  return nullptr;
}

QWindow *QgsScreenHelper::windowHandle()
{
  if ( QWidget *window = mWidget->window() )
    return window->windowHandle();

  return nullptr;
}

bool QgsScreenHelper::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched != mWidget )
    return false;

  switch ( event->type() )
  {
    case QEvent::Show:
    {
      updateDevicePixelFromScreen();
      updateAvailableGeometryFromScreen();

      // keep device pixel ratio up to date on screen or resolution change
      if ( QWindow *handle = windowHandle() )
      {
        connect( handle, &QWindow::screenChanged, this, [ = ]( QScreen * )
        {
          disconnect( mScreenDpiChangedConnection );
          disconnect( mAvailableGeometryChangedConnection );

          if ( QWindow *windowHandleInLambda = windowHandle() )
          {
            mScreenDpiChangedConnection = connect( windowHandleInLambda->screen(), &QScreen::physicalDotsPerInchChanged, this, &QgsScreenHelper::updateDevicePixelFromScreen );
            updateDevicePixelFromScreen();

            mAvailableGeometryChangedConnection = connect( windowHandleInLambda->screen(), &QScreen::availableGeometryChanged, this, &QgsScreenHelper::updateAvailableGeometryFromScreen );
            updateAvailableGeometryFromScreen();
          }
        } );

        mScreenDpiChangedConnection = connect( handle->screen(), &QScreen::physicalDotsPerInchChanged, this, &QgsScreenHelper::updateDevicePixelFromScreen );
        mAvailableGeometryChangedConnection = connect( handle->screen(), &QScreen::availableGeometryChanged, this, &QgsScreenHelper::updateAvailableGeometryFromScreen );
      }
      break;
    }

    default:
      break;
  }

  return false;
}

void QgsScreenHelper::updateDevicePixelFromScreen()
{
  if ( QScreen *screen = QgsScreenHelper::screen() )
  {
    const double newDpi = screen->physicalDotsPerInch();
    if ( !qgsDoubleNear( newDpi, mScreenDpi ) )
    {
      mScreenDpi = newDpi;
      emit screenDpiChanged( mScreenDpi );
    }
  }
}

void QgsScreenHelper::updateAvailableGeometryFromScreen()
{
  if ( QScreen *screen = QgsScreenHelper::screen() )
  {
    const QRect newGeometry = screen->availableGeometry();
    if ( newGeometry != mAvailableGeometry )
    {
      mAvailableGeometry = newGeometry;
      emit availableGeometryChanged( mAvailableGeometry );
    }
  }
}
