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

#include <QDesktopWidget>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QThread>

#include "qgis.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsdistancearea.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"

#include "qgsquickmapsettings.h"
#include "qgsquickutils.h"
#include "qgsunittypes.h"

QgsQuickUtils *QgsQuickUtils::sInstance = nullptr;

QgsQuickUtils *QgsQuickUtils::instance()
{
  if ( !sInstance )
  {
    QgsDebugMsg( QStringLiteral( "QgsQuickUtils created: %1" ).arg( long( QThread::currentThreadId() ) ) );
    sInstance = new QgsQuickUtils();
  }
  return sInstance;
}

QgsQuickUtils::QgsQuickUtils( QObject *parent )
  : QObject( parent )
{
  // calculate screen density for calculation of real pixel sizes from density-independent pixels
  int dpiX = QApplication::desktop()->physicalDpiX();
  int dpiY = QApplication::desktop()->physicalDpiY();
  int dpi = dpiX < dpiY ? dpiX : dpiY; // In case of asymmetrical DPI. Improbable
  mScreenDensity = dpi / 160.;  // 160 DPI is baseline for density-independent pixels in Android
}

QString QgsQuickUtils::dumpScreenInfo() const
{
  QRect rec = QApplication::desktop()->screenGeometry();
  int dpiX = QApplication::desktop()->physicalDpiX();
  int dpiY = QApplication::desktop()->physicalDpiY();
  int height = rec.height();
  int width = rec.width();
  double sizeX = static_cast<double>( width ) / dpiX * 25.4;
  double sizeY = static_cast<double>( height ) / dpiY * 25.4;

  QString msg;
  msg += tr( "screen resolution: %1x%2 px\n" ).arg( width, height );
  msg += tr( "screen DPI: %1x%2\n" ).arg( dpiX,  dpiY );
  msg += tr( "screen size: %1x%2 mm\n" ).arg( QString::number( sizeX, 'f', 0 ), QString::number( sizeY, 'f', 0 ) );
  msg += tr( "screen density: %1" ).arg( mScreenDensity );
  return msg;
}

qreal QgsQuickUtils::screenDensity() const
{
  return mScreenDensity;
}
