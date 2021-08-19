/***************************************************************************
     testqgsappbrowserproviders.cpp
     --------------------------------------
    Date                 : October 30 2018
    Copyright            : (C) 2018 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
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
#include <QStringList>

//qgis includes...
#include "qgsdataitem.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsdataitemprovider.h"
#include "qgsdataitemproviderregistry.h"
#include "qgssettings.h"
#include "qgsappbrowserproviders.h"
#include "qgsdirectoryitem.h"

class TestQgsAppBrowserProviders : public QObject
{
    Q_OBJECT

  public:
    TestQgsAppBrowserProviders();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testProjectItemCreation();

  private:
    QgsDirectoryItem *mDirItem = nullptr;
    QString mScanItemsSetting;
    QString mTestDataDir;
};

TestQgsAppBrowserProviders::TestQgsAppBrowserProviders() = default;

void TestQgsAppBrowserProviders::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = dataDir + '/';

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );
  // save current scanItemsSetting value
  const QgsSettings settings;
  mScanItemsSetting = settings.value( QStringLiteral( "/qgis/scanItemsInBrowser2" ), QVariant( "" ) ).toString();

  //create a directory item that will be used in all tests...
  mDirItem = new QgsDirectoryItem( nullptr, QStringLiteral( "Test" ), TEST_DATA_DIR );
}

void TestQgsAppBrowserProviders::cleanupTestCase()
{
  // restore scanItemsSetting
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/qgis/scanItemsInBrowser2" ), mScanItemsSetting );
  if ( mDirItem )
    delete mDirItem;

  QgsApplication::exitQgis();
}


void TestQgsAppBrowserProviders::testProjectItemCreation()
{
  QgsDirectoryItem *dirItem = new QgsDirectoryItem( nullptr, QStringLiteral( "Test" ), mTestDataDir + QStringLiteral( "qgis_server/" ) );
  QVector<QgsDataItem *> children = dirItem->createChildren();

  // now, add a specific provider which handles project files
  QgsApplication::dataItemProviderRegistry()->addProvider( new QgsProjectDataItemProvider() );

  dirItem = new QgsDirectoryItem( nullptr, QStringLiteral( "Test" ), mTestDataDir + QStringLiteral( "qgis_server/" ) );
  children = dirItem->createChildren();

  for ( QgsDataItem *child : children )
  {
    if ( child->type() == Qgis::BrowserItemType::Project && child->path() == mTestDataDir + QStringLiteral( "qgis_server/test_project.qgs" ) )
    {
      child->populate( true );

      QCOMPARE( child->children().count(), 7 );
      QVERIFY( dynamic_cast< QgsProjectLayerTreeGroupItem * >( child->children().at( 2 ) ) );
      QCOMPARE( child->children().at( 2 )->name(), QStringLiteral( "groupwithoutshortname" ) );

      QCOMPARE( child->children().at( 2 )->children().count(), 1 );
      QVERIFY( dynamic_cast< QgsLayerItem * >( child->children().at( 2 )->children().at( 0 ) ) );
      QCOMPARE( child->children().at( 2 )->children().at( 0 )->name(), QStringLiteral( "testlayer3" ) );

      QVERIFY( dynamic_cast< QgsProjectLayerTreeGroupItem * >( child->children().at( 3 ) ) );
      QCOMPARE( child->children().at( 3 )->name(), QStringLiteral( "groupwithshortname" ) );

      QCOMPARE( child->children().at( 3 )->children().count(), 1 );
      QVERIFY( dynamic_cast< QgsLayerItem * >( child->children().at( 3 )->children().at( 0 ) ) );
      QCOMPARE( child->children().at( 3 )->children().at( 0 )->name(), QStringLiteral( "testlayer2" ) );

      QVERIFY( dynamic_cast< QgsLayerItem * >( child->children().at( 5 ) ) );
      QCOMPARE( child->children().at( 5 )->name(), QStringLiteral( "testlayer" ) );

      QVERIFY( dynamic_cast< QgsLayerItem * >( child->children().at( 6 ) ) );
      QCOMPARE( child->children().at( 6 )->name(), QStringLiteral( u"testlayer \u00E8\u00E9" ) );

      delete dirItem;
      return;
    }
  }
  delete dirItem;
  QVERIFY( false ); // should not be reached
}

QGSTEST_MAIN( TestQgsAppBrowserProviders )
#include "testqgsappbrowserproviders.moc"
