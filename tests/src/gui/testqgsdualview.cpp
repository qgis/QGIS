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


#include <memory>

#include "attributetable/qgsattributetableview.h"
#include "attributetable/qgsdualview.h"
#include "editform/qgsattributeeditorhtmlelement.h"
#include "editorwidgets/core/qgseditorwidgetregistry.h"
#include "qgsapplication.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorfield.h"
#include "qgsattributeform.h"
#include "qgsattributeformeditorwidget.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturelistmodel.h"
#include "qgsgui.h"
#include "qgsmapcanvas.h"
#include "qgstest.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayercache.h"

class TestQgsDualView : public QObject
{
    Q_OBJECT
  public:
    TestQgsDualView() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testColumnCount();

    void testColumnHeaders();

    void testData();
    void testAttributeTableConfig();
    void testFilterSelected();

    void testSelectAll();

    void testSort();

    void testAttributeFormSharedValueScanning();
    void testNoGeom();
    void testNoShowFirstFeature();

    void testDuplicateField();

  private:
    QgsMapCanvas *mCanvas = nullptr;
    QgsVectorLayer *mPointsLayer = nullptr;
    QString mTestDataDir;
    QgsDualView *mDualView = nullptr;
};

void TestQgsDualView::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  QgsGui::editorWidgetRegistry()->initEditors();

  // Setup a map canvas with a vector layer loaded...
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  // load a vector layer
  //
  const QString myPointsFileName = mTestDataDir + "points.shp";
  const QFileInfo myPointFileInfo( myPointsFileName );
  mPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(), myPointFileInfo.completeBaseName(), u"ogr"_s );

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
    const QgsField fld = mPointsLayer->fields().at( i );
    QCOMPARE( mDualView->tableView()->model()->headerData( i, Qt::Horizontal ).toString(), fld.name() );
  }
}

void TestQgsDualView::testData()
{
  QgsFeature feature;
  mPointsLayer->getFeatures( QgsFeatureRequest().setFilterFid( 0 ) ).nextFeature( feature );

  for ( int i = 0; i < mPointsLayer->fields().count(); ++i )
  {
    const QgsField fld = mPointsLayer->fields().at( i );

    const QModelIndex index = mDualView->tableView()->model()->index( 0, i );
    QCOMPARE( mDualView->tableView()->model()->data( index ).toString(), fld.displayString( feature.attribute( i ) ) );
  }
}

void TestQgsDualView::testAttributeTableConfig()
{
  QCOMPARE( mDualView->attributeTableConfig().columns().count(), mPointsLayer->attributeTableConfig().columns().count() );
}

void TestQgsDualView::testFilterSelected()
{
  QgsFeature feature;
  QList<QgsFeatureId> ids;
  QgsFeatureIterator it = mPointsLayer->getFeatures( QgsFeatureRequest().setOrderBy( QgsFeatureRequest::OrderBy() << QgsFeatureRequest::OrderByClause( u"Heading"_s ) ) );
  while ( it.nextFeature( feature ) )
    ids << feature.id();

  // select some features
  QList<QgsFeatureId> selected;
  selected << ids.at( 1 ) << ids.at( 3 );
  mPointsLayer->selectByIds( qgis::listToSet( selected ) );

  mDualView->setFilterMode( QgsAttributeTableFilterModel::ShowSelected );
  QCOMPARE( mDualView->tableView()->model()->rowCount(), 2 );

  const int headingIdx = mPointsLayer->fields().lookupField( u"Heading"_s );
  const QgsField fld = mPointsLayer->fields().at( headingIdx );
  for ( int i = 0; i < selected.count(); ++i )
  {
    mPointsLayer->getFeatures( QgsFeatureRequest().setFilterFid( selected.at( i ) ) ).nextFeature( feature );
    const QModelIndex index = mDualView->tableView()->model()->index( i, headingIdx );
    QCOMPARE( mDualView->tableView()->model()->data( index ).toString(), fld.displayString( feature.attribute( headingIdx ) ) );
  }

  // select none
  mPointsLayer->removeSelection();
  QCOMPARE( mDualView->tableView()->model()->rowCount(), 0 );
}

void TestQgsDualView::testSelectAll()
{
  QEventLoop loop;
  connect( qobject_cast<QgsAttributeTableFilterModel *>( mDualView->mFilterModel ), &QgsAttributeTableFilterModel::visibleReloaded, &loop, &QEventLoop::quit );
  mDualView->setFilterMode( QgsAttributeTableFilterModel::ShowVisible );
  // Only show parts of the canvas, so only one selected feature is visible
  mCanvas->setExtent( QgsRectangle( -139, 23, -100, 48 ) );
  loop.exec();
  mDualView->mTableView->selectAll();
  QCOMPARE( mPointsLayer->selectedFeatureCount(), 10 );

  mPointsLayer->selectByIds( QgsFeatureIds() );
  mCanvas->setExtent( QgsRectangle( -110, 40, -100, 48 ) );
  loop.exec();
  mDualView->mTableView->selectAll();
  QCOMPARE( mPointsLayer->selectedFeatureCount(), 1 );
}

void TestQgsDualView::testSort()
{
  mDualView->setSortExpression( u"Class"_s );

  QStringList classes;
  classes << u"B52"_s
          << u"B52"_s
          << u"B52"_s
          << u"B52"_s
          << u"Biplane"_s
          << u"Biplane"_s
          << u"Biplane"_s
          << u"Biplane"_s
          << u"Biplane"_s
          << u"Jet"_s
          << u"Jet"_s
          << u"Jet"_s
          << u"Jet"_s
          << u"Jet"_s
          << u"Jet"_s
          << u"Jet"_s
          << u"Jet"_s;

  for ( int i = 0; i < classes.length(); ++i )
  {
    const QModelIndex index = mDualView->tableView()->model()->index( i, 0 );
    QCOMPARE( mDualView->tableView()->model()->data( index ).toString(), classes.at( i ) );
  }

  QStringList headings;
  headings << u"0"_s
           << u"0"_s
           << u"12"_s
           << u"34"_s
           << u"80"_s
           << u"85"_s
           << u"90"_s
           << u"90"_s
           << u"95"_s
           << u"100"_s
           << u"140"_s
           << u"160"_s
           << u"180"_s
           << u"240"_s
           << u"270"_s
           << u"300"_s
           << u"340"_s;

  mDualView->setSortExpression( u"Heading"_s );

  for ( int i = 0; i < headings.length(); ++i )
  {
    const QModelIndex index = mDualView->tableView()->model()->index( i, 1 );
    QCOMPARE( mDualView->tableView()->model()->data( index ).toString(), headings.at( i ) );
  }
}

void TestQgsDualView::testAttributeFormSharedValueScanning()
{
  // test QgsAttributeForm::scanForEqualAttributes

  QSet<int> mixedValueFields;
  QHash<int, QVariant> fieldSharedValues;

  // make a temporary layer to check through
  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?field=col1:integer&field=col2:integer&field=col3:integer&field=col4:integer"_s, u"test"_s, u"memory"_s );
  QVERIFY( layer->isValid() );
  QgsFeature f1( layer->dataProvider()->fields(), 1 );
  f1.setAttribute( u"col1"_s, 1 );
  f1.setAttribute( u"col2"_s, 1 );
  f1.setAttribute( u"col3"_s, 3 );
  f1.setAttribute( u"col4"_s, 1 );
  QgsFeature f2( layer->dataProvider()->fields(), 2 );
  f2.setAttribute( u"col1"_s, 1 );
  f2.setAttribute( u"col2"_s, 2 );
  f2.setAttribute( u"col3"_s, 3 );
  f2.setAttribute( u"col4"_s, 2 );
  QgsFeature f3( layer->dataProvider()->fields(), 3 );
  f3.setAttribute( u"col1"_s, 1 );
  f3.setAttribute( u"col2"_s, 2 );
  f3.setAttribute( u"col3"_s, 3 );
  f3.setAttribute( u"col4"_s, 2 );
  QgsFeature f4( layer->dataProvider()->fields(), 4 );
  f4.setAttribute( u"col1"_s, 1 );
  f4.setAttribute( u"col2"_s, 1 );
  f4.setAttribute( u"col3"_s, 3 );
  f4.setAttribute( u"col4"_s, 2 );
  layer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 );

  const QgsAttributeForm form( layer );

  QgsFeatureIterator it = layer->getFeatures();

  form.scanForEqualAttributes( it, mixedValueFields, fieldSharedValues );

  QCOMPARE( mixedValueFields, QSet<int>() << 1 << 3 );
  QCOMPARE( fieldSharedValues.value( 0 ).toInt(), 1 );
  QCOMPARE( fieldSharedValues.value( 2 ).toInt(), 3 );

  // add another feature so all attributes are different
  QgsFeature f5( layer->dataProvider()->fields(), 5 );
  f5.setAttribute( u"col1"_s, 11 );
  f5.setAttribute( u"col2"_s, 11 );
  f5.setAttribute( u"col3"_s, 13 );
  f5.setAttribute( u"col4"_s, 12 );
  layer->dataProvider()->addFeatures( QgsFeatureList() << f5 );

  it = layer->getFeatures();

  form.scanForEqualAttributes( it, mixedValueFields, fieldSharedValues );
  QCOMPARE( mixedValueFields, QSet<int>() << 0 << 1 << 2 << 3 );
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

void TestQgsDualView::testNoGeom()
{
  //test that both the master model and cache for the dual view either both request geom or both don't request geom
  auto dv = std::make_unique<QgsDualView>();

  // request with geometry
  QgsFeatureRequest req;
  dv->init( mPointsLayer, mCanvas, req );
  // check that both master model AND cache are using geometry
  QgsAttributeTableModel *model = dv->masterModel();
  QVERIFY( model->layerCache()->cacheGeometry() );
  QVERIFY( !( model->request().flags() & Qgis::FeatureRequestFlag::NoGeometry ) );

  // request with NO geometry, but using filter rect (which should override and request geom)
  req = QgsFeatureRequest().setFilterRect( QgsRectangle( 1, 2, 3, 4 ) );
  dv = std::make_unique<QgsDualView>();
  dv->init( mPointsLayer, mCanvas, req );
  model = dv->masterModel();
  QVERIFY( model->layerCache()->cacheGeometry() );
  QVERIFY( !( model->request().flags() & Qgis::FeatureRequestFlag::NoGeometry ) );

  // request with NO geometry
  req = QgsFeatureRequest().setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  dv = std::make_unique<QgsDualView>();
  dv->init( mPointsLayer, mCanvas, req );
  model = dv->masterModel();
  QVERIFY( !model->layerCache()->cacheGeometry() );
  QVERIFY( ( model->request().flags() & Qgis::FeatureRequestFlag::NoGeometry ) );

  // request with NO geometry but with an ordering expression which does
  req = QgsFeatureRequest().setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  dv = std::make_unique<QgsDualView>();
  dv->init( mPointsLayer, mCanvas, req );
  auto config = mPointsLayer->attributeTableConfig();
  config.setSortExpression( "$x" );
  dv->setAttributeTableConfig( config );
  model = dv->masterModel();
  QVERIFY( model->layerCache()->cacheGeometry() );
  QVERIFY( !( model->request().flags() & Qgis::FeatureRequestFlag::NoGeometry ) );
}

void TestQgsDualView::testNoShowFirstFeature()
{
  auto dv = std::make_unique<QgsDualView>();

  QgsAttributeTableConfig config = mPointsLayer->attributeTableConfig();
  config.setSortExpression( u"\"Class\""_s );
  mPointsLayer->setAttributeTableConfig( config );

  QgsFeatureRequest req;
  dv->init( mPointsLayer, mCanvas, req, QgsAttributeEditorContext(), true, false );
  QCOMPARE( dv->mFeatureListModel->data( dv->mFeatureListModel->index( 0, 0 ), QgsFeatureListModel::Role::FeatureRole ).value<QgsFeature>().attribute( u"Class"_s ), u"B52"_s );

  config.setSortExpression( QString() );
  mPointsLayer->setAttributeTableConfig( config );
}

void TestQgsDualView::testDuplicateField()
{
  // test updating same field appearing in different widget

  // make a temporary vector layer
  const QString def = u"Point?field=col0:integer"_s;
  QgsVectorLayer *layer = new QgsVectorLayer( def, u"test"_s, u"memory"_s );
  layer->setEditorWidgetSetup( 0, QgsEditorWidgetSetup( u"Range"_s, QVariantMap() ) );

  // add same field twice so they get synced
  QgsEditFormConfig editFormConfig = layer->editFormConfig();
  editFormConfig.clearTabs();
  editFormConfig.invisibleRootContainer()->addChildElement( new QgsAttributeEditorField( "col0", 0, editFormConfig.invisibleRootContainer() ) );
  editFormConfig.invisibleRootContainer()->addChildElement( new QgsAttributeEditorField( "col0", 0, editFormConfig.invisibleRootContainer() ) );
  editFormConfig.setLayout( Qgis::AttributeFormLayout::DragAndDrop );
  layer->setEditFormConfig( editFormConfig );

  // add a feature to the vector layer
  QgsFeature ft( layer->dataProvider()->fields(), 1 );
  ft.setAttribute( u"col0"_s, 1 );
  layer->dataProvider()->addFeature( ft );

  QgsDualView dualView;
  dualView.init( layer, mCanvas );

  layer->startEditing();

  const QList<QgsAttributeFormEditorWidget *> formEditorWidgets = dualView.mAttributeForm->mFormEditorWidgets.values( 0 );

  // reset mIsChanged state
  formEditorWidgets[0]->changesCommitted();
  formEditorWidgets[1]->changesCommitted();
  QVERIFY( !formEditorWidgets[0]->hasChanged() );
  QVERIFY( !formEditorWidgets[1]->hasChanged() );

  formEditorWidgets[0]->editorWidget()->setValues( 20, QVariantList() );
  QCOMPARE( formEditorWidgets[0]->editorWidget()->value().toInt(), 20 );
  QCOMPARE( formEditorWidgets[1]->editorWidget()->value().toInt(), 20 );
  QVERIFY( formEditorWidgets[0]->hasChanged() );
  QVERIFY( formEditorWidgets[1]->hasChanged() );
  ft = layer->getFeature( ft.id() );
  QCOMPARE( ft.attribute( u"col0"_s ).toInt(), 20 );

  // reset mIsChanged state
  formEditorWidgets[0]->changesCommitted();
  formEditorWidgets[1]->changesCommitted();
  QVERIFY( !formEditorWidgets[0]->hasChanged() );
  QVERIFY( !formEditorWidgets[1]->hasChanged() );

  formEditorWidgets[1]->editorWidget()->setValues( 21, QVariantList() );
  QCOMPARE( formEditorWidgets[0]->editorWidget()->value().toInt(), 21 );
  QCOMPARE( formEditorWidgets[1]->editorWidget()->value().toInt(), 21 );
  QVERIFY( formEditorWidgets[0]->hasChanged() );
  QVERIFY( formEditorWidgets[1]->hasChanged() );
  ft = layer->getFeature( ft.id() );
  QCOMPARE( ft.attribute( u"col0"_s ).toInt(), 21 );

  layer->rollBack();
}


QGSTEST_MAIN( TestQgsDualView )
#include "testqgsdualview.moc"
