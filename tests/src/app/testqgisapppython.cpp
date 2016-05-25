/***************************************************************************
     testqgsapppython.cpp
     --------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QApplication>
#include <QObject>
#include <QSplashScreen>
#include <QString>
#include <QStringList>
#include <QtTest/QtTest>

#include <qgisapp.h>
#include <qgsapplication.h>

/** \ingroup UnitTests
 * This is a unit test for the QgisApp python support.
 */
class TestQgisAppPython : public QObject
{
    Q_OBJECT

  public:
    TestQgisAppPython();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void runString();
    void evalString();

  private:
    QgisApp * mQgisApp;
    QString mTestDataDir;
};

TestQgisAppPython::TestQgisAppPython()
    : mQgisApp( nullptr )
{

}

//runs before all tests
void TestQgisAppPython::initTestCase()
{
  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );

  qDebug() << "TestQgisAppClipboard::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mTestDataDir = QString( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  mQgisApp = new QgisApp();
  mQgisApp->loadPythonSupport();
}

//runs after all tests
void TestQgisAppPython::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgisAppPython::runString()
{
  QVERIFY( mQgisApp->mPythonUtils->runString( "a=1+1" ) );
  QVERIFY( !mQgisApp->mPythonUtils->runString( "x" ) );
  QVERIFY( !mQgisApp->mPythonUtils->runString( "" ) );
}

void TestQgisAppPython::evalString()
{
  QString result;
  //good string
  QVERIFY( mQgisApp->mPythonUtils->evalString( "1+1", result ) );
  QCOMPARE( result, QString( "2" ) );

  //bad string
  QVERIFY( !mQgisApp->mPythonUtils->evalString( "1+", result ) );
}

QTEST_MAIN( TestQgisAppPython )
#include "testqgisapppython.moc"
