/***************************************************************************
    qgsmultirenderchecker.cpp
     --------------------------------------
    Date                 : 6.11.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmultirenderchecker.h"

#include <QDebug>

QgsMultiRenderChecker::QgsMultiRenderChecker()
{
}

void QgsMultiRenderChecker::setControlName( const QString& theName )
{
  mControlName = theName;
}

void QgsMultiRenderChecker::setControlPathPrefix( const QString& prefix )
{
  mControlPathPrefix = prefix;
}

void QgsMultiRenderChecker::setMapSettings( const QgsMapSettings& mapSettings )
{
  mMapSettings = mapSettings;
}

bool QgsMultiRenderChecker::runTest( const QString& theTestName, unsigned int theMismatchCount )
{
  bool successful = false;

  const QString baseDir = controlImagePath();

  QStringList subDirs = QDir( baseDir ).entryList( QDir::Dirs | QDir::NoDotAndDotDot );

  if ( subDirs.count() == 0 )
  {
    subDirs << "";
  }

  QVector<QgsDartMeasurement> dartMeasurements;

  Q_FOREACH ( const QString& suffix, subDirs )
  {
    qDebug() << "Checking subdir " << suffix;
    bool result;
    QgsRenderChecker checker;
    checker.enableDashBuffering( true );
    checker.setColorTolerance( mColorTolerance );
    checker.setControlPathPrefix( mControlPathPrefix );
    checker.setControlPathSuffix( suffix );
    checker.setControlName( mControlName );
    checker.setMapSettings( mMapSettings );

    if ( !mRenderedImage.isNull() )
    {
      checker.setRenderedImage( mRenderedImage );
      result = checker.compareImages( theTestName, theMismatchCount, mRenderedImage );
    }
    else
    {
      result = checker.runTest( theTestName, theMismatchCount );
      mRenderedImage = checker.renderedImage();
    }

    successful |= result;

    dartMeasurements << checker.dartMeasurements();

    mReport += checker.report();
  }

  if ( !successful )
  {
    Q_FOREACH ( const QgsDartMeasurement& measurement, dartMeasurements )
      measurement.send();

    QgsDartMeasurement msg( "Image not accepted by test", QgsDartMeasurement::Text, "This may be caused because the test is supposed to fail or rendering inconsistencies."
                            "If this is a rendering inconsistency, please add another control image folder, add an anomaly image or increase the color tolerance." );
    msg.send();
  }

  return successful;
}

const QString QgsMultiRenderChecker::controlImagePath() const
{
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString myControlImageDir = myDataDir + QDir::separator() + "control_images" +
                              QDir::separator() + mControlPathPrefix + QDir::separator() + mControlName + QDir::separator();
  return myControlImageDir;
}
