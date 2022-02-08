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
#include "qgstest.h"

#include <qgisapp.h>
#include <qgsapplication.h>
#include "qgspythonutils.h"

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
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

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
  qputenv( "QGIS_PLUGINPATH", QByteArray( TEST_DATA_DIR ) + "/test_plugin_path" );

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  qDebug() << "TestQgisAppClipboard::initTestCase()";
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
  QVERIFY( mQgisApp->mPythonUtils->pluginList().contains( QStringLiteral( "PluginPathTest" ) ) );
  QVERIFY( !mQgisApp->mPythonUtils->isPluginLoaded( QStringLiteral( "PluginPathTest" ) ) );
  QVERIFY( mQgisApp->mPythonUtils->listActivePlugins().isEmpty() );
  // load plugin
  QVERIFY( !mQgisApp->mPythonUtils->unloadPlugin( QStringLiteral( "PluginPathTest" ) ) );
  QVERIFY( mQgisApp->mPythonUtils->loadPlugin( QStringLiteral( "PluginPathTest" ) ) );
  QVERIFY( !mQgisApp->mPythonUtils->isPluginLoaded( QStringLiteral( "PluginPathTest" ) ) );
  QVERIFY( mQgisApp->mPythonUtils->startPlugin( QStringLiteral( "PluginPathTest" ) ) );
  QVERIFY( mQgisApp->mPythonUtils->isPluginLoaded( QStringLiteral( "PluginPathTest" ) ) );
  QCOMPARE( mQgisApp->mPythonUtils->listActivePlugins(), QStringList() << QStringLiteral( "PluginPathTest" ) );
}

void TestQgisAppPython::pythonPlugin()
{
  QVERIFY( mQgisApp->mPythonUtils->pluginList().contains( QStringLiteral( "ProcessingPluginTest" ) ) );
  QVERIFY( mQgisApp->mPythonUtils->loadPlugin( QStringLiteral( "ProcessingPluginTest" ) ) );
  QVERIFY( mQgisApp->mPythonUtils->startProcessingPlugin( QStringLiteral( "ProcessingPluginTest" ) ) );
  QVERIFY( !mQgisApp->mPythonUtils->startProcessingPlugin( QStringLiteral( "PluginPathTest" ) ) );
}

void TestQgisAppPython::pluginMetadata()
{
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( QStringLiteral( "not a plugin" ), QStringLiteral( "name" ) ), QStringLiteral( "__error__" ) );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( QStringLiteral( "PluginPathTest" ), QStringLiteral( "invalid" ) ), QStringLiteral( "__error__" ) );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( QStringLiteral( "PluginPathTest" ), QStringLiteral( "name" ) ), QStringLiteral( "plugin path test" ) );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( QStringLiteral( "PluginPathTest" ), QStringLiteral( "qgisMinimumVersion" ) ), QStringLiteral( "2.0" ) );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( QStringLiteral( "PluginPathTest" ), QStringLiteral( "description" ) ), QStringLiteral( "desc" ) );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( QStringLiteral( "PluginPathTest" ), QStringLiteral( "version" ) ), QStringLiteral( "0.1" ) );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( QStringLiteral( "PluginPathTest" ), QStringLiteral( "author" ) ), QStringLiteral( "HM/Oslandia" ) );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( QStringLiteral( "PluginPathTest" ), QStringLiteral( "email" ) ), QStringLiteral( "hugo.mercier@oslandia.com" ) );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( QStringLiteral( "PluginPathTest" ), QStringLiteral( "hasProcessingProvider" ) ), QStringLiteral( "__error__" ) );
  QVERIFY( !mQgisApp->mPythonUtils->pluginHasProcessingProvider( QStringLiteral( "x" ) ) );
  QVERIFY( !mQgisApp->mPythonUtils->pluginHasProcessingProvider( QStringLiteral( "PluginPathTest" ) ) );

  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( QStringLiteral( "ProcessingPluginTest" ), QStringLiteral( "name" ) ), QStringLiteral( "processing plugin test" ) );
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( QStringLiteral( "ProcessingPluginTest" ), QStringLiteral( "hasProcessingProvider" ) ), QStringLiteral( "yes" ) );
  QVERIFY( mQgisApp->mPythonUtils->pluginHasProcessingProvider( QStringLiteral( "ProcessingPluginTest" ) ) );
  // hasProcessingProvider also accepts true/True
  QCOMPARE( mQgisApp->mPythonUtils->getPluginMetadata( QStringLiteral( "ProcessingPluginTest2" ), QStringLiteral( "hasProcessingProvider" ) ), QStringLiteral( "True" ) );
  QVERIFY( mQgisApp->mPythonUtils->pluginHasProcessingProvider( QStringLiteral( "ProcessingPluginTest2" ) ) );
}

void TestQgisAppPython::pythonPluginDependencyOrder()
{
  QVERIFY( mQgisApp->mPythonUtils->pluginList().contains( QStringLiteral( "PluginPathTest" ) ) );
  QVERIFY( mQgisApp->mPythonUtils->pluginList().contains( QStringLiteral( "dependent_plugin_1" ) ) );
  QVERIFY( mQgisApp->mPythonUtils->pluginList().contains( QStringLiteral( "dependent_plugin_2" ) ) );

  const int indexIndependentPlugin = mQgisApp->mPythonUtils->pluginList().indexOf( QLatin1String( "PluginPathTest" ) );
  const int indexDependentPlugin1 = mQgisApp->mPythonUtils->pluginList().indexOf( QLatin1String( "dependent_plugin_1" ) );
  const int indexDependentPlugin2 = mQgisApp->mPythonUtils->pluginList().indexOf( QLatin1String( "dependent_plugin_2" ) );

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
