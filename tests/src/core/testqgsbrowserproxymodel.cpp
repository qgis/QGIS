/***************************************************************************
     testqgsbrowserproxymodel.cpp
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
#include "qgsapplication.h"
#include "qgsbrowsermodel.h"
#include "qgsbrowserproxymodel.h"
#include "qgsdatacollectionitem.h"
#include "qgslayeritem.h"


class TestQgsBrowserProxyModel : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testModel();
    void testShowLayers();
};

class TestCollectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    TestCollectionItem( QgsDataItem *parent, const QString &name, const QString &path = QString(), const QString &providerKey = QString() )
      : QgsDataCollectionItem( parent, name, path, providerKey ) {
      };

    bool layerCollection() const override { return true; };
};


void TestQgsBrowserProxyModel::initTestCase()
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

void TestQgsBrowserProxyModel::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsBrowserProxyModel::testModel()
{
  QgsBrowserModel model;
  QgsBrowserProxyModel proxy;
  QVERIFY( !proxy.browserModel() );
  proxy.setBrowserModel( &model );
  QCOMPARE( proxy.browserModel(), &model );

  // empty model
  QCOMPARE( proxy.rowCount(), 0 );
  QCOMPARE( proxy.columnCount(), 1 );
  QVERIFY( !proxy.data( QModelIndex() ).isValid() );
  QVERIFY( !proxy.flags( QModelIndex() ) );
  QVERIFY( !proxy.hasChildren() );
  QVERIFY( !proxy.dataItem( QModelIndex() ) );

  // add a root child to model
  QgsDataCollectionItem *rootItem1 = new QgsDataCollectionItem( nullptr, u"Test"_s, u"root1"_s );
  model.setupItemConnections( rootItem1 );
  model.beginInsertRows( QModelIndex(), 0, 0 );
  model.mRootItems.append( rootItem1 );
  model.endInsertRows();

  QCOMPARE( proxy.rowCount(), 1 );
  QCOMPARE( proxy.columnCount(), 1 );
  QVERIFY( !proxy.data( QModelIndex() ).isValid() );
  QVERIFY( !proxy.flags( QModelIndex() ) );
  QVERIFY( proxy.hasChildren() );
  QModelIndex root1Index = proxy.index( 0, 0 );
  QVERIFY( root1Index.isValid() );
  QCOMPARE( proxy.rowCount( root1Index ), 0 );
  QCOMPARE( proxy.columnCount( root1Index ), 1 );
  QCOMPARE( proxy.data( root1Index ).toString(), u"Test"_s );
  QCOMPARE( proxy.data( root1Index, static_cast<int>( QgsBrowserModel::CustomRole::Path ) ).toString(), u"root1"_s );
  QCOMPARE( proxy.dataItem( root1Index ), rootItem1 );

  // second root item
  QgsDataCollectionItem *rootItem2 = new QgsDataCollectionItem( nullptr, u"Test2"_s, u"root2"_s, u"provider2"_s );
  model.setupItemConnections( rootItem2 );
  model.beginInsertRows( QModelIndex(), 1, 1 );
  model.mRootItems.append( rootItem2 );
  model.endInsertRows();

  QCOMPARE( proxy.rowCount(), 2 );
  QVERIFY( proxy.hasChildren() );
  QModelIndex root2Index = proxy.index( 1, 0 );
  QVERIFY( root2Index.isValid() );
  QCOMPARE( proxy.rowCount( root2Index ), 0 );
  QCOMPARE( proxy.columnCount( root2Index ), 1 );
  QCOMPARE( proxy.data( root2Index ).toString(), u"Test2"_s );
  QCOMPARE( proxy.data( root2Index, static_cast<int>( QgsBrowserModel::CustomRole::Path ) ).toString(), u"root2"_s );
  QCOMPARE( proxy.dataItem( root2Index ), rootItem2 );

  // child item
  QgsDataCollectionItem *childItem1 = new QgsDataCollectionItem( nullptr, u"Child1"_s, u"child1"_s, u"provider1"_s );
  rootItem1->addChildItem( childItem1, true );

  QCOMPARE( proxy.rowCount(), 2 );
  QCOMPARE( proxy.columnCount(), 1 );
  QCOMPARE( proxy.rowCount( root1Index ), 1 );
  QCOMPARE( proxy.columnCount( root1Index ), 1 );
  QVERIFY( proxy.hasChildren( root1Index ) );
  QModelIndex child1Index = proxy.index( 0, 0, root1Index );
  QCOMPARE( proxy.data( child1Index ).toString(), u"Child1"_s );
  QCOMPARE( proxy.data( child1Index, static_cast<int>( QgsBrowserModel::CustomRole::Path ) ).toString(), u"child1"_s );
  QCOMPARE( proxy.dataItem( child1Index ), childItem1 );

  // more children
  QgsDataCollectionItem *childItem2 = new QgsDataCollectionItem( nullptr, u"Child2"_s, u"child2"_s );
  rootItem1->addChildItem( childItem2, true );

  QgsLayerItem *childItem3 = new QgsLayerItem( nullptr, u"Child3"_s, u"child3"_s, QString(), Qgis::BrowserLayerType::Vector, QString() );
  childItem2->addChildItem( childItem3, true );
  QCOMPARE( childItem2->rowCount(), 1 );

  QgsLayerItem *childItem4 = new QgsLayerItem( nullptr, u"Child4"_s, u"child4"_s, QString(), Qgis::BrowserLayerType::Raster, QString() );
  rootItem2->addChildItem( childItem4, true );
  QCOMPARE( rootItem2->rowCount(), 1 );

  QCOMPARE( proxy.rowCount(), 2 );
  root1Index = proxy.index( 0, 0 );
  root2Index = proxy.index( 1, 0 );
  QCOMPARE( proxy.rowCount( root1Index ), 2 );
  child1Index = proxy.index( 0, 0, root1Index );
  QCOMPARE( proxy.data( child1Index ).toString(), u"Child1"_s );
  QModelIndex child2Index = proxy.index( 1, 0, root1Index );
  QCOMPARE( proxy.data( child2Index ).toString(), u"Child2"_s );
  QCOMPARE( proxy.rowCount( child1Index ), 0 );
  QCOMPARE( proxy.dataItem( child2Index ), childItem2 );
  QCOMPARE( childItem2->rowCount(), 1 );
  QCOMPARE( proxy.rowCount( child2Index ), 1 );
  QCOMPARE( proxy.data( proxy.index( 0, 0, child2Index ) ).toString(), u"Child3"_s );
  QCOMPARE( proxy.rowCount( root2Index ), 1 );
  QCOMPARE( proxy.data( proxy.index( 0, 0, root2Index ) ).toString(), u"Child4"_s );

  // filtering
  proxy.setFilterString( u"2"_s );
  proxy.setFilterSyntax( QgsBrowserProxyModel::Normal );

  QCOMPARE( proxy.rowCount(), 2 );
  root1Index = proxy.index( 0, 0 );
  QCOMPARE( proxy.data( root1Index ).toString(), u"Test"_s );
  QCOMPARE( proxy.rowCount( root1Index ), 1 );
  child1Index = proxy.index( 0, 0, root1Index );
  QCOMPARE( proxy.data( child1Index ).toString(), u"Child2"_s );
  // children of string matched items should be shown
  QCOMPARE( proxy.rowCount( child1Index ), 1 );
  QCOMPARE( proxy.data( proxy.index( 0, 0, child1Index ) ).toString(), u"Child3"_s );
  root2Index = proxy.index( 1, 0 );
  QCOMPARE( proxy.data( root2Index ).toString(), u"Test2"_s );
  QCOMPARE( proxy.rowCount( root2Index ), 1 );
  QCOMPARE( proxy.data( proxy.index( 0, 0, root2Index ) ).toString(), u"Child4"_s );

  // wildcards
  proxy.setFilterString( u"2"_s );
  proxy.setFilterSyntax( QgsBrowserProxyModel::Wildcards );
  QCOMPARE( proxy.rowCount(), 0 );
  proxy.setFilterString( u"*2"_s );
  QCOMPARE( proxy.rowCount(), 2 );
  root1Index = proxy.index( 0, 0 );
  QCOMPARE( proxy.data( root1Index ).toString(), u"Test"_s );
  QCOMPARE( proxy.rowCount( root1Index ), 1 );
  child1Index = proxy.index( 0, 0, root1Index );
  QCOMPARE( proxy.data( child1Index ).toString(), u"Child2"_s );
  // children of string matched items should be shown
  QCOMPARE( proxy.rowCount( child1Index ), 1 );
  QCOMPARE( proxy.data( proxy.index( 0, 0, child1Index ) ).toString(), u"Child3"_s );
  root2Index = proxy.index( 1, 0 );
  QCOMPARE( proxy.data( root2Index ).toString(), u"Test2"_s );
  QCOMPARE( proxy.rowCount( root2Index ), 1 );
  QCOMPARE( proxy.data( proxy.index( 0, 0, root2Index ) ).toString(), u"Child4"_s );

  // regex
  proxy.setFilterString( u"^.*[td]2$"_s );
  proxy.setFilterSyntax( QgsBrowserProxyModel::Normal );
  QCOMPARE( proxy.rowCount(), 0 );
  proxy.setFilterSyntax( QgsBrowserProxyModel::RegularExpression );

  QCOMPARE( proxy.rowCount(), 2 );
  root1Index = proxy.index( 0, 0 );
  QCOMPARE( proxy.data( root1Index ).toString(), u"Test"_s );
  QCOMPARE( proxy.rowCount( root1Index ), 1 );
  child1Index = proxy.index( 0, 0, root1Index );
  QCOMPARE( proxy.data( child1Index ).toString(), u"Child2"_s );
  // children of string matched items should be shown
  QCOMPARE( proxy.rowCount( child1Index ), 1 );
  QCOMPARE( proxy.data( proxy.index( 0, 0, child1Index ) ).toString(), u"Child3"_s );
  root2Index = proxy.index( 1, 0 );
  QCOMPARE( proxy.data( root2Index ).toString(), u"Test2"_s );
  QCOMPARE( proxy.rowCount( root2Index ), 1 );
  QCOMPARE( proxy.data( proxy.index( 0, 0, root2Index ) ).toString(), u"Child4"_s );

  proxy.setFilterString( QString() );

  // layer type filtering
  proxy.setLayerType( Qgis::LayerType::Vector );
  proxy.setFilterByLayerType( true );

  QCOMPARE( proxy.rowCount(), 2 );
  root1Index = proxy.index( 0, 0 );
  root2Index = proxy.index( 1, 0 );
  QCOMPARE( proxy.rowCount( root1Index ), 2 );
  child1Index = proxy.index( 0, 0, root1Index );
  QCOMPARE( proxy.data( child1Index ).toString(), u"Child1"_s );
  child2Index = proxy.index( 1, 0, root1Index );
  QCOMPARE( proxy.data( child2Index ).toString(), u"Child2"_s );
  QCOMPARE( proxy.rowCount( child1Index ), 0 );
  QCOMPARE( proxy.dataItem( child2Index ), childItem2 );
  QCOMPARE( childItem2->rowCount(), 1 );
  QCOMPARE( proxy.rowCount( child2Index ), 1 );
  QCOMPARE( proxy.data( proxy.index( 0, 0, child2Index ) ).toString(), u"Child3"_s );
  QCOMPARE( proxy.rowCount( root2Index ), 0 );

  proxy.setLayerType( Qgis::LayerType::Raster );
  QCOMPARE( proxy.rowCount(), 2 );
  root1Index = proxy.index( 0, 0 );
  root2Index = proxy.index( 1, 0 );
  QCOMPARE( proxy.rowCount( root1Index ), 2 );
  child1Index = proxy.index( 0, 0, root1Index );
  QCOMPARE( proxy.data( child1Index ).toString(), u"Child1"_s );
  child2Index = proxy.index( 1, 0, root1Index );
  QCOMPARE( proxy.data( child2Index ).toString(), u"Child2"_s );
  QCOMPARE( proxy.rowCount( child1Index ), 0 );
  QCOMPARE( proxy.dataItem( child2Index ), childItem2 );
  QCOMPARE( childItem2->rowCount(), 1 );
  QCOMPARE( proxy.rowCount( child2Index ), 0 );
  QCOMPARE( proxy.rowCount( root2Index ), 1 );
  QCOMPARE( proxy.data( proxy.index( 0, 0, root2Index ) ).toString(), u"Child4"_s );

  proxy.setFilterByLayerType( false );

  // provider filtering
  proxy.setHiddenDataItemProviderKeyFilter( QStringList( { u"provider1"_s } ) );
  QCOMPARE( proxy.rowCount(), 2 );
  root1Index = proxy.index( 0, 0 );
  QCOMPARE( proxy.rowCount( root1Index ), 1 );
  proxy.setHiddenDataItemProviderKeyFilter( QStringList( { u"provider2"_s } ) );
  QCOMPARE( proxy.rowCount(), 1 );
  root1Index = proxy.index( 0, 0 );
  QCOMPARE( proxy.rowCount( root1Index ), 2 );
  proxy.setHiddenDataItemProviderKeyFilter( QStringList() );
  QCOMPARE( proxy.rowCount(), 2 );

  // provider filtering
  proxy.setHiddenDataItemProviderKeyFilter( QStringList() );
  proxy.setShownDataItemProviderKeyFilter( QStringList( { u"provider2"_s } ) );
  QCOMPARE( proxy.rowCount(), 2 );
  root1Index = proxy.index( 0, 0 );
  QCOMPARE( proxy.rowCount( root1Index ), 1 );
  proxy.setShownDataItemProviderKeyFilter( QStringList( { u"provider1"_s } ) );
  QCOMPARE( proxy.rowCount(), 1 );
  root1Index = proxy.index( 0, 0 );
  QCOMPARE( proxy.rowCount( root1Index ), 2 );
  proxy.setShownDataItemProviderKeyFilter( QStringList() );
  QCOMPARE( proxy.rowCount(), 2 );
}

void TestQgsBrowserProxyModel::testShowLayers()
{
  QgsBrowserModel model;
  QgsBrowserProxyModel proxy;
  QVERIFY( !proxy.browserModel() );
  proxy.setBrowserModel( &model );
  QCOMPARE( proxy.browserModel(), &model );

  // add a root child to model
  QgsDataCollectionItem *rootItem1 = new QgsDataCollectionItem( nullptr, u"Test"_s, u"root1"_s );
  model.setupItemConnections( rootItem1 );
  model.beginInsertRows( QModelIndex(), 0, 0 );
  model.mRootItems.append( rootItem1 );
  model.endInsertRows();

  // Add a layer collection item
  QgsDataCollectionItem *containerItem1 = new TestCollectionItem( nullptr, u"Test"_s, u"root1"_s );
  rootItem1->addChildItem( containerItem1 );
  QgsLayerItem *childItem1 = new QgsLayerItem( nullptr, u"Child1"_s, u"child1"_s, QString(), Qgis::BrowserLayerType::Vector, QString() );
  containerItem1->addChildItem( childItem1, true );
  QCOMPARE( proxy.rowCount(), 1 );
  const auto root1Index = proxy.index( 0, 0 );
  QVERIFY( root1Index.isValid() );
  const auto container1Index = proxy.index( 0, 0, root1Index );
  QVERIFY( container1Index.isValid() );
  QVERIFY( proxy.hasChildren( container1Index ) );

  proxy.setShowLayers( false );
  QVERIFY( !proxy.hasChildren( container1Index ) );
}

QGSTEST_MAIN( TestQgsBrowserProxyModel )
#include "testqgsbrowserproxymodel.moc"
