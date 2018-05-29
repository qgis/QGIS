/***************************************************************************
     testqgsquickutils.cpp
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
#include <QObject>
#include <QApplication>
#include <QDesktopWidget>

#include "qgsapplication.h"
#include "qgstest.h"
#include "qgis.h"

#include "qgsquickutils.h"

class TestQgsQuickUtils: public QObject
{
    Q_OBJECT
  private slots:
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void screen_density();
    void dump_screen_info();
    void screenUnitsToMeters();

  private:
    QgsQuickUtils utils;
};

void TestQgsQuickUtils::screen_density()
{
  qreal dp = utils.screenDensity();
  QVERIFY( ( dp > 0 ) && ( dp < 1000 ) );
}

void TestQgsQuickUtils::dump_screen_info()
{
  qreal dp = utils.screenDensity();
  QVERIFY( utils.dumpScreenInfo().contains( QStringLiteral( "%1" ).arg( dp ) ) );
}

void TestQgsQuickUtils::screenUnitsToMeters()
{
  QgsCoordinateReferenceSystem crsGPS = QgsCoordinateReferenceSystem::fromEpsgId( 4326 );
  QVERIFY( crsGPS.authid() == "EPSG:4326" );

  QgsQuickMapSettings ms;
  ms.setDestinationCrs( crsGPS );
  ms.setExtent( QgsRectangle( 49, 16, 50, 17 ) );
  ms.setOutputSize( QSize( 1000, 500 ) );
  double sutm = utils.screenUnitsToMeters( &ms, 1 );
  QGSCOMPARENEAR( sutm, 213, 1.0 );
}

QGSTEST_MAIN( TestQgsQuickUtils )
#include "testqgsquickutils.moc"
