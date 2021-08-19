/***************************************************************************
  qgsquickutils.cpp
  --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QGuiApplication>
#include <QScreen>
#include <QString>
#include <QWindow>

#include "qgsquickmapsettings.h"
#include "qgsquickutils.h"

QgsQuickUtils::QgsQuickUtils( QObject *parent )
  : QObject( parent )
  , mScreenDensity( calculateScreenDensity() )
{
}

qreal QgsQuickUtils::screenDensity() const
{
  return mScreenDensity;
}

qreal QgsQuickUtils::calculateScreenDensity()
{
  // calculate screen density for calculation of real pixel sizes from density-independent pixels
  // take the first top level window
  double dpi = 96.0;
  const QWindowList windows = QGuiApplication::topLevelWindows();
  if ( !windows.isEmpty() )
  {
    QScreen *screen = windows.at( 0 )->screen();
    double dpiX = screen->physicalDotsPerInchX();
    double dpiY = screen->physicalDotsPerInchY();
    dpi = dpiX < dpiY ? dpiX : dpiY; // In case of asymmetrical DPI. Improbable
  }
  return dpi / 160.;  // 160 DPI is baseline for density-independent pixels in Android
}
