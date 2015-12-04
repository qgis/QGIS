/***************************************************************************
    testqgsattributetablemodel.cpp
    --------------------------------------
    Date                 : 12/02/2015
    Copyright            : (C) 20153 Alessandro Pasotti
    Email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QtTest/QtTest>

#include <attributetable/qgsattributetablemodel.h>
#include <editorwidgets/core/qgseditorwidgetregistry.h>
#include <qgsapplication.h>
#include <qgsvectorlayer.h>
#include <qgsvectorlayercache.h>
#include <qgsfeature.h>
#include <qgsvectordataprovider.h>


class TestQgsAttributeTableModel : public QObject
{
    Q_OBJECT
  public:
    TestQgsAttributeTableModel()
        : mLayer( 0 )
        , mCache( 0 )
        , mModel( 0 )
    {}

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testLoad();
    void testRemove();
    void testAdd();
    void testRemoveColumns();
    // Doing our best to segfault
    void testCrash();

  private:
    void createLayer();
    void testForIdsInMap();
    QgsVectorLayer* mLayer;
    QgsVectorLayerCache* mCache;
    QgsAttributeTableModel* mModel;
};


void TestQgsAttributeTableModel::createLayer()
{
  mLayer = new QgsVectorLayer( "Point?field=fldtxt:string&field=fldint:integer",
                               "addfeat", "memory" );
  QgsFeatureList features;
  for ( int i = 1; i <= 10; i++ )
  {
    QgsFeature f;
    QgsAttributes atts;
    atts.append( "test" );
    atts.append( i );
    f.setAttributes( atts );
    f.setGeometry( QgsGeometry::fromPoint( QgsPoint( 100 * i, 2 ^ i ) ) );
    features.append( f );
  }
  QgsVectorDataProvider *pr( mLayer->dataProvider() );
  pr->addFeatures( features );
}

void TestQgsAttributeTableModel::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  QgsEditorWidgetRegistry::initEditors();
}

void TestQgsAttributeTableModel::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAttributeTableModel::init()
{
  createLayer();
  mCache = new QgsVectorLayerCache( mLayer, 100 );
  mModel = new QgsAttributeTableModel( mCache );
  mModel->loadLayer();
}

void TestQgsAttributeTableModel::cleanup()
{
  delete mModel;
  delete mCache;
  delete mLayer;
}


void TestQgsAttributeTableModel::testForIdsInMap()
{
  QgsFeatureIterator features = mLayer->getFeatures();
  QgsFeature f;
  while ( features.nextFeature( f ) )
  {
    QCOMPARE( mModel->rowToId( mModel->idToRow( f.id() ) ), f.id() );
  }
}

void TestQgsAttributeTableModel::testLoad()
{
  mModel->waitLoader();
  QCOMPARE( mModel->rowCount(), 10 );
  QCOMPARE( mModel->columnCount(), 2 );
  testForIdsInMap();
}

void TestQgsAttributeTableModel::testRemove()
{
  mLayer->startEditing();
  mLayer->deleteFeature( 5 );
  testForIdsInMap();
  QCOMPARE( mModel->rowCount(), 9 );
  QgsFeatureIds ids;
  ids << ( long )1 << ( long )3 << ( long )6 << ( long )7;
  mLayer->setSelectedFeatures( ids );
  mLayer->deleteSelectedFeatures();
  testForIdsInMap();
  QCOMPARE( mModel->rowCount(), 5 );
}

void TestQgsAttributeTableModel::testAdd()
{
  mLayer->startEditing();
  QgsFeature f;
  QgsAttributes atts;
  atts.append( "test" );
  atts.append( "8" );
  f.setAttributes( atts );
  f.setGeometry( QgsGeometry::fromPoint( QgsPoint( 100, 200 ) ) );
  QVERIFY( mLayer->addFeature( f ) );
  testForIdsInMap();
  QCOMPARE( mModel->rowCount(), 11 );
}


void TestQgsAttributeTableModel::testRemoveColumns()
{
  mLayer->startEditing();
  mLayer->deleteAttribute( 1 );
  QCOMPARE( mModel->columnCount(), 1 );
}

void TestQgsAttributeTableModel::testCrash()
{
  mLayer->startEditing();
  QgsFeature f;
  QgsAttributes atts;
  atts.append( "test" );
  atts.append( "8" );
  f.setAttributes( atts );
  f.setGeometry( QgsGeometry::fromPoint( QgsPoint( 100, 200 ) ) );
  for ( int i = 0; i < 101; i++ )
  {
    if ( i % 5 == 0 )
    {
      for ( int j = 0; j < 2001; j++ )
      {
        mLayer->addFeature( f );
      }
    }
    mModel->loadLayer();
  }
}

QTEST_MAIN( TestQgsAttributeTableModel )
#include "testqgsattributetablemodel.moc"
