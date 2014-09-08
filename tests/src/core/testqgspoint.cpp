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
#include <QtTest>
#include <QObject>
#include <QString>
#include <QObject>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

#include <iostream>
//qgis includes...
#include <qgsapplication.h>
#include <qgsgeometry.h>
//header for class being tested
#include <qgspoint.h>

class TestQgsPoint: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void toString();
    void toDegreesMinutesSeconds();
    void toDegreesMinutesSecondsNoSuffix();
    void toDegreesMinutesSecondsPadded();
    void toDegreesMinutes();
    void toDegreesMinutesNoSuffix();
    void toDegreesMinutesPadded();
    void wellKnownText();
    void sqrDist();
    void multiply();
    void onSegment();
  private:
    QgsPoint mPoint1;
    QgsPoint mPoint2;
    QgsPoint mPoint3;
    QgsPoint mPoint4;
    QString mReport;
};

void TestQgsPoint::init()
{
  //
  // Reset / reinitialise the geometries before each test is run
  //
  mPoint1 = QgsPoint( 20.0, -20.0 );
  mPoint2 = QgsPoint( -80.0, 20.0 );
  mPoint3 = QgsPoint( -80.0, -20.0 );
  mPoint4 = QgsPoint( 80.0, 20.0 );
}

void TestQgsPoint::cleanup()
{
  // will be called after every testfunction.
}

void TestQgsPoint::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::showSettings();
  mReport += "<h1>Point Tests</h1>\n";
}


void TestQgsPoint::cleanupTestCase()
{
  //
  // Runs once after all tests are run
  //
  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
    //QDesktopServices::openUrl( "file:///" + myReportFile );
  }

}

void TestQgsPoint::toString()
{
  mReport += "<p>Testing toString()</p>";
  mReport += "<p>" + mPoint1.toString( 2 )  +  "</p>";
  mReport += "<p>" + mPoint2.toString( 2 )  +  "</p>";
  mReport += "<p>" + mPoint3.toString( 2 )  +  "</p>";
  mReport += "<p>" + mPoint4.toString( 2 )  +  "</p>";
  QCOMPARE( mPoint1.toString( 2 ), QString( "20.00,-20.00" ) );
}

void TestQgsPoint::toDegreesMinutesSeconds()
{
  mReport += "<p>Testing toDegreesMinutesSeconds()</p>";
  mReport += "<p>" + mPoint1.toDegreesMinutesSeconds( 2 )  +  "</p>";
  mReport += "<p>" + mPoint2.toDegreesMinutesSeconds( 2 )  +  "</p>";
  mReport += "<p>" + mPoint3.toDegreesMinutesSeconds( 2 )  +  "</p>";
  mReport += "<p>" + mPoint4.toDegreesMinutesSeconds( 2 )  +  "</p>";

  qDebug() << mPoint4.toDegreesMinutesSeconds( 2 );
  QString myControlString = QString( "80" ) + QChar( 176 ) +
                            QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) +
                            QChar( 0x2033 ) +
                            QString( "E,20" ) + QChar( 176 ) +
                            QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 ) +
                            QString( "N" );
  qDebug() << myControlString;
  QCOMPARE( mPoint4.toDegreesMinutesSeconds( 2 ), myControlString );

  //check if longitudes > 180 or <-180 wrap around
  myControlString = QString( "10" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 ) + QString( "E" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( 370, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QString( "10" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 ) + QString( "W" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( -370, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QString( "179" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 ) + QString( "W" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( 181, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QString( "179" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 ) + QString( "E" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( -181, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QString( "1" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 ) + QString( "W" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( 359, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QString( "1" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 ) + QString( "E" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( -359, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );

  //should be no directional suffixes for 0 degree coordinates
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) +
                    QChar( 0x2033 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( 0, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  //should also be no directional suffix for 0 degree coordinates within specified precision
  QCOMPARE( QgsPoint( 0, 0.000001 ).toDegreesMinutesSeconds( 2 ), myControlString );
  QCOMPARE( QgsPoint( 0, -0.000001 ).toDegreesMinutesSeconds( 2 ), myControlString );
  QCOMPARE( QgsPoint( -0.000001, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  QCOMPARE( QgsPoint( 0.000001, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00000" ) +
                    QChar( 0x2033 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00360" ) + QChar( 0x2033 ) + QString( "N" );
  QCOMPARE( QgsPoint( 0, 0.000001 ).toDegreesMinutesSeconds( 5 ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00000" ) +
                    QChar( 0x2033 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00360" ) + QChar( 0x2033 ) + QString( "S" );
  QCOMPARE( QgsPoint( 0, -0.000001 ).toDegreesMinutesSeconds( 5 ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00360" ) + QChar( 0x2033 ) + QString( "E" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( 0.000001, 0 ).toDegreesMinutesSeconds( 5 ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00360" ) + QChar( 0x2033 ) + QString( "W" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( -0.000001, 0 ).toDegreesMinutesSeconds( 5 ), myControlString );

  //test rounding does not create seconds >= 60
  myControlString = QString( "100" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 ) + QString( "E" ) +
                    QString( ",100" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 ) + QString( "N" );
  QCOMPARE( QgsPoint( 99.999999, 99.999999 ).toDegreesMinutesSeconds( 2 ), myControlString );

  //should be no directional suffixes for 180 degree longitudes
  myControlString = QString( "180" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) +
                    QChar( 0x2033 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( 180, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  //should also be no directional suffix for 180 degree longitudes within specified precision
  QCOMPARE( QgsPoint( 180.000001, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  QCOMPARE( QgsPoint( 179.999999, 0 ).toDegreesMinutesSeconds( 2 ), myControlString );
  myControlString = QString( "179" ) + QChar( 176 ) +
                    QString( "59" ) + QChar( 0x2032 ) + QString( "59.99640" ) + QChar( 0x2033 ) + QString( "W" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( 180.000001, 0 ).toDegreesMinutesSeconds( 5 ), myControlString );
  myControlString = QString( "179" ) + QChar( 176 ) +
                    QString( "59" ) + QChar( 0x2032 ) + QString( "59.99640" ) + QChar( 0x2033 ) + QString( "E" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( 179.999999, 0 ).toDegreesMinutesSeconds( 5 ), myControlString );
}

void TestQgsPoint::toDegreesMinutesSecondsNoSuffix()
{
  QString myControlString = QString( "80" ) + QChar( 176 ) +
                            QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) +
                            QChar( 0x2033 ) +
                            QString( ",20" ) + QChar( 176 ) +
                            QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 );
  QCOMPARE( mPoint4.toDegreesMinutesSeconds( 2, false ), myControlString );

  //test 0 lat/long
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) +
                    QChar( 0x2033 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00" ) + QChar( 0x2033 );
  QVERIFY( QgsPoint( 0, 0 ).toDegreesMinutesSeconds( 2, false ) == myControlString );
  //test near zero lat/long
  QCOMPARE( QgsPoint( 0, 0.000001 ).toDegreesMinutesSeconds( 2, false ), myControlString );
  QCOMPARE( QgsPoint( 0.000001, 0 ).toDegreesMinutesSeconds( 2, false ), myControlString );
  //should be no "-" prefix for near-zero lat/long when rounding to 2 decimal places
  QCOMPARE( QgsPoint( 0, -0.000001 ).toDegreesMinutesSeconds( 2, false ), myControlString );
  QCOMPARE( QgsPoint( -0.000001, 0 ).toDegreesMinutesSeconds( 2, false ), myControlString );

  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00000" ) +
                    QChar( 0x2033 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00360" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( 0, 0.000001 ).toDegreesMinutesSeconds( 5, false ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00000" ) +
                    QChar( 0x2033 ) +
                    QString( ",-0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00360" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( 0, -0.000001 ).toDegreesMinutesSeconds( 5, false ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00360" ) +
                    QChar( 0x2033 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( 0.000001, 0 ).toDegreesMinutesSeconds( 5, false ), myControlString );
  myControlString = QString( "-0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00360" ) +
                    QChar( 0x2033 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0" ) + QChar( 0x2032 ) + QString( "0.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( -0.000001, 0 ).toDegreesMinutesSeconds( 5, false ), myControlString );
}

void TestQgsPoint::toDegreesMinutesSecondsPadded()
{
  QString myControlString = QString( "80" ) + QChar( 176 ) +
                            QString( "00" ) + QChar( 0x2032 ) + QString( "00.00" ) +
                            QChar( 0x2033 ) +
                            QString( "E,20" ) + QChar( 176 ) +
                            QString( "00" ) + QChar( 0x2032 ) + QString( "00.00" ) + QChar( 0x2033 ) +
                            QString( "N" );
  qDebug() << myControlString;
  QCOMPARE( mPoint4.toDegreesMinutesSeconds( 2, true, true ), myControlString );

  //should be no directional suffixes for 0 degree coordinates
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "00" ) + QChar( 0x2032 ) + QString( "00.00" ) +
                    QChar( 0x2033 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "00" ) + QChar( 0x2032 ) + QString( "00.00" ) + QChar( 0x2033 );
  QVERIFY( QgsPoint( 0, 0 ).toDegreesMinutesSeconds( 2, true, true ) == myControlString );
  //should also be no directional suffix for 0 degree coordinates within specified precision
  QCOMPARE( QgsPoint( 0, 0.000001 ).toDegreesMinutesSeconds( 2, true, true ), myControlString );
  QCOMPARE( QgsPoint( 0, -0.000001 ).toDegreesMinutesSeconds( 2, true, true ), myControlString );
  QCOMPARE( QgsPoint( -0.000001, 0 ).toDegreesMinutesSeconds( 2, true, true ), myControlString );
  QCOMPARE( QgsPoint( 0.000001, 0 ).toDegreesMinutesSeconds( 2, true, true ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "00" ) + QChar( 0x2032 ) + QString( "00.00000" ) +
                    QChar( 0x2033 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "00" ) + QChar( 0x2032 ) + QString( "00.00360" ) + QChar( 0x2033 ) + QString( "N" );
  QCOMPARE( QgsPoint( 0, 0.000001 ).toDegreesMinutesSeconds( 5, true, true ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "00" ) + QChar( 0x2032 ) + QString( "00.00000" ) +
                    QChar( 0x2033 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "00" ) + QChar( 0x2032 ) + QString( "00.00360" ) + QChar( 0x2033 ) + QString( "S" );
  QCOMPARE( QgsPoint( 0, -0.000001 ).toDegreesMinutesSeconds( 5, true, true ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "00" ) + QChar( 0x2032 ) + QString( "00.00360" ) + QChar( 0x2033 ) + QString( "E" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "00" ) + QChar( 0x2032 ) + QString( "00.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( 0.000001, 0 ).toDegreesMinutesSeconds( 5, true, true ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "00" ) + QChar( 0x2032 ) + QString( "00.00360" ) + QChar( 0x2033 ) + QString( "W" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "00" ) + QChar( 0x2032 ) + QString( "00.00000" ) + QChar( 0x2033 );
  QCOMPARE( QgsPoint( -0.000001, 0 ).toDegreesMinutesSeconds( 5, true, true ), myControlString );
}

void TestQgsPoint::toDegreesMinutes()
{
  mReport += "<p>Testing toDegreesMinutes()</p>";
  mReport += "<p>" + mPoint1.toDegreesMinutes( 2 )  +  "</p>";
  mReport += "<p>" + mPoint2.toDegreesMinutes( 2 )  +  "</p>";
  mReport += "<p>" + mPoint3.toDegreesMinutes( 2 )  +  "</p>";
  mReport += "<p>" + mPoint4.toDegreesMinutes( 2 )  +  "</p>";

  qDebug() << mPoint4.toDegreesMinutes( 2 );
  QString myControlString = QString( "80" ) + QChar( 176 ) +
                            QString( "0.00" ) + QChar( 0x2032 ) +
                            QString( "E,20" ) + QChar( 176 ) +
                            QString( "0.00" ) + QChar( 0x2032 ) + QString( "N" );
  qDebug() << myControlString;
  QCOMPARE( mPoint4.toDegreesMinutes( 2 ), myControlString );

  //check if longitudes > 180 or <-180 wrap around
  myControlString = QString( "10" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 ) + QString( "E" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( 370, 0 ).toDegreesMinutes( 2 ), myControlString );
  myControlString = QString( "10" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 ) + QString( "W" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( -370, 0 ).toDegreesMinutes( 2 ), myControlString );
  myControlString = QString( "179" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 ) + QString( "W" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( 181, 0 ).toDegreesMinutes( 2 ), myControlString );
  myControlString = QString( "179" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 ) + QString( "E" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( -181, 0 ).toDegreesMinutes( 2 ), myControlString );
  myControlString = QString( "1" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 ) + QString( "W" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( 359, 0 ).toDegreesMinutes( 2 ), myControlString );
  myControlString = QString( "1" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 ) + QString( "E" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( -359, 0 ).toDegreesMinutes( 2 ), myControlString );

  //should be no directional suffixes for 0 degree coordinates
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 );
  QVERIFY( QgsPoint( 0, 0 ).toDegreesMinutes( 2 ) == myControlString );
  //should also be no directional suffix for 0 degree coordinates within specified precision
  QCOMPARE( QgsPoint( 0, 0.000001 ).toDegreesMinutes( 2 ), myControlString );
  QCOMPARE( QgsPoint( 0, -0.000001 ).toDegreesMinutes( 2 ), myControlString );
  QCOMPARE( QgsPoint( -0.000001, 0 ).toDegreesMinutes( 2 ), myControlString );
  QCOMPARE( QgsPoint( 0.000001, 0 ).toDegreesMinutes( 2 ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0.00000" ) + QChar( 0x2032 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00006" ) + QChar( 0x2032 ) + QString( "N" );
  QCOMPARE( QgsPoint( 0, 0.000001 ).toDegreesMinutes( 5 ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0.00000" ) + QChar( 0x2032 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00006" ) + QChar( 0x2032 ) + QString( "S" );
  QCOMPARE( QgsPoint( 0, -0.000001 ).toDegreesMinutes( 5 ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0.00006" ) + QChar( 0x2032 ) + QString( "E" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( 0.000001, 0 ).toDegreesMinutes( 5 ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0.00006" ) + QChar( 0x2032 ) + QString( "W" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( -0.000001, 0 ).toDegreesMinutes( 5 ), myControlString );

  //test rounding does not create minutes >= 60
  myControlString = QString( "100" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 ) + QString( "E" ) +
                    QString( ",100" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 ) + QString( "N" );
  QCOMPARE( QgsPoint( 99.999999, 99.999999 ).toDegreesMinutes( 2 ), myControlString );

  //should be no directional suffixes for 180 degree longitudes
  myControlString = QString( "180" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( 180, 0 ).toDegreesMinutes( 2 ), myControlString );
  //should also be no directional suffix for 180 degree longitudes within specified precision
  QCOMPARE( QgsPoint( 180.000001, 0 ).toDegreesMinutes( 2 ), myControlString );
  QCOMPARE( QgsPoint( 179.999999, 0 ).toDegreesMinutes( 2 ), myControlString );
  myControlString = QString( "179" ) + QChar( 176 ) +
                    QString( "59.99994" ) + QChar( 0x2032 ) + QString( "W" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( 180.000001, 0 ).toDegreesMinutes( 5 ), myControlString );
  myControlString = QString( "179" ) + QChar( 176 ) +
                    QString( "59.99994" ) + QChar( 0x2032 ) + QString( "E" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( 179.999999, 0 ).toDegreesMinutes( 5 ), myControlString );
}

void TestQgsPoint::toDegreesMinutesNoSuffix()
{
  QString myControlString = QString( "80" ) + QChar( 176 ) +
                            QString( "0.00" ) + QChar( 0x2032 ) +
                            QString( ",20" ) + QChar( 176 ) +
                            QString( "0.00" ) + QChar( 0x2032 );
  QCOMPARE( mPoint4.toDegreesMinutes( 2, false ), myControlString );

  //test 0 lat/long
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00" ) + QChar( 0x2032 );
  QVERIFY( QgsPoint( 0, 0 ).toDegreesMinutes( 2, false ) == myControlString );
  //test near zero lat/long
  QCOMPARE( QgsPoint( 0, 0.000001 ).toDegreesMinutes( 2, false ), myControlString );
  QCOMPARE( QgsPoint( 0.000001, 0 ).toDegreesMinutes( 2, false ), myControlString );
  //should be no "-" prefix for near-zero lat/long when rounding to 2 decimal places
  QCOMPARE( QgsPoint( 0, -0.000001 ).toDegreesMinutes( 2, false ), myControlString );
  QCOMPARE( QgsPoint( -0.000001, 0 ).toDegreesMinutes( 2, false ), myControlString );

  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0.00000" ) + QChar( 0x2032 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00006" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( 0, 0.000001 ).toDegreesMinutes( 5, false ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0.00000" ) + QChar( 0x2032 ) +
                    QString( ",-0" ) + QChar( 176 ) +
                    QString( "0.00006" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( 0, -0.000001 ).toDegreesMinutes( 5, false ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "0.00006" ) + QChar( 0x2032 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( 0.000001, 0 ).toDegreesMinutes( 5, false ), myControlString );
  myControlString = QString( "-0" ) + QChar( 176 ) +
                    QString( "0.00006" ) + QChar( 0x2032 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "0.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( -0.000001, 0 ).toDegreesMinutes( 5, false ), myControlString );
}

void TestQgsPoint::toDegreesMinutesPadded()
{
  QString myControlString = QString( "80" ) + QChar( 176 ) +
                            QString( "00.00" ) + QChar( 0x2032 ) +
                            QString( "E,20" ) + QChar( 176 ) +
                            QString( "00.00" ) + QChar( 0x2032 ) + QString( "N" );
  qDebug() << myControlString;
  QCOMPARE( mPoint4.toDegreesMinutes( 2, true, true ), myControlString );

  //should be no directional suffixes for 0 degree coordinates
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "00.00" ) + QChar( 0x2032 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "00.00" ) + QChar( 0x2032 );
  QVERIFY( QgsPoint( 0, 0 ).toDegreesMinutes( 2, true, true ) == myControlString );
  //should also be no directional suffix for 0 degree coordinates within specified precision
  QCOMPARE( QgsPoint( 0, 0.000001 ).toDegreesMinutes( 2, true, true ), myControlString );
  QCOMPARE( QgsPoint( 0, -0.000001 ).toDegreesMinutes( 2, true, true ), myControlString );
  QCOMPARE( QgsPoint( -0.000001, 0 ).toDegreesMinutes( 2, true, true ), myControlString );
  QCOMPARE( QgsPoint( 0.000001, 0 ).toDegreesMinutes( 2, true, true ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "00.00000" ) + QChar( 0x2032 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "00.00006" ) + QChar( 0x2032 ) + QString( "N" );
  QCOMPARE( QgsPoint( 0, 0.000001 ).toDegreesMinutes( 5, true, true ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "00.00000" ) + QChar( 0x2032 ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "00.00006" ) + QChar( 0x2032 ) + QString( "S" );
  QCOMPARE( QgsPoint( 0, -0.000001 ).toDegreesMinutes( 5, true, true ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "00.00006" ) + QChar( 0x2032 ) + QString( "E" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "00.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( 0.000001, 0 ).toDegreesMinutes( 5, true, true ), myControlString );
  myControlString = QString( "0" ) + QChar( 176 ) +
                    QString( "00.00006" ) + QChar( 0x2032 ) + QString( "W" ) +
                    QString( ",0" ) + QChar( 176 ) +
                    QString( "00.00000" ) + QChar( 0x2032 );
  QCOMPARE( QgsPoint( -0.000001, 0 ).toDegreesMinutes( 5, true, true ), myControlString );
}

void TestQgsPoint::wellKnownText()
{

}

void TestQgsPoint::sqrDist()
{

}

void TestQgsPoint::multiply()
{

}

void TestQgsPoint::onSegment()
{

}

QTEST_MAIN( TestQgsPoint )
#include "moc_testqgspoint.cxx"
