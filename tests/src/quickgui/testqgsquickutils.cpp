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
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgspoint.h"
#include "qgspointxy.h"
#include "qgstest.h"
#include "qgis.h"
#include "qgsunittypes.h"

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
    void transformedPoint();
    void qgsPointToString();
    void distanceToString();

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

void TestQgsQuickUtils::transformedPoint()
{
  QgsPointXY pointXY = utils.pointXYFactory( 49.9, 16.3 );
  QVERIFY( pointXY.x() == 49.9 );
  QVERIFY( pointXY.y() == 16.3 );

  QgsPoint point = utils.pointFactory( 1.0, -1.0 );
  QVERIFY( point.x() == 1.0 );
  QVERIFY( point.y() == -1.0 );

  QgsCoordinateReferenceSystem crs3857 = QgsCoordinateReferenceSystem::fromEpsgId( 3857 );
  QVERIFY( crs3857.authid() == "EPSG:3857" );

  QgsCoordinateReferenceSystem crsGPS = QgsCoordinateReferenceSystem::fromEpsgId( 4326 );
  QVERIFY( crsGPS.authid() == "EPSG:4326" );

  QgsPointXY transformedPoint = utils.transformPoint( crsGPS,
                                crs3857,
                                QgsCoordinateTransformContext(),
                                pointXY );
  QVERIFY( fabs( transformedPoint.x() - 5554843 ) < 1.0 );
  QVERIFY( fabs( transformedPoint.y() - 1839491 ) < 1.0 );
}

void TestQgsQuickUtils::qgsPointToString()
{
  QgsPoint point( -2.234521, 34.4444421 );
  QString point2str = utils.qgsPointToString( point, 3 );
  QVERIFY( point2str == "-2.235, 34.444" );

  point2str = utils.qgsPointToString( point, 2 );
  QVERIFY( point2str == "-2.23, 34.44" );

  point2str = utils.qgsPointToString( point, 1 );
  QVERIFY( point2str == "-2.2, 34.4" );

  point2str = utils.qgsPointToString( point, 0 );
  QVERIFY( point2str == "-2, 34" );
}

void TestQgsQuickUtils::distanceToString()
{
  QString dist2str = utils.distanceToString( 1222.234, QgsUnitTypes::DistanceMeters,  2 );
  QVERIFY( dist2str == "1.22 km" );

  dist2str = utils.distanceToString( 1222.234, QgsUnitTypes::DistanceMeters, 1 );
  QVERIFY( dist2str == "1.2 km" );

  dist2str = utils.distanceToString( 1222.234, QgsUnitTypes::DistanceMeters, 0 );
  QVERIFY( dist2str == "1 km" );

  dist2str = utils.distanceToString( 700.22, QgsUnitTypes::DistanceMeters, 1 );
  QVERIFY( dist2str == "700.2 m" );

  dist2str = utils.distanceToString( 0.22, QgsUnitTypes::DistanceMeters, 0 );
  QVERIFY( dist2str == "220 mm" );

  dist2str = utils.distanceToString( -0.22, QgsUnitTypes::DistanceMeters, 0 );
  QVERIFY( dist2str == "0 m" );

  dist2str = utils.distanceToString( 1.222234, QgsUnitTypes::DistanceKilometers,  2 );
  QVERIFY( dist2str == "1.22 km" );
}

QGSTEST_MAIN( TestQgsQuickUtils )
#include "testqgsquickutils.moc"
