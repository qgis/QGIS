/***************************************************************************
  testqgsapplication.cpp
  --------------------------------------
Date                 : Sun Sep 16 12:22:49 AKDT 2007
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
#include <QtTest/QtTest>
#include <QPixmap>

#define CPL_SUPRESS_CPLUSPLUS
#include <gdal.h>

//header for class being tested
#include <qgsapplication.h>

class TestQgsApplication: public QObject
{
    Q_OBJECT
  private slots:
    void checkPaths();
    void checkGdalSkip();
    void initTestCase();
    void cleanupTestCase();

    void accountName();
    void osName();
    void platformName();

  private:
    QString getQgisPath();
};


void TestQgsApplication::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  qDebug( "%s", QgsApplication::showSettings().toUtf8().constData() );
}

void TestQgsApplication::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsApplication::accountName()
{
  QString loginName = QgsApplication::userLoginName();
  qDebug() << QString( "Got login name: '%1'" ).arg( loginName );
  QVERIFY( !loginName.isEmpty() );
  //test cached return works correctly
  QCOMPARE( loginName, QgsApplication::userLoginName() );

  //can't test contents, as it can be validly empty (eg on Travis). Just testing that we don't crash
  QString fullName = QgsApplication::userFullName();
  qDebug() << QString( "Got full name: '%1'" ).arg( fullName );
  //test cached return works correctly
  QCOMPARE( fullName, QgsApplication::userFullName() );
}

void TestQgsApplication::osName()
{
  // can't test expected result, so just check for non-empty result
  qDebug() << QString( "Got OS name: '%1'" ).arg( QgsApplication::osName() );
  QVERIFY( !QgsApplication::osName().isEmpty() );
}

void TestQgsApplication::platformName()
{
  // test will always be run under desktop platform
  QCOMPARE( QgsApplication::platform(), QString( "desktop" ) );
}

void TestQgsApplication::checkPaths()
{
  QString myPath = QgsApplication::authorsFilePath();
  qDebug( "Checking authors file exists:" );
  qDebug( "%s", myPath.toLocal8Bit().constData() );
  QVERIFY( !myPath.isEmpty() );
}

void TestQgsApplication::checkGdalSkip()
{
  GDALAllRegister();
  QgsApplication::skipGdalDriver( "GTiff" );
  QVERIFY( QgsApplication::skippedGdalDrivers().contains( "GTiff" ) );
  QgsApplication::restoreGdalDriver( "GTiff" );
  QVERIFY( !QgsApplication::skippedGdalDrivers().contains( "GTiff" ) );
}

QTEST_MAIN( TestQgsApplication )
#include "testqgsapplication.moc"
