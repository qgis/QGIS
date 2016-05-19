/***************************************************************************
    testqgsdualview.cpp
     --------------------------------------
    Date                 : 14.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QtTest/QtTest>

#include <editorwidgets/core/qgseditorwidgetregistry.h>
#include <attributetable/qgsattributetableview.h>
#include <attributetable/qgsdualview.h>
#include "qgsattributeform.h"
#include <qgsapplication.h>
#include <qgsvectorlayer.h>
#include "qgsvectordataprovider.h"
#include <qgsmapcanvas.h>
#include <qgsfeature.h>

class TestQgsDualView : public QObject
{
    Q_OBJECT
  public:
    TestQgsDualView()
        : mCanvas( 0 )
        , mPointsLayer( 0 )
        , mDualView( 0 )
    {}

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testColumnCount();

    void testColumnHeaders();

    void testData();

    void testSelectAll();

    void testSort();

    void testAttributeFormSharedValueScanning();

  private:
    QgsMapCanvas* mCanvas;
    QgsVectorLayer* mPointsLayer;
    QString mTestDataDir;
    QgsDualView* mDualView;
};

void TestQgsDualView::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  QgsEditorWidgetRegistry::initEditors();

  // Setup a map canvas with a vector layer loaded...
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  // load a vector layer
  //
  QString myPointsFileName = mTestDataDir + "points.shp";
  QFileInfo myPointFileInfo( myPointsFileName );
  mPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                     myPointFileInfo.completeBaseName(), "ogr" );

  mCanvas = new QgsMapCanvas();
}

void TestQgsDualView::cleanupTestCase()
{
  delete mPointsLayer;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsDualView::init()
{
  mDualView = new QgsDualView();
  mDualView->init( mPointsLayer, mCanvas );
}

void TestQgsDualView::cleanup()
{
  delete mDualView;
}

void TestQgsDualView::testColumnCount()
{
  QCOMPARE( mDualView->tableView()->model()->columnCount(), mPointsLayer->fields().count() );
}

void TestQgsDualView::testColumnHeaders()
{
  for ( int i = 0; i < mPointsLayer->fields().count(); ++i )
  {
    const QgsField& fld = mPointsLayer->fields().at( i );
    QCOMPARE( mDualView->tableView()->model()->headerData( i, Qt::Horizontal ).toString(), fld.name() );
  }
}

void TestQgsDualView::testData()
{
  QgsFeature feature;
  mPointsLayer->getFeatures( QgsFeatureRequest().setFilterFid( 0 ) ).nextFeature( feature );

  for ( int i = 0; i < mPointsLayer->fields().count(); ++i )
  {
    const QgsField& fld = mPointsLayer->fields().at( i );

    QModelIndex index = mDualView->tableView()->model()->index( 0, i );
    QCOMPARE( mDualView->tableView()->model()->data( index ).toString(), fld.displayString( feature.attribute( i ) ) );
  }
}

void TestQgsDualView::testSelectAll()
{
  mDualView->setFilterMode( QgsAttributeTableFilterModel::ShowVisible );
  // Only show parts of the canvas, so only one selected feature is visible
  mCanvas->setExtent( QgsRectangle( -139, 23, -100, 48 ) );
  mDualView->mTableView->selectAll();
  QVERIFY( mPointsLayer->selectedFeatureCount() == 10 );

  mPointsLayer->selectByIds( QgsFeatureIds() );
  mCanvas->setExtent( QgsRectangle( -110, 40, -100, 48 ) );
  mDualView->mTableView->selectAll();
  QVERIFY( mPointsLayer->selectedFeatureCount() == 1 );
}

void TestQgsDualView::testSort()
{
  mDualView->setSortExpression( "Class" );

  QStringList classes;
  classes << "B52"
  << "B52"
  << "B52"
  << "B52"
  << "Biplane"
  << "Biplane"
  << "Biplane"
  << "Biplane"
  << "Biplane"
  << "Jet"
  << "Jet"
  << "Jet"
  << "Jet"
  << "Jet"
  << "Jet"
  << "Jet"
  << "Jet";

  for ( int i = 0; i < classes.length(); ++i )
  {
    QModelIndex index = mDualView->tableView()->model()->index( i, 0 );
    QCOMPARE( mDualView->tableView()->model()->data( index ).toString(), classes.at( i ) );
  }
}

void TestQgsDualView::testAttributeFormSharedValueScanning()
{
  // test QgsAttributeForm::scanForEqualAttributes

  QSet< int > mixedValueFields;
  QHash< int, QVariant > fieldSharedValues;

  // make a temporary layer to check through
  QgsVectorLayer* layer = new QgsVectorLayer( "Point?field=col1:integer&field=col2:integer&field=col3:integer&field=col4:integer", "test", "memory" );
  QVERIFY( layer->isValid() );
  QgsFeature f1( layer->dataProvider()->fields(), 1 );
  f1.setAttribute( "col1", 1 );
  f1.setAttribute( "col2", 1 );
  f1.setAttribute( "col3", 3 );
  f1.setAttribute( "col4", 1 );
  QgsFeature f2( layer->dataProvider()->fields(), 2 );
  f2.setAttribute( "col1", 1 );
  f2.setAttribute( "col2", 2 );
  f2.setAttribute( "col3", 3 );
  f2.setAttribute( "col4", 2 );
  QgsFeature f3( layer->dataProvider()->fields(), 3 );
  f3.setAttribute( "col1", 1 );
  f3.setAttribute( "col2", 2 );
  f3.setAttribute( "col3", 3 );
  f3.setAttribute( "col4", 2 );
  QgsFeature f4( layer->dataProvider()->fields(), 4 );
  f4.setAttribute( "col1", 1 );
  f4.setAttribute( "col2", 1 );
  f4.setAttribute( "col3", 3 );
  f4.setAttribute( "col4", 2 );
  layer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

  QgsAttributeForm form( layer );

  QgsFeatureIterator it = layer->getFeatures();

  form.scanForEqualAttributes( it, mixedValueFields, fieldSharedValues );

  QCOMPARE( mixedValueFields, QSet< int >() << 1 << 3 );
  QCOMPARE( fieldSharedValues.value( 0 ).toInt(), 1 );
  QCOMPARE( fieldSharedValues.value( 2 ).toInt(), 3 );

  // add another feature so all attributes are different
  QgsFeature f5( layer->dataProvider()->fields(), 5 );
  f5.setAttribute( "col1", 11 );
  f5.setAttribute( "col2", 11 );
  f5.setAttribute( "col3", 13 );
  f5.setAttribute( "col4", 12 );
  layer->dataProvider()->addFeatures( QgsFeatureList() << f5 );

  it = layer->getFeatures();

  form.scanForEqualAttributes( it, mixedValueFields, fieldSharedValues );
  QCOMPARE( mixedValueFields, QSet< int >() << 0 << 1 << 2 << 3 );
  QVERIFY( fieldSharedValues.isEmpty() );

  // single feature, all attributes should be shared
  it = layer->getFeatures( QgsFeatureRequest().setFilterFid( 4 ) );
  form.scanForEqualAttributes( it, mixedValueFields, fieldSharedValues );
  QCOMPARE( fieldSharedValues.value( 0 ).toInt(), 1 );
  QCOMPARE( fieldSharedValues.value( 1 ).toInt(), 1 );
  QCOMPARE( fieldSharedValues.value( 2 ).toInt(), 3 );
  QCOMPARE( fieldSharedValues.value( 3 ).toInt(), 2 );
  QVERIFY( mixedValueFields.isEmpty() );
}

QTEST_MAIN( TestQgsDualView )
#include "testqgsdualview.moc"




