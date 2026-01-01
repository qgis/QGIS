/***************************************************************************
     testqgsbrowsermodel.cpp
     --------------------------------------
    Date                 : October 2018
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
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsbrowsermodel.h"
#include "qgsdataitemprovider.h"
#include "qgsdataitemproviderregistry.h"
#include "qgsdatacollectionitem.h"
#include "qgsdirectoryitem.h"

class TestQgsBrowserModel : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testModel();
    void driveItems();
    void updatesToDataItemProviderRegistry();
};

void TestQgsBrowserModel::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );
}

void TestQgsBrowserModel::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsBrowserModel::testModel()
{
  QgsBrowserModel model;

  // empty
  QCOMPARE( model.rowCount(), 0 );
  QCOMPARE( model.columnCount(), 1 );
  QVERIFY( !model.data( QModelIndex() ).isValid() );
  QVERIFY( !model.flags( QModelIndex() ) );
  QVERIFY( !model.hasChildren() );
  QVERIFY( !model.dataItem( QModelIndex() ) );

  // add a root child
  QgsDataCollectionItem *rootItem1 = new QgsDataCollectionItem( nullptr, u"Test"_s, u"root1"_s, u"providerKeyRoot1"_s );
  QVERIFY( !model.findItem( rootItem1 ).isValid() );
  model.setupItemConnections( rootItem1 );
  model.mRootItems.append( rootItem1 );

  QCOMPARE( model.rowCount(), 1 );
  QCOMPARE( model.columnCount(), 1 );
  QVERIFY( !model.data( QModelIndex() ).isValid() );
  QVERIFY( !model.flags( QModelIndex() ) );
  QVERIFY( model.hasChildren() );
  QModelIndex root1Index = model.index( 0, 0 );
  QVERIFY( root1Index.isValid() );
  QCOMPARE( model.rowCount( root1Index ), 0 );
  QCOMPARE( model.columnCount( root1Index ), 1 );
  // initially, we say the item has children, until it's populated and we know for sure
  QVERIFY( model.hasChildren( root1Index ) );
  rootItem1->setState( Qgis::BrowserItemState::Populated );
  QVERIFY( !model.hasChildren( root1Index ) );
  QCOMPARE( model.data( root1Index ).toString(), u"Test"_s );
  QCOMPARE( model.data( root1Index, static_cast<int>( QgsBrowserModel::CustomRole::Path ) ).toString(), u"root1"_s );
  QCOMPARE( model.data( root1Index, static_cast<int>( QgsBrowserModel::CustomRole::ProviderKey ) ).toString(), u"providerKeyRoot1"_s );
  QCOMPARE( model.dataItem( root1Index ), rootItem1 );
  QCOMPARE( model.findItem( rootItem1 ), root1Index );

  // second root item
  QgsDataCollectionItem *rootItem2 = new QgsDataCollectionItem( nullptr, u"Test2"_s, u"root2"_s );
  model.setupItemConnections( rootItem2 );
  model.mRootItems.append( rootItem2 );

  QCOMPARE( model.rowCount(), 2 );
  QVERIFY( model.hasChildren() );
  QModelIndex root2Index = model.index( 1, 0 );
  QVERIFY( root2Index.isValid() );
  QCOMPARE( model.rowCount( root2Index ), 0 );
  QCOMPARE( model.columnCount( root2Index ), 1 );
  QCOMPARE( model.data( root2Index ).toString(), u"Test2"_s );
  QCOMPARE( model.data( root2Index, static_cast<int>( QgsBrowserModel::CustomRole::Path ) ).toString(), u"root2"_s );
  QVERIFY( model.data( root2Index, static_cast<int>( QgsBrowserModel::CustomRole::ProviderKey ) ).toString().isEmpty() );
  QCOMPARE( model.dataItem( root2Index ), rootItem2 );
  QCOMPARE( model.findItem( rootItem2 ), root2Index );

  // child item
  QgsDataCollectionItem *childItem1 = new QgsDataCollectionItem( nullptr, u"Child1"_s, u"child1"_s, u"providerKeyChild1"_s );
  model.setupItemConnections( childItem1 );
  rootItem1->addChild( childItem1 );

  QCOMPARE( model.rowCount(), 2 );
  QCOMPARE( model.columnCount(), 1 );
  QCOMPARE( model.rowCount( root1Index ), 1 );
  QCOMPARE( model.columnCount( root1Index ), 1 );
  QVERIFY( model.hasChildren( root1Index ) );
  QModelIndex child1Index = model.index( 0, 0, root1Index );
  QCOMPARE( model.data( child1Index ).toString(), u"Child1"_s );
  QCOMPARE( model.data( child1Index, static_cast<int>( QgsBrowserModel::CustomRole::Path ) ).toString(), u"child1"_s );
  QCOMPARE( model.data( child1Index, static_cast<int>( QgsBrowserModel::CustomRole::ProviderKey ) ).toString(), u"providerKeyChild1"_s );
  QCOMPARE( model.dataItem( child1Index ), childItem1 );
  QCOMPARE( model.findItem( childItem1 ), child1Index );
  QCOMPARE( model.findItem( childItem1, rootItem1 ), child1Index );
  // search for child in wrong parent
  QVERIFY( !model.findItem( childItem1, rootItem2 ).isValid() );


  // more children
  QgsDataCollectionItem *childItem2 = new QgsDataCollectionItem( nullptr, u"Child2"_s, u"child2"_s );
  rootItem1->addChildItem( childItem2, true );

  QgsDataCollectionItem *childItem3 = new QgsDataCollectionItem( nullptr, u"Child3"_s, u"child3"_s );
  childItem2->addChildItem( childItem3, true );
  QCOMPARE( childItem2->rowCount(), 1 );

  QgsDataCollectionItem *childItem4 = new QgsDataCollectionItem( nullptr, u"Child4"_s, u"child4"_s );
  rootItem2->addChildItem( childItem4, true );

  QCOMPARE( model.rowCount(), 2 );
  root1Index = model.index( 0, 0 );
  root2Index = model.index( 1, 0 );
  QCOMPARE( model.rowCount( root1Index ), 2 );
  child1Index = model.index( 0, 0, root1Index );
  QCOMPARE( model.data( child1Index ).toString(), u"Child1"_s );
  const QModelIndex child2Index = model.index( 1, 0, root1Index );
  QCOMPARE( model.data( child2Index ).toString(), u"Child2"_s );
  QCOMPARE( model.rowCount( child1Index ), 0 );
  QCOMPARE( model.dataItem( child2Index ), childItem2 );
  QCOMPARE( childItem2->rowCount(), 1 );
  QCOMPARE( model.rowCount( child2Index ), 1 );
  QCOMPARE( model.data( model.index( 0, 0, child2Index ) ).toString(), u"Child3"_s );
  QCOMPARE( model.rowCount( root2Index ), 1 );
  QCOMPARE( model.data( model.index( 0, 0, root2Index ) ).toString(), u"Child4"_s );
}

void TestQgsBrowserModel::driveItems()
{
  // an unapologetically linux-directed test ;)
  QgsBrowserModel model;
  QVERIFY( model.driveItems().empty() );

  model.initialize();
  QVERIFY( !model.driveItems().empty() );
  QVERIFY( model.driveItems().contains( u"/"_s ) );
  QgsDirectoryItem *rootItem = model.driveItems().value( u"/"_s );
  QVERIFY( rootItem );
  QCOMPARE( rootItem->path(), u"/"_s );
}


class TestDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override { return u"test"_s; }
    Qgis::DataItemProviderCapabilities capabilities() const override { return Qgis::DataItemProviderCapability::NetworkSources; }
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override
    {
      if ( path.isEmpty() )
        return new QgsDataItem( Qgis::BrowserItemType::Custom, parentItem, u"test-root-item"_s, path );
      return nullptr;
    }
};

static int testRootItemCount( QgsBrowserModel &model )
{
  int count = 0;
  for ( int i = 0; i < model.rowCount(); ++i )
  {
    if ( model.data( model.index( i, 0 ) ).toString() == "test-root-item"_L1 )
      ++count;
  }
  return count;
}

void TestQgsBrowserModel::updatesToDataItemProviderRegistry()
{
  QgsBrowserModel model;
  model.initialize();

  QCOMPARE( testRootItemCount( model ), 0 );

  QgsDataItemProvider *provider = new TestDataItemProvider;
  QgsApplication::dataItemProviderRegistry()->addProvider( provider );

  // browser should react to providerAdded() signal from the registry
  QCOMPARE( testRootItemCount( model ), 1 );

  QgsApplication::dataItemProviderRegistry()->removeProvider( provider );

  // browser should react to providerWillBeRemoved() signal from the registry
  QCOMPARE( testRootItemCount( model ), 0 );
}

QGSTEST_MAIN( TestQgsBrowserModel )
#include "testqgsbrowsermodel.moc"
