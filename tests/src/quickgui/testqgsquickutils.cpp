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
#include <QFile>
#include <QTextStream>
#include <QObject>
#include <QString>
#include <QStringList>

#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgspoint.h"
#include "qgspointxy.h"
#include "qgstest.h"

#include "qgsquickutils.h"

class TestQgsQuickUtils: public QObject
{
    Q_OBJECT
  private slots:
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void crs_and_geometry();
    void formatting();
};

void TestQgsQuickUtils::crs_and_geometry()
{
  QgsCoordinateReferenceSystem crs3857 = QgsQuickUtils::instance()->coordinateReferenceSystemFromEpsgId( 3857 );
  QVERIFY( crs3857.authid() == "EPSG:3857" );

  QgsCoordinateReferenceSystem crsGPS = QgsQuickUtils::instance()->coordinateReferenceSystemFromEpsgId( 4326 );
  QVERIFY( crsGPS.authid() == "EPSG:4326" );

  QgsPointXY pointXY = QgsQuickUtils::instance()->pointXYFactory( 49.9, 16.3 );
  QVERIFY( pointXY.x() == 49.9 );
  QVERIFY( pointXY.y() == 16.3 );

  QgsPoint point = QgsQuickUtils::instance()->pointFactory( 1.0, -1.0 );
  QVERIFY( point.x() == 1.0 );
  QVERIFY( point.y() == -1.0 );

  QgsPointXY transformedPoint = QgsQuickUtils::instance()->transformPoint( crsGPS,
                                crs3857,
                                QgsCoordinateTransformContext(),
                                pointXY );
  QVERIFY( fabs( transformedPoint.x() - 5554843 ) < 1.0 );
  QVERIFY( fabs( transformedPoint.y() - 1839491 ) < 1.0 );

  QgsQuickMapSettings ms;
  ms.setDestinationCrs( crsGPS );
  ms.setExtent( QgsRectangle( 49, 16, 50, 17 ) );
  ms.setOutputSize( QSize( 1000, 500 ) );
  double sutm = QgsQuickUtils::instance()->screenUnitsToMeters( &ms, 1 );
  QVERIFY( fabs( sutm - 213 ) < 1.0 );
}

void TestQgsQuickUtils::formatting()
{
  QgsPoint point( -2.234521, 34.4444421 );
  QString point2str = QgsQuickUtils::instance()->qgsPointToString( point, 3 );
  QVERIFY( point2str == "-2.235, 34.444" );

  point2str = QgsQuickUtils::instance()->qgsPointToString( point, 2 );
  QVERIFY( point2str == "-2.23, 34.44" );

  point2str = QgsQuickUtils::instance()->qgsPointToString( point, 1 );
  QVERIFY( point2str == "-2.2, 34.4" );

  point2str = QgsQuickUtils::instance()->qgsPointToString( point, 0 );
  QVERIFY( point2str == "-2, 34" );

  QString dist2str = QgsQuickUtils::instance()->distanceToString( 1222.234, 2 );
  QVERIFY( dist2str == "1.22 km" );

  dist2str = QgsQuickUtils::instance()->distanceToString( 1222.234, 1 );
  QVERIFY( dist2str == "1.2 km" );

  dist2str = QgsQuickUtils::instance()->distanceToString( 1222.234, 0 );
  QVERIFY( dist2str == "1 km" );

  dist2str = QgsQuickUtils::instance()->distanceToString( 700.22, 1 );
  QVERIFY( dist2str == "700.2 m" );

  dist2str = QgsQuickUtils::instance()->distanceToString( 0.22, 0 );
  QVERIFY( dist2str == "220 mm" );
}

QGSTEST_MAIN( TestQgsQuickUtils )
#include "testqgsquickutils.moc"
