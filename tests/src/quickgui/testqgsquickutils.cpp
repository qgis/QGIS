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
    void formatPoint();
    void formatDistance();
    void loadIcon();
    void fileExists();
    void loadQmlComponent();
    void getRelativePath();

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
  QgsPointXY pointXY = utils.pointXY( 49.9, 16.3 );
  QGSCOMPARENEAR( pointXY.x(), 49.9, 1e-4 );
  QGSCOMPARENEAR( pointXY.y(), 16.3, 1e-4 );

  QgsPoint point = utils.point( 1.0, -1.0 );
  QGSCOMPARENEAR( point.x(), 1.0, 1e-4 );
  QGSCOMPARENEAR( point.y(), -1.0, 1e-4 );

  QgsCoordinateReferenceSystem crs3857 = QgsCoordinateReferenceSystem::fromEpsgId( 3857 );
  QVERIFY( crs3857.authid() == "EPSG:3857" );

  QgsCoordinateReferenceSystem crsGPS = QgsCoordinateReferenceSystem::fromEpsgId( 4326 );
  QVERIFY( crsGPS.authid() == "EPSG:4326" );

  QgsPointXY transformedPoint = utils.transformPoint( crsGPS,
                                crs3857,
                                QgsCoordinateTransformContext(),
                                pointXY );
  QGSCOMPARENEAR( transformedPoint.x(), 5554843, 1.0 );
  QGSCOMPARENEAR( transformedPoint.y(), 1839491, 1.0 );
}

void TestQgsQuickUtils::formatPoint()
{
  QgsPoint point( -2.234521, 34.4444421 );
  QString point2str = utils.formatPoint( point );
  QVERIFY( point2str == "-2.235,34.444" );
}

void TestQgsQuickUtils::formatDistance()
{
  QString dist2str = utils.formatDistance( 1222.234, QgsUnitTypes::DistanceMeters,  2 );
  QVERIFY( dist2str == "1.22 km" );

  dist2str = utils.formatDistance( 1222.234, QgsUnitTypes::DistanceMeters, 1 );
  QVERIFY( dist2str == "1.2 km" );

  dist2str = utils.formatDistance( 1222.234, QgsUnitTypes::DistanceMeters, 0 );
  QVERIFY( dist2str == "1 km" );

  dist2str = utils.formatDistance( 700.22, QgsUnitTypes::DistanceMeters, 1 );
  QVERIFY( dist2str == "700.2 m" );

  dist2str = utils.formatDistance( 0.22, QgsUnitTypes::DistanceMeters, 0 );
  QVERIFY( dist2str == "22 cm" );

  dist2str = utils.formatDistance( -0.22, QgsUnitTypes::DistanceMeters, 0 );
  QVERIFY( dist2str == "0 mm" );

  dist2str = utils.formatDistance( 1.222234, QgsUnitTypes::DistanceKilometers,  2 );
  QVERIFY( dist2str == "1.22 km" );

  /////////////////////////////////////////////////////////
  dist2str = utils.formatDistance( 6000, QgsUnitTypes::DistanceFeet, 1, QgsUnitTypes::ImperialSystem );
  QVERIFY( dist2str == "1.1 mi" );

  dist2str = utils.formatDistance( 5, QgsUnitTypes::DistanceFeet, 1, QgsUnitTypes::ImperialSystem );
  QVERIFY( dist2str == "1.7 yd" );

  /////////////////////////////////////////////////////////
  dist2str = utils.formatDistance( 7000, QgsUnitTypes::DistanceFeet, 1, QgsUnitTypes::USCSSystem );
  QVERIFY( dist2str == "1.2 NM" );
}

void TestQgsQuickUtils::loadIcon()
{
  QUrl url = utils.getThemeIcon( "ic_save_white" );
  Q_ASSERT( url.toString() == QStringLiteral( "qrc:/ic_save_white.svg" ) );

  QFileInfo fileInfo( url.toString() );
  QString fileName( fileInfo.fileName() );
  Q_ASSERT( fileName == QStringLiteral( "ic_save_white.svg" ) );
}

void TestQgsQuickUtils::fileExists()
{
  QString path = QStringLiteral( TEST_DATA_DIR ) + "/quickapp_project.qgs";
  Q_ASSERT( utils.fileExists( path ) );
}


void TestQgsQuickUtils::loadQmlComponent()
{
  QUrl dummy = utils.getEditorComponentSource( "dummy" );
  Q_ASSERT( dummy.path() == QString( "qgsquicktextedit.qml" ) );

  QUrl valuemap = utils.getEditorComponentSource( "valuemap" );
  Q_ASSERT( valuemap.path() == QString( "qgsquickvaluemap.qml" ) );
}

void TestQgsQuickUtils::getRelativePath()
{
  QString prefixPath = QStringLiteral( "%1/" ).arg( TEST_DATA_DIR );
  QString fileName = QStringLiteral( "quickapp_project.qgs" );
  QString path =  prefixPath + fileName;
  QString relativePath = utils.getRelativePath( path, prefixPath );
  QCOMPARE( fileName, relativePath );

  QString fileName2 = QStringLiteral( "zip/test.zip" );
  QString path2 = prefixPath + fileName2;
  QString relativePath2 = utils.getRelativePath( path2, prefixPath );
  QCOMPARE( fileName2, relativePath2 );

  QString path3 = QStringLiteral( "file://" ) + path2;
  QString relativePath3 = utils.getRelativePath( path3, prefixPath );
  QCOMPARE( fileName2, relativePath3 );

  QString relativePath4 = utils.getRelativePath( path2, QStringLiteral( "/dummy/path/" ) );
  QCOMPARE( QString(), relativePath4 );

  QString relativePath5 = utils.getRelativePath( path2, QStringLiteral( "" ) );
  QCOMPARE( path2, relativePath5 );
}


QGSTEST_MAIN( TestQgsQuickUtils )
#include "testqgsquickutils.moc"
