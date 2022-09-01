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
#include "qgstest.h"
#include <QPixmap>

#define CPL_SUPRESS_CPLUSPLUS  //#spellok
#include <gdal.h>

//header for class being tested
#include <qgsapplication.h>
#include "qgsrenderchecker.h"

class TestQgsApplication: public QgsTest
{
    Q_OBJECT

  public:

    TestQgsApplication() : QgsTest( QStringLiteral( "QgsApplication Tests" ) ) {}

  private slots:
    void checkPaths();
    void checkGdalSkip();
    void initTestCase();
    void cleanupTestCase();

    void accountName();
    void osName();
    void platformName();
    void themeIcon();

  private:
    QString getQgisPath();
    bool renderCheck( const QString &testName, QImage &image, int mismatchCount = 0 );

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
  const QString loginName = QgsApplication::userLoginName();
  qDebug() << QStringLiteral( "Got login name: '%1'" ).arg( loginName );
  QVERIFY( !loginName.isEmpty() );
  //test cached return works correctly
  QCOMPARE( loginName, QgsApplication::userLoginName() );

  //can't test contents, as it can be validly empty (e.g., on Travis). Just testing that we don't crash
  const QString fullName = QgsApplication::userFullName();
  qDebug() << QStringLiteral( "Got full name: '%1'" ).arg( fullName );
  //test cached return works correctly
  QCOMPARE( fullName, QgsApplication::userFullName() );
}

void TestQgsApplication::osName()
{
  // can't test expected result, so just check for non-empty result
  qDebug() << QStringLiteral( "Got OS name: '%1'" ).arg( QgsApplication::osName() );
  QVERIFY( !QgsApplication::osName().isEmpty() );
}

void TestQgsApplication::platformName()
{
  // test will always be run under external platform
  QCOMPARE( QgsApplication::platform(), QString( "external" ) );
}

void TestQgsApplication::themeIcon()
{
  QIcon icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconFolder.svg" ) );
  QVERIFY( !icon.isNull() );
  QImage im( icon.pixmap( 16, 16 ).toImage() );
  QVERIFY( renderCheck( QStringLiteral( "theme_icon" ), im, 0 ) );

  // with colors
  icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconFolderParams.svg" ), QColor( 255, 100, 100 ), QColor( 255, 0, 0 ) );
  im = QImage( icon.pixmap( 16, 16 ).toImage() );
  QVERIFY( renderCheck( QStringLiteral( "theme_icon_colors_1" ), im, 0 ) );

  // different colors
  icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconFolderParams.svg" ), QColor( 170, 255, 170 ), QColor( 0, 255, 0 ) );
  im = QImage( icon.pixmap( 16, 16 ).toImage() );
  QVERIFY( renderCheck( QStringLiteral( "theme_icon_colors_2" ), im, 0 ) );
}

bool TestQgsApplication::renderCheck( const QString &testName, QImage &image, int mismatchCount )
{
  const QString myTmpDir = QDir::tempPath() + '/';
  const QString myFileName = myTmpDir + testName + ".png";
  image.save( myFileName, "PNG" );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "application" ) );
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  const bool myResultFlag = myChecker.compareImages( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}

void TestQgsApplication::checkPaths()
{
  const QString myPath = QgsApplication::authorsFilePath();
  qDebug( "Checking authors file exists:" );
  qDebug( "%s", myPath.toLocal8Bit().constData() );
  QVERIFY( !myPath.isEmpty() );
}

void TestQgsApplication::checkGdalSkip()
{
  GDALAllRegister();
  QgsApplication::skipGdalDriver( QStringLiteral( "GTiff" ) );
  QVERIFY( QgsApplication::skippedGdalDrivers().contains( "GTiff" ) );
  QgsApplication::restoreGdalDriver( QStringLiteral( "GTiff" ) );
  QVERIFY( !QgsApplication::skippedGdalDrivers().contains( "GTiff" ) );
}

QGSTEST_MAIN( TestQgsApplication )
#include "testqgsapplication.moc"
