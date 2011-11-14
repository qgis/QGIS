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
  QString qgisPath = QCoreApplication::applicationDirPath();
  QgsApplication::init( INSTALL_PREFIX );
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
  QVERIFY( mPoint1.toString( 2 ) == QString( "20.00,-20.00" ) );
};
void TestQgsPoint::toDegreesMinutesSeconds()
{
  mReport += "<p>Testing toDegreesMinutesSecods()</p>";
  mReport += "<p>" + mPoint1.toDegreesMinutesSeconds( 2 )  +  "</p>";
  mReport += "<p>" + mPoint2.toDegreesMinutesSeconds( 2 )  +  "</p>";
  mReport += "<p>" + mPoint3.toDegreesMinutesSeconds( 2 )  +  "</p>";
  mReport += "<p>" + mPoint4.toDegreesMinutesSeconds( 2 )  +  "</p>";
  QVERIFY( mPoint4.toDegreesMinutesSeconds( 2 ) == QString( "80°0'0.00\"E,20°0'0.00\"N" ) );

};
void TestQgsPoint::wellKnownText()
{

};
void TestQgsPoint::sqrDist()
{

};
void TestQgsPoint::multiply()
{

};
void TestQgsPoint::onSegment()
{

};


QTEST_MAIN( TestQgsPoint )
#include "moc_testqgspoint.cxx"
