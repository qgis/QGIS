/***************************************************************************
    qgscanvasrefreshblocker.cpp
    ---------------------------
    begin                : July 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscanvasrefreshblocker.h"
#include "qgisapp.h"

QgsCanvasRefreshBlocker::QgsCanvasRefreshBlocker()
{
  if ( QgisApp::instance()->mFreezeCount++ == 0 )
  {
    // going from unfrozen to frozen, so freeze canvases
    QgisApp::instance()->freezeCanvases( true );
  }
}

void QgsCanvasRefreshBlocker::release()
{
  if ( mReleased )
    return;

  mReleased = true;
  if ( --QgisApp::instance()->mFreezeCount == 0 )
  {
    // going from frozen to unfrozen, so unfreeze canvases
    QgisApp::instance()->freezeCanvases( false );
    QgisApp::instance()->refreshMapCanvas();
  }
}

QgsCanvasRefreshBlocker::~QgsCanvasRefreshBlocker()
{
  if ( !mReleased )
    release();
}

