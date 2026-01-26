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
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgspythonutils.h"
#include "qgstest.h"

#include <QApplication>
#include <QObject>
#include <QSplashScreen>
#include <QString>
#include <QStringList>

/**
 * \ingroup UnitTests
 * This is a unit test for the QgisApp python support.
 */
class TestQgisAppPython : public QObject
{
    Q_OBJECT

  public:
    TestQgisAppPython();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void hasPython();
    void plugins();
    void pythonPlugin();
    void pluginMetadata();
    void pythonPluginDependencyOrder();
    void runString();
    void evalString();

  private:
    QgisApp *mQgisApp = nullptr;
    QString mTestDataDir;
};

TestQgisAppPython::TestQgisAppPython() = default;

//runs before all tests
void TestQgisAppPython::initTestCase()
{
  const QByteArray pluginPath = QByteArray( TEST_DATA_DIR ) + "/test_plugin_path";
  qputenv( "QGIS_PLUGINPATH", pluginPath );

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  qDebug() << "TestQgisAppPython::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  mQgisApp = new QgisApp();
  mQgisApp->loadPythonSupport();
}

//runs after all tests
void TestQgisAppPython::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgisAppPython::hasPython()
{
  QVERIFY( mQgisApp->mPythonUtils->isEnabled() );
}

void TestQgisAppPython::plugins()
{
  QVERIFY( mQgisApp->mPythonUtils->pluginList().contains( u"PluginPathTest"_s ) );
  QVERIFY( !mQgisApp->mPythonUtils->isPluginLoaded( u"PluginPathTest"_s ) );
  QVERIFY( mQgisApp->mPythonUtils->listActivePlugins().isEmpty() );
  // load plugin
  QVERIFY( !mQgisApp->mPythonUtils->unloadPlugin( u"PluginPathTest"_s ) );
  QVERIFY( mQgisApp->mPythonUtils->loadPlugin( u"PluginPathTest"_s ) );
  QVERIFY( !mQgisApp->mPythonUtils->isPluginLoaded( u"PluginPathTest"_s ) );
  QVERIFY( mQgisApp->mPythonUtils->startPlugin( u"PluginPathTest"_s ) );
  QVERIFY( mQgisApp->mPythonUtils->isPluginLoaded( u"PluginPathTest"_s ) );
  QCOMPARE( mQgisApp->mPythonUtils->listActivePlugins(), QStringList() << u"PluginPathTest"_s );
}

void TestQgisAppPython::pythonPlugin()
{
  QVERIFY( mQgisApp->mPythonUtils->pluginList().contains( u"ProcessingPluginTest"_s ) );
  QVERIFY( mQgisApp->mPythonUtils->loadPlugin( u"ProcessingPluginTest"_s ) );
  QVERIFY( mQgisApp->mPythonUtils->startProcessingPlugin( u"ProcessingPluginTest"_s ) );
  QVERIFY( !mQgisApp->mPythonUtils->startProcessingPlugin( u"PluginPathTest"_s ) );
}

void TestQgisAppPython::pluginMetadata()
{
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( u"not a plugin"_s, u"name"_s ), u"__error__"_s );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( u"PluginPathTest"_s, u"invalid"_s ), u"__error__"_s );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( u"PluginPathTest"_s, u"name"_s ), u"plugin path test"_s );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( u"PluginPathTest"_s, u"qgisMinimumVersion"_s ), u"2.0"_s );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( u"PluginPathTest"_s, u"description"_s ), u"desc"_s );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( u"PluginPathTest"_s, u"version"_s ), u"0.1"_s );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( u"PluginPathTest"_s, u"author"_s ), u"HM/Oslandia"_s );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( u"PluginPathTest"_s, u"email"_s ), u"hugo.mercier@oslandia.com"_s );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( u"PluginPathTest"_s, u"hasProcessingProvider"_s ), u"__error__"_s );
  QVERIFY( !mQgisApp->mPythonUtils->pluginHasProcessingProvider( u"x"_s ) );
  QVERIFY( !mQgisApp->mPythonUtils->pluginHasProcessingProvider( u"PluginPathTest"_s ) );

  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( u"ProcessingPluginTest"_s, u"name"_s ), u"processing plugin test"_s );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( u"ProcessingPluginTest"_s, u"hasProcessingProvider"_s ), u"yes"_s );
  QVERIFY( mQgisApp->mPythonUtils->pluginHasProcessingProvider( u"ProcessingPluginTest"_s ) );
  // hasProcessingProvider also accepts true/True
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( u"ProcessingPluginTest2"_s, u"hasProcessingProvider"_s ), u"True"_s );
  QVERIFY( mQgisApp->mPythonUtils->pluginHasProcessingProvider( u"ProcessingPluginTest2"_s ) );
}

void TestQgisAppPython::pythonPluginDependencyOrder()
{
  QVERIFY( mQgisApp->mPythonUtils->pluginList().contains( u"PluginPathTest"_s ) );
  QVERIFY( mQgisApp->mPythonUtils->pluginList().contains( u"dependent_plugin_1"_s ) );
  QVERIFY( mQgisApp->mPythonUtils->pluginList().contains( u"dependent_plugin_2"_s ) );

  const int indexIndependentPlugin = mQgisApp->mPythonUtils->pluginList().indexOf( "PluginPathTest"_L1 );
  const int indexDependentPlugin1 = mQgisApp->mPythonUtils->pluginList().indexOf( "dependent_plugin_1"_L1 );
  const int indexDependentPlugin2 = mQgisApp->mPythonUtils->pluginList().indexOf( "dependent_plugin_2"_L1 );

  // Dependent plugins should appear in this list after their dependencies,
  // since that's the order in which they'll be loaded to QGIS
  QVERIFY( indexIndependentPlugin < indexDependentPlugin1 );
  QVERIFY( indexDependentPlugin1 < indexDependentPlugin2 );
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


QGSTEST_MAIN( TestQgisAppPython )
#include "testqgisapppython.moc"
