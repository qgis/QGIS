/***************************************************************************
     test_template.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:23 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

//qgis includes...
#include <qgsapplication.h>
#include <qgsgeometry.h>
//header for class being tested
#include <qgspoint.h>

class TestQgsPointXY: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void equality();
    void gettersSetters();
    void constructors();
    void toQPointF();
    void operators();
    void toString();
    void toDegreesMinutesSeconds();
    void toDegreesMinutesSecondsNoSuffix();
    void toDegreesMinutesSecondsPadded();
    void toDegreesMinutes();
    void toDegreesMinutesNoSuffix();
    void toDegreesMinutesPadded();
    void sqrDist();
    void distance();
    void compare();
    void project();
    void vector(); //tests for QgsVector

  private:
    QgsPointXY mPoint1;
    QgsPointXY mPoint2;
    QgsPointXY mPoint3;
    QgsPointXY mPoint4;
    QString mReport;
};

void TestQgsPointXY::init()
{
  //
  // Reset / reinitialize the geometries before each test is run
  //
  mPoint1 = QgsPointXY( 20.0, -20.0 );
  mPoint2 = QgsPointXY( -80.0, 20.0 );
  mPoint3 = QgsPointXY( -80.0, -20.0 );
  mPoint4 = QgsPointXY( 80.0, 20.0 );
}

void TestQgsPointXY::cleanup()
{
  // will be called after every testfunction.
}

void TestQgsPointXY::equality()
{
  QgsPointXY point1( 5.0, 9.0 );
  QgsPointXY point2( 5.0, 9.0 );
  QCOMPARE( point1, point2 );
  QgsPointXY point3( 5.0, 6.0 );
  QVERIFY( !( point3 == point1 ) );
  QVERIFY( point3 != point1 );
  QgsPointXY point4( 8.0, 9.0 );
  QVERIFY( !( point4 == point1 ) );
  QVERIFY( point4 != point1 );
  QVERIFY( !( point4 == point3 ) );
  QVERIFY( point4 != point3 );
}

void TestQgsPointXY::gettersSetters()
{
  QgsPointXY point;
  point.setX( 1.0 );
  QCOMPARE( point.x(), 1.0 );
  point.setY( 2.0 );
  QCOMPARE( point.y(), 2.0 );
  point.set( 3.0, 4.0 );
  QCOMPARE( point.x(), 3.0 );
  QCOMPARE( point.y(), 4.0 );
}

void TestQgsPointXY::constructors()
{
  QgsPointXY point1 = QgsPointXY( 20.0, -20.0 );
  QCOMPARE( point1.x(), 20.0 );
  QCOMPARE( point1.y(), -20.0 );
  QgsPointXY point2( point1 );
  QCOMPARE( point2, point1 );

  QPointF sourceQPointF( 20.0, -20.0 );
  QgsPointXY fromQPointF( sourceQPointF );
  QCOMPARE( fromQPointF.x(), 20.0 );
  QCOMPARE( fromQPointF.y(), -20.0 );

  QPointF sourceQPoint( 20, -20 );
  QgsPointXY fromQPoint( sourceQPoint );
  QCOMPARE( fromQPoint.x(), 20.0 );
  QCOMPARE( fromQPoint.y(), -20.0 );
}

void TestQgsPointXY::toQPointF()
{
  QgsPointXY point( 20.0, -20.0 );
  QPointF result = point.toQPointF();
  QCOMPARE( result.x(), 20.0 );
  QCOMPARE( result.y(), -20.0 );
}

void TestQgsPointXY::operators()
{
  QgsPointXY p( 1, 2 );
  QCOMPARE( p - QgsVector( 3, 5 ), QgsPointXY( -2, -3 ) );
  p -= QgsVector( 3, 5 );
  QCOMPARE( p, QgsPointXY( -2, -3 ) );

  p = QgsPointXY( 1, 2 );
  QCOMPARE( p + QgsVector( 3, 5 ), QgsPointXY( 4, 7 ) );
  p += QgsVector( 3, 5 );
  QCOMPARE( p, QgsPointXY( 4, 7 ) );

  p = QgsPointXY( 1, 2 );
  QCOMPARE( p * 3, QgsPointXY( 3, 6 ) );
  p *= 3;
  QCOMPARE( p, QgsPointXY( 3, 6 ) );

  QCOMPARE( p / 3.0, QgsPointXY( 1, 2 ) );
  p /= 3;
  QCOMPARE( p, QgsPointXY( 1, 2 ) );
}

void TestQgsPointXY::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::showSettings();
  mReport += QLatin1String( "<h1>Point Tests</h1>\n" );
}


void TestQgsPointXY::cleanupTestCase()
{
  //
  // Runs once after all tests are run
  //
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
    //QDesktopServices::openUrl( "file:///" + myReportFile );
  }

}

void TestQgsPointXY::toString()
{
  mReport += QLatin1String( "<p>Testing toString()</p>" );
  mReport += "<p>" + mPoint1.toString( 2 )  +  "</p>";
  mReport += "<p>" + mPoint2.toString( 2 )  +  "</p>";
  mReport += "<p>" + mPoint3.toString( 2 )  +  "</p>";
  mReport += "<p>" + mPoint4.toString( 2 )  +  "</p>";
  QCOMPARE( mPoint1.toString( 2 ), QString( "20.00,-20.00" ) );
}

void TestQgsPointXY::toDegreesMinutesSeconds()
{
  mReport += QLatin1String( "<p>Testing toDegreesMinutesSeconds()</p>" );
  mReport += "<p>" + mPoint1.toDegreesMinutesSeconds( 2 )  +  "</p>";
  mReport += "<p>" + mPoint2.toDegreesMinutesSeconds( 2 )  +  "</p>";
  mReport += "<p>" + mPoint3.toDegreesMinutesSeconds( 2 )  +  "</p>";
  mReport += "<p>" + mPoint4.toDegreesMinutesSeconds( 2 )  +  "</p>";

  qDebug() << mPoint4.toDegreesMinutesSeconds( 2 );
  QString myControlString = QStringLiteral( "80" ) + QChar( 176 ) +
                            QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) +
                            QChar( 0x2033 ) +
                            QStringLiteral( "E,20" ) + QChar( 176 ) +
                            QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) +
                            QStringLiteral( "N" );
  qDebug() << myControlString;
  QCOMPARE( mPoint4.toDegreesMinutesSeconds( 2 ), myControlString );

  //check if longitudes > 180 or <-180 wrap around
  myControlString = QStringLiteral( "10" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) + QStringLiteral( "E" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( 370, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QStringLiteral( "10" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) + QStringLiteral( "W" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( -370, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QStringLiteral( "179" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) + QStringLiteral( "W" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( 181, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QStringLiteral( "179" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) + QStringLiteral( "E" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( -181, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QStringLiteral( "1" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) + QStringLiteral( "W" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( 359, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QStringLiteral( "1" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) + QStringLiteral( "E" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( -359, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );

  //check if latitudes > 90 or <-90 wrap around
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) +
                    QStringLiteral( ",10" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) + QStringLiteral( "N" );
  QCOMPARE( QgsPointXY( 0, 190 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) +
                    QStringLiteral( ",10" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) + QStringLiteral( "S" );
  QCOMPARE( QgsPointXY( 0, -190 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) +
                    QStringLiteral( ",89" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) + QStringLiteral( "S" );
  QCOMPARE( QgsPointXY( 0, 91 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) +
                    QStringLiteral( ",89" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) + QStringLiteral( "N" );
  QCOMPARE( QgsPointXY( 0, -91 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) +
                    QStringLiteral( ",1" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) + QStringLiteral( "S" );
  QCOMPARE( QgsPointXY( 0, 179 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) +
                    QStringLiteral( ",1" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) + QStringLiteral( "N" );
  QCOMPARE( QgsPointXY( 0, -179 ).toDegreesMinutesSeconds( 2 ), myControlString );

  //should be no directional suffixes for 0 degree coordinates
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) +
                    QChar( 0x2033 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( 0, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  //should also be no directional suffix for 0 degree coordinates within specified precision
  QCOMPARE( QgsPointXY( 0, 0.000001 ).toDegreesMinutesSeconds( 2 ), myControlString );
  QCOMPARE( QgsPointXY( 0, -0.000001 ).toDegreesMinutesSeconds( 2 ), myControlString );
  QCOMPARE( QgsPointXY( -0.000001, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  QCOMPARE( QgsPointXY( 0.000001, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00000" ) +
                    QChar( 0x2033 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00360" ) + QChar( 0x2033 ) + QStringLiteral( "N" );
  QCOMPARE( QgsPointXY( 0, 0.000001 ).toDegreesMinutesSeconds( 5 ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00000" ) +
                    QChar( 0x2033 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00360" ) + QChar( 0x2033 ) + QStringLiteral( "S" );
  QCOMPARE( QgsPointXY( 0, -0.000001 ).toDegreesMinutesSeconds( 5 ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00360" ) + QChar( 0x2033 ) + QStringLiteral( "E" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( 0.000001, 0 ).toDegreesMinutesSeconds( 5 ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00360" ) + QChar( 0x2033 ) + QStringLiteral( "W" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( -0.000001, 0 ).toDegreesMinutesSeconds( 5 ), myControlString );

  //test rounding does not create seconds >= 60
  myControlString = QStringLiteral( "100" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) + QStringLiteral( "E" ) +
                    QStringLiteral( ",90" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 ) + QStringLiteral( "N" );
  QCOMPARE( QgsPointXY( 99.999999, 89.999999 ).toDegreesMinutesSeconds( 2 ), myControlString );

  //should be no directional suffixes for 180 degree longitudes
  myControlString = QStringLiteral( "180" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) +
                    QChar( 0x2033 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( 180, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  //should also be no directional suffix for 180 degree longitudes within specified precision
  QCOMPARE( QgsPointXY( 180.000001, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  QCOMPARE( QgsPointXY( 179.999999, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QStringLiteral( "179" ) + QChar( 176 ) +
                    QStringLiteral( "59" ) + QChar( 0x2032 ) + QStringLiteral( "59.99640" ) + QChar( 0x2033 ) + QStringLiteral( "W" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( 180.000001, 0 ).toDegreesMinutesSeconds( 5 ), myControlString );
  myControlString = QStringLiteral( "179" ) + QChar( 176 ) +
                    QStringLiteral( "59" ) + QChar( 0x2032 ) + QStringLiteral( "59.99640" ) + QChar( 0x2033 ) + QStringLiteral( "E" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( 179.999999, 0 ).toDegreesMinutesSeconds( 5 ), myControlString );
}

void TestQgsPointXY::toDegreesMinutesSecondsNoSuffix()
{
  QString myControlString = QStringLiteral( "80" ) + QChar( 176 ) +
                            QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) +
                            QChar( 0x2033 ) +
                            QStringLiteral( ",20" ) + QChar( 176 ) +
                            QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( mPoint4.toDegreesMinutesSeconds( 2, false ), myControlString );

  //test 0 lat/long
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) +
                    QChar( 0x2033 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00" ) + QChar( 0x2033 );
  QVERIFY( QgsPointXY( 0, 0 ).toDegreesMinutesSeconds( 2, false ) == myControlString );
  //test near zero lat/long
  QCOMPARE( QgsPointXY( 0, 0.000001 ).toDegreesMinutesSeconds( 2, false ), myControlString );
  QCOMPARE( QgsPointXY( 0.000001, 0 ).toDegreesMinutesSeconds( 2, false ), myControlString );
  //should be no "-" prefix for near-zero lat/long when rounding to 2 decimal places
  QCOMPARE( QgsPointXY( 0, -0.000001 ).toDegreesMinutesSeconds( 2, false ), myControlString );
  QCOMPARE( QgsPointXY( -0.000001, 0 ).toDegreesMinutesSeconds( 2, false ), myControlString );

  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00000" ) +
                    QChar( 0x2033 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00360" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( 0, 0.000001 ).toDegreesMinutesSeconds( 5, false ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00000" ) +
                    QChar( 0x2033 ) +
                    QStringLiteral( ",-0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00360" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( 0, -0.000001 ).toDegreesMinutesSeconds( 5, false ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00360" ) +
                    QChar( 0x2033 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( 0.000001, 0 ).toDegreesMinutesSeconds( 5, false ), myControlString );
  myControlString = QStringLiteral( "-0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00360" ) +
                    QChar( 0x2033 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0" ) + QChar( 0x2032 ) + QStringLiteral( "0.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( -0.000001, 0 ).toDegreesMinutesSeconds( 5, false ), myControlString );
}

void TestQgsPointXY::toDegreesMinutesSecondsPadded()
{
  QString myControlString = QStringLiteral( "80" ) + QChar( 176 ) +
                            QStringLiteral( "00" ) + QChar( 0x2032 ) + QStringLiteral( "00.00" ) +
                            QChar( 0x2033 ) +
                            QStringLiteral( "E,20" ) + QChar( 176 ) +
                            QStringLiteral( "00" ) + QChar( 0x2032 ) + QStringLiteral( "00.00" ) + QChar( 0x2033 ) +
                            QStringLiteral( "N" );
  qDebug() << myControlString;
  QCOMPARE( mPoint4.toDegreesMinutesSeconds( 2, true, true ), myControlString );

  //should be no directional suffixes for 0 degree coordinates
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "00" ) + QChar( 0x2032 ) + QStringLiteral( "00.00" ) +
                    QChar( 0x2033 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "00" ) + QChar( 0x2032 ) + QStringLiteral( "00.00" ) + QChar( 0x2033 );
  QVERIFY( QgsPointXY( 0, 0 ).toDegreesMinutesSeconds( 2, true, true ) == myControlString );
  //should also be no directional suffix for 0 degree coordinates within specified precision
  QCOMPARE( QgsPointXY( 0, 0.000001 ).toDegreesMinutesSeconds( 2, true, true ), myControlString );
  QCOMPARE( QgsPointXY( 0, -0.000001 ).toDegreesMinutesSeconds( 2, true, true ), myControlString );
  QCOMPARE( QgsPointXY( -0.000001, 0 ).toDegreesMinutesSeconds( 2, true, true ), myControlString );
  QCOMPARE( QgsPointXY( 0.000001, 0 ).toDegreesMinutesSeconds( 2, true, true ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "00" ) + QChar( 0x2032 ) + QStringLiteral( "00.00000" ) +
                    QChar( 0x2033 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "00" ) + QChar( 0x2032 ) + QStringLiteral( "00.00360" ) + QChar( 0x2033 ) + QStringLiteral( "N" );
  QCOMPARE( QgsPointXY( 0, 0.000001 ).toDegreesMinutesSeconds( 5, true, true ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "00" ) + QChar( 0x2032 ) + QStringLiteral( "00.00000" ) +
                    QChar( 0x2033 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "00" ) + QChar( 0x2032 ) + QStringLiteral( "00.00360" ) + QChar( 0x2033 ) + QStringLiteral( "S" );
  QCOMPARE( QgsPointXY( 0, -0.000001 ).toDegreesMinutesSeconds( 5, true, true ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "00" ) + QChar( 0x2032 ) + QStringLiteral( "00.00360" ) + QChar( 0x2033 ) + QStringLiteral( "E" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "00" ) + QChar( 0x2032 ) + QStringLiteral( "00.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( 0.000001, 0 ).toDegreesMinutesSeconds( 5, true, true ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "00" ) + QChar( 0x2032 ) + QStringLiteral( "00.00360" ) + QChar( 0x2033 ) + QStringLiteral( "W" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "00" ) + QChar( 0x2032 ) + QStringLiteral( "00.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPointXY( -0.000001, 0 ).toDegreesMinutesSeconds( 5, true, true ), myControlString );
}

void TestQgsPointXY::toDegreesMinutes()
{
  mReport += QLatin1String( "<p>Testing toDegreesMinutes()</p>" );
  mReport += "<p>" + mPoint1.toDegreesMinutes( 2 )  +  "</p>";
  mReport += "<p>" + mPoint2.toDegreesMinutes( 2 )  +  "</p>";
  mReport += "<p>" + mPoint3.toDegreesMinutes( 2 )  +  "</p>";
  mReport += "<p>" + mPoint4.toDegreesMinutes( 2 )  +  "</p>";

  qDebug() << mPoint4.toDegreesMinutes( 2 );
  QString myControlString = QStringLiteral( "80" ) + QChar( 176 ) +
                            QStringLiteral( "0.00" ) + QChar( 0x2032 ) +
                            QStringLiteral( "E,20" ) + QChar( 176 ) +
                            QStringLiteral( "0.00" ) + QChar( 0x2032 ) + QStringLiteral( "N" );
  qDebug() << myControlString;
  QCOMPARE( mPoint4.toDegreesMinutes( 2 ), myControlString );

  //check if longitudes > 180 or <-180 wrap around
  myControlString = QStringLiteral( "10" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 ) + QStringLiteral( "E" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( 370, 0 ).toDegreesMinutes( 2 ), myControlString );
  myControlString = QStringLiteral( "10" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 ) + QStringLiteral( "W" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( -370, 0 ).toDegreesMinutes( 2 ), myControlString );
  myControlString = QStringLiteral( "179" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 ) + QStringLiteral( "W" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( 181, 0 ).toDegreesMinutes( 2 ), myControlString );
  myControlString = QStringLiteral( "179" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 ) + QStringLiteral( "E" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( -181, 0 ).toDegreesMinutes( 2 ), myControlString );
  myControlString = QStringLiteral( "1" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 ) + QStringLiteral( "W" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( 359, 0 ).toDegreesMinutes( 2 ), myControlString );
  myControlString = QStringLiteral( "1" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 ) + QStringLiteral( "E" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( -359, 0 ).toDegreesMinutes( 2 ), myControlString );

  //should be no directional suffixes for 0 degree coordinates
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 );
  QVERIFY( QgsPointXY( 0, 0 ).toDegreesMinutes( 2 ) == myControlString );
  //should also be no directional suffix for 0 degree coordinates within specified precision
  QCOMPARE( QgsPointXY( 0, 0.000001 ).toDegreesMinutes( 2 ), myControlString );
  QCOMPARE( QgsPointXY( 0, -0.000001 ).toDegreesMinutes( 2 ), myControlString );
  QCOMPARE( QgsPointXY( -0.000001, 0 ).toDegreesMinutes( 2 ), myControlString );
  QCOMPARE( QgsPointXY( 0.000001, 0 ).toDegreesMinutes( 2 ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00000" ) + QChar( 0x2032 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00006" ) + QChar( 0x2032 ) + QStringLiteral( "N" );
  QCOMPARE( QgsPointXY( 0, 0.000001 ).toDegreesMinutes( 5 ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00000" ) + QChar( 0x2032 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00006" ) + QChar( 0x2032 ) + QStringLiteral( "S" );
  QCOMPARE( QgsPointXY( 0, -0.000001 ).toDegreesMinutes( 5 ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00006" ) + QChar( 0x2032 ) + QStringLiteral( "E" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( 0.000001, 0 ).toDegreesMinutes( 5 ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00006" ) + QChar( 0x2032 ) + QStringLiteral( "W" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( -0.000001, 0 ).toDegreesMinutes( 5 ), myControlString );

  //test rounding does not create minutes >= 60
  myControlString = QStringLiteral( "100" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 ) + QStringLiteral( "E" ) +
                    QStringLiteral( ",100" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 ) + QStringLiteral( "N" );
  QCOMPARE( QgsPointXY( 99.999999, 99.999999 ).toDegreesMinutes( 2 ), myControlString );

  //should be no directional suffixes for 180 degree longitudes
  myControlString = QStringLiteral( "180" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( 180, 0 ).toDegreesMinutes( 2 ), myControlString );
  //should also be no directional suffix for 180 degree longitudes within specified precision
  QCOMPARE( QgsPointXY( 180.000001, 0 ).toDegreesMinutes( 2 ), myControlString );
  QCOMPARE( QgsPointXY( 179.999999, 0 ).toDegreesMinutes( 2 ), myControlString );
  myControlString = QStringLiteral( "179" ) + QChar( 176 ) +
                    QStringLiteral( "59.99994" ) + QChar( 0x2032 ) + QStringLiteral( "W" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( 180.000001, 0 ).toDegreesMinutes( 5 ), myControlString );
  myControlString = QStringLiteral( "179" ) + QChar( 176 ) +
                    QStringLiteral( "59.99994" ) + QChar( 0x2032 ) + QStringLiteral( "E" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( 179.999999, 0 ).toDegreesMinutes( 5 ), myControlString );
}

void TestQgsPointXY::toDegreesMinutesNoSuffix()
{
  QString myControlString = QStringLiteral( "80" ) + QChar( 176 ) +
                            QStringLiteral( "0.00" ) + QChar( 0x2032 ) +
                            QStringLiteral( ",20" ) + QChar( 176 ) +
                            QStringLiteral( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( mPoint4.toDegreesMinutes( 2, false ), myControlString );

  //test 0 lat/long
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00" ) + QChar( 0x2032 );
  QVERIFY( QgsPointXY( 0, 0 ).toDegreesMinutes( 2, false ) == myControlString );
  //test near zero lat/long
  QCOMPARE( QgsPointXY( 0, 0.000001 ).toDegreesMinutes( 2, false ), myControlString );
  QCOMPARE( QgsPointXY( 0.000001, 0 ).toDegreesMinutes( 2, false ), myControlString );
  //should be no "-" prefix for near-zero lat/long when rounding to 2 decimal places
  QCOMPARE( QgsPointXY( 0, -0.000001 ).toDegreesMinutes( 2, false ), myControlString );
  QCOMPARE( QgsPointXY( -0.000001, 0 ).toDegreesMinutes( 2, false ), myControlString );

  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00000" ) + QChar( 0x2032 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00006" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( 0, 0.000001 ).toDegreesMinutes( 5, false ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00000" ) + QChar( 0x2032 ) +
                    QStringLiteral( ",-0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00006" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( 0, -0.000001 ).toDegreesMinutes( 5, false ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00006" ) + QChar( 0x2032 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( 0.000001, 0 ).toDegreesMinutes( 5, false ), myControlString );
  myControlString = QStringLiteral( "-0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00006" ) + QChar( 0x2032 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "0.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( -0.000001, 0 ).toDegreesMinutes( 5, false ), myControlString );
}

void TestQgsPointXY::toDegreesMinutesPadded()
{
  QString myControlString = QStringLiteral( "80" ) + QChar( 176 ) +
                            QStringLiteral( "00.00" ) + QChar( 0x2032 ) +
                            QStringLiteral( "E,20" ) + QChar( 176 ) +
                            QStringLiteral( "00.00" ) + QChar( 0x2032 ) + QStringLiteral( "N" );
  qDebug() << myControlString;
  QCOMPARE( mPoint4.toDegreesMinutes( 2, true, true ), myControlString );

  //should be no directional suffixes for 0 degree coordinates
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "00.00" ) + QChar( 0x2032 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "00.00" ) + QChar( 0x2032 );
  QVERIFY( QgsPointXY( 0, 0 ).toDegreesMinutes( 2, true, true ) == myControlString );
  //should also be no directional suffix for 0 degree coordinates within specified precision
  QCOMPARE( QgsPointXY( 0, 0.000001 ).toDegreesMinutes( 2, true, true ), myControlString );
  QCOMPARE( QgsPointXY( 0, -0.000001 ).toDegreesMinutes( 2, true, true ), myControlString );
  QCOMPARE( QgsPointXY( -0.000001, 0 ).toDegreesMinutes( 2, true, true ), myControlString );
  QCOMPARE( QgsPointXY( 0.000001, 0 ).toDegreesMinutes( 2, true, true ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "00.00000" ) + QChar( 0x2032 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "00.00006" ) + QChar( 0x2032 ) + QStringLiteral( "N" );
  QCOMPARE( QgsPointXY( 0, 0.000001 ).toDegreesMinutes( 5, true, true ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "00.00000" ) + QChar( 0x2032 ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "00.00006" ) + QChar( 0x2032 ) + QStringLiteral( "S" );
  QCOMPARE( QgsPointXY( 0, -0.000001 ).toDegreesMinutes( 5, true, true ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "00.00006" ) + QChar( 0x2032 ) + QStringLiteral( "E" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "00.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( 0.000001, 0 ).toDegreesMinutes( 5, true, true ), myControlString );
  myControlString = QStringLiteral( "0" ) + QChar( 176 ) +
                    QStringLiteral( "00.00006" ) + QChar( 0x2032 ) + QStringLiteral( "W" ) +
                    QStringLiteral( ",0" ) + QChar( 176 ) +
                    QStringLiteral( "00.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPointXY( -0.000001, 0 ).toDegreesMinutes( 5, true, true ), myControlString );
}

void TestQgsPointXY::sqrDist()
{
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( QgsPointXY( 2, 2 ) ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( 2, 2 ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( QgsPointXY( 3, 2 ) ), 4.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( 3, 2 ), 4.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( QgsPointXY( 1, 3 ) ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( 1, 3 ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( QgsPointXY( 1, 4 ) ), 4.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).sqrDist( 1, 4 ), 4.0 );
  QCOMPARE( QgsPointXY( 1, -2 ).sqrDist( QgsPointXY( 1, -4 ) ), 4.0 );
  QCOMPARE( QgsPointXY( 1, -2 ).sqrDist( 1, -4 ), 4.0 );
}

void TestQgsPointXY::distance()
{
  QCOMPARE( QgsPointXY( 1, 2 ).distance( QgsPointXY( 2, 2 ) ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).distance( 2, 2 ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).distance( QgsPointXY( 3, 2 ) ), 2.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).distance( 3, 2 ), 2.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).distance( QgsPointXY( 1, 3 ) ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).distance( 1, 3 ), 1.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).distance( QgsPointXY( 1, 4 ) ), 2.0 );
  QCOMPARE( QgsPointXY( 1, 2 ).distance( 1, 4 ), 2.0 );
  QCOMPARE( QgsPointXY( 1, -2 ).distance( QgsPointXY( 1, -4 ) ), 2.0 );
  QCOMPARE( QgsPointXY( 1, -2 ).distance( 1, -4 ), 2.0 );
}

void TestQgsPointXY::compare()
{
  QgsPointXY point1( 5.000000000001, 9.0 );
  QgsPointXY point2( 5.0, 8.999999999999999 );
  QVERIFY( point1.compare( point2, 0.00000001 ) );
  QgsPointXY point3( 5.0, 6.0 );
  QVERIFY( !( point3.compare( point1 ) ) );
  QgsPointXY point4( 10 / 3.0, 12 / 7.0 );
  QVERIFY( point4.compare( QgsPointXY( 10 / 3.0, 12 / 7.0 ) ) );
}

void TestQgsPointXY::project()
{
  // test projecting a point
  QgsPointXY p( 1, 2 );
  QVERIFY( p.project( 1, 0 ).compare( QgsPointXY( 1, 3 ), 0.0000000001 ) );
  QVERIFY( p.project( 1.5, 90 ).compare( QgsPointXY( 2.5, 2 ), 0.0000000001 ) );
  QVERIFY( p.project( 2, 180 ).compare( QgsPointXY( 1, 0 ), 0.0000000001 ) );
  QVERIFY( p.project( 5, 270 ).compare( QgsPointXY( -4, 2 ), 0.0000000001 ) );
  QVERIFY( p.project( 6, 360 ).compare( QgsPointXY( 1, 8 ), 0.0000000001 ) );
  QVERIFY( p.project( 5, 450 ).compare( QgsPointXY( 6, 2 ), 0.0000000001 ) );
  QVERIFY( p.project( -1, 0 ).compare( QgsPointXY( 1, 1 ), 0.0000000001 ) );
  QVERIFY( p.project( 1.5, -90 ).compare( QgsPointXY( -0.5, 2 ), 0.0000000001 ) );
}

void TestQgsPointXY::vector()
{
  //equality
  QVERIFY( QgsVector( 1, 2 ) == QgsVector( 1, 2 ) );
  QVERIFY( QgsVector( 1, 2 ) != QgsVector( 3, 2 ) );
  QVERIFY( QgsVector( 1, 2 ) != QgsVector( 1, 3 ) );

  //test constructors, x(), y() accessors
  QgsVector v1;
  QCOMPARE( v1.x(), 0.0 );
  QCOMPARE( v1.y(), 0.0 );
  QgsVector v2( 1.0, 2.0 );
  QCOMPARE( v2.x(), 1.0 );
  QCOMPARE( v2.y(), 2.0 );

  // operator-
  QCOMPARE( ( -v2 ).x(), -1.0 );
  QCOMPARE( ( -v2 ).y(), -2.0 );

  // operator*
  QCOMPARE( ( v2 * 2.0 ).x(), 2.0 );
  QCOMPARE( ( v2 * 2.0 ).y(), 4.0 );

  // operator/
  QCOMPARE( ( v2 / 2.0 ).x(), 0.5 );
  QCOMPARE( ( v2 / 2.0 ).y(), 1.0 );

  // QgsVector * QgsVector
  QCOMPARE( ( v2 * v2 ), 5.0 );

  // length
  QCOMPARE( v1.length(), 0.0 );
  QVERIFY( qgsDoubleNear( v2.length(), std::sqrt( 5.0 ), 0.000000001 ) );

  // perpVector
  QCOMPARE( QgsVector( 2, 3 ).perpVector().x(), -3.0 );
  QCOMPARE( QgsVector( 2, 3 ).perpVector().y(), 2.0 );

  // angle
  QVERIFY( qgsDoubleNear( QgsVector( 0, 1 ).angle(), M_PI_2, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( QgsVector( 1, 0 ).angle(), 0, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( QgsVector( -1, 0 ).angle(), M_PI, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( QgsVector( 0, -1 ).angle(), 3 * M_PI_2, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( QgsVector( 0, 0 ).angle(), 0, 0.0000001 ) );

  QVERIFY( qgsDoubleNear( QgsVector( 0, 1 ).angle( QgsVector( 0, 1 ) ), 0, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( QgsVector( 1, 0 ).angle( QgsVector( 0, 1 ) ), M_PI_2, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( QgsVector( 0, 1 ).angle( QgsVector( -1, 0 ) ), M_PI_2, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( QgsVector( 1, 0 ).angle( QgsVector( -1, 0 ) ), M_PI, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( QgsVector( -1, 0 ).angle( QgsVector( 0, 0 ) ), -M_PI, 0.0000001 ) );

  // rotateBy
  QVERIFY( qgsDoubleNear( QgsVector( 0, 1 ).rotateBy( M_PI_2 ).x(), -1.0, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( QgsVector( 0, 1 ).rotateBy( M_PI_2 ).y(), 0.0, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( QgsVector( 0, 1 ).rotateBy( M_PI ).x(), 0.0, 0.0000001 ) );
  QVERIFY( qgsDoubleNear( QgsVector( 0, 1 ).rotateBy( M_PI ).y(), -1.0, 0.0000001 ) );

  // normalized
  QCOMPARE( QgsVector( 0, 2 ).normalized().x(), 0.0 );
  QCOMPARE( QgsVector( 0, 2 ).normalized().y(), 1.0 );
  QCOMPARE( QgsVector( 2, 0 ).normalized().x(), 1.0 );
  QCOMPARE( QgsVector( 2, 0 ).normalized().y(), 0.0 );

  // operator +, -
  v1 = QgsVector( 1, 3 );
  v2 = QgsVector( 2, 5 );
  QgsVector v3 = v1 + v2;
  QCOMPARE( v3.x(), 3.0 );
  QCOMPARE( v3.y(), 8.0 );
  v3 = v1 - v2;
  QCOMPARE( v3.x(), -1.0 );
  QCOMPARE( v3.y(), -2.0 );
  // operator +=, -=
  v1 += v2;
  QCOMPARE( v1.x(), 3.0 );
  QCOMPARE( v1.y(), 8.0 );
  v1 -= v2;
  QCOMPARE( v1.x(), 1.0 );
  QCOMPARE( v1.y(), 3.0 );
}

QGSTEST_MAIN( TestQgsPointXY )
#include "testqgspoint.moc"
