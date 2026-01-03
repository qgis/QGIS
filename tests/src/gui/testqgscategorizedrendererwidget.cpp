/***************************************************************************
                         testqgscategorizedrendererwidget.cpp
                         ---------------------------
    begin                : January 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <memory>

#include "qgsapplication.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgscategorizedsymbolrendererwidget.h"
#include "qgsfeature.h"
#include "qgssymbol.h"
#include "qgstest.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

class TestQgsCategorizedRendererWidget : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void testAddMissingCategories();
    void merge();
    void model();
};

void TestQgsCategorizedRendererWidget::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsCategorizedRendererWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsCategorizedRendererWidget::init()
{
}

void TestQgsCategorizedRendererWidget::cleanup()
{
}

void TestQgsCategorizedRendererWidget::testAddMissingCategories()
{
  auto vl = std::make_unique<QgsVectorLayer>( "Point?crs=EPSG:4326&field=idx:integer&field=name:string", QString(), u"memory"_s );
  QVERIFY( vl->isValid() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << "a" );
  QVERIFY( vl->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << "b" );
  vl->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 3 << "c" );
  vl->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 4 << "d" );
  vl->dataProvider()->addFeature( f );

  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer( u"name"_s );
  vl->setRenderer( renderer );

  auto widget = std::make_unique<QgsCategorizedSymbolRendererWidget>( vl.get(), nullptr, renderer );
  QVERIFY( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().isEmpty() );

  widget->addCategories();
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().count(), 5 );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).value().toString(), u"a"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).value().toString(), u"b"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 2 ).value().toString(), u"c"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 3 ).value().toString(), u"d"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 4 ).value().toString(), QString() );

  // add a new value
  f.setAttributes( QgsAttributes() << 4 << "e" );
  vl->dataProvider()->addFeature( f );

  widget->addCategories();
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().count(), 6 );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 5 ).value().toString(), u"e"_s );

  // test with a value list category
  widget.reset();
  renderer = new QgsCategorizedSymbolRenderer( u"name"_s );
  renderer->addCategory( QgsRendererCategory( u"b"_s, QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), QString() ) );
  renderer->addCategory( QgsRendererCategory( QVariantList() << u"a"_s << u"c"_s, QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), QString() ) );

  vl->setRenderer( renderer );

  widget = std::make_unique<QgsCategorizedSymbolRendererWidget>( vl.get(), nullptr, renderer );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().count(), 2 );

  // values inside list categories should not be re-added
  widget->addCategories();
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().count(), 5 );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).value().toString(), u"b"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).value().toList().at( 0 ).toString(), u"a"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).value().toList().at( 1 ).toString(), u"c"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 2 ).value().toString(), u"d"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 3 ).value().toString(), u"e"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 4 ).value().toString(), QString() );
}

void TestQgsCategorizedRendererWidget::merge()
{
  // test merging categories

  auto vl = std::make_unique<QgsVectorLayer>( "Point?crs=EPSG:4326&field=idx:integer&field=name:string", QString(), u"memory"_s );
  QVERIFY( vl->isValid() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << "a" );
  QVERIFY( vl->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << "b" );
  vl->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 3 << "c" );
  vl->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 4 << "d" );
  vl->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 5 << "e" );
  vl->dataProvider()->addFeature( f );

  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer( u"name"_s );
  vl->setRenderer( renderer );

  auto widget = std::make_unique<QgsCategorizedSymbolRendererWidget>( vl.get(), nullptr, renderer );
  widget->addCategories();
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().count(), 6 );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).value().toString(), u"a"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).value().toString(), u"b"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 2 ).value().toString(), u"c"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 3 ).value().toString(), u"d"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 4 ).value().toString(), u"e"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 5 ).value().toString(), QString() );

  // no selection, should have no effect
  widget->mergeSelectedCategories();
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().count(), 6 );

  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 1, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  // one selection, should have no effect
  widget->mergeSelectedCategories();
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().count(), 6 );

  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 3, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 4, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  widget->mergeSelectedCategories();
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().count(), 4 );

  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).value().toString(), u"a"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).value().toList().at( 0 ).toString(), u"b"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).value().toList().at( 1 ).toString(), u"d"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).value().toList().at( 2 ).toString(), u"e"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 2 ).value().toString(), u"c"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 3 ).value().toString(), QString() );

  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).label(), u"a"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).label(), u"b,d,e"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 2 ).label(), u"c"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 3 ).label(), QString() );

  // selection should always "merge into" first selected item
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 2, 0 ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 0, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  widget->mergeSelectedCategories();
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().count(), 3 );

  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).value().toList().at( 0 ).toString(), u"b"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).value().toList().at( 1 ).toString(), u"d"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).value().toList().at( 2 ).toString(), u"e"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).value().toList().at( 0 ).toString(), u"c"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).value().toList().at( 1 ).toString(), u"a"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 2 ).value().toString(), QString() );

  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).label(), u"b,d,e"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).label(), u"c,a"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 2 ).label(), QString() );

  // merging categories which are already lists
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 1, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  //"" entry should be ignored
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 2, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  widget->mergeSelectedCategories();

  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().count(), 2 );

  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).value().toList().at( 0 ).toString(), u"b"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).value().toList().at( 1 ).toString(), u"d"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).value().toList().at( 2 ).toString(), u"e"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).value().toList().at( 3 ).toString(), u"c"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).value().toList().at( 4 ).toString(), u"a"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).value().toString(), QString() );

  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).label(), u"b,d,e,c,a"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).label(), QString() );

  widget->viewCategories->selectionModel()->clearSelection();
  // unmerge
  widget->unmergeSelectedCategories();
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().count(), 2 );
  // not a list
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 1, 0 ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  widget->unmergeSelectedCategories();
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().count(), 2 );
  // list
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  widget->unmergeSelectedCategories();

  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().count(), 6 );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).value().toString(), u"b"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).value().toString(), QString() );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 2 ).value().toString(), u"d"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 3 ).value().toString(), u"e"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 4 ).value().toString(), u"c"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 5 ).value().toString(), u"a"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).label(), u"b"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).label(), QString() );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 2 ).label(), u"d"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 3 ).label(), u"e"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 4 ).label(), u"c"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 5 ).label(), u"a"_s );

  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 2, 0 ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 3, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  widget->mergeSelectedCategories();
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 3, 0 ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 4, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  widget->mergeSelectedCategories();
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 1, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 2, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 3, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  widget->unmergeSelectedCategories();
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().count(), 6 );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).value().toString(), u"b"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).value().toString(), QString() );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 2 ).value().toString(), u"d"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 3 ).value().toString(), u"c"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 4 ).value().toString(), u"e"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 5 ).value().toString(), u"a"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 0 ).label(), u"b"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 1 ).label(), QString() );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 2 ).label(), u"d"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 3 ).label(), u"c"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 4 ).label(), u"e"_s );
  QCOMPARE( static_cast<QgsCategorizedSymbolRenderer *>( widget->renderer() )->categories().at( 5 ).label(), u"a"_s );
}

void TestQgsCategorizedRendererWidget::model()
{
  auto vl = std::make_unique<QgsVectorLayer>( "Point?crs=EPSG:4326&field=idx:integer&field=name:string", QString(), u"memory"_s );
  QVERIFY( vl->isValid() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << "a" );
  QVERIFY( vl->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 << "b" );
  vl->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 3 << "c" );
  vl->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 4 << "d" );
  vl->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 5 << "e" );
  vl->dataProvider()->addFeature( f );

  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer( u"name"_s );
  renderer = new QgsCategorizedSymbolRenderer( u"name"_s );
  renderer->addCategory( QgsRendererCategory( u"b"_s, QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), u"aa"_s ) );
  renderer->addCategory( QgsRendererCategory( QVariantList() << u"a"_s << u"c"_s, QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), u"list"_s ) );
  renderer->addCategory( QgsRendererCategory( u"d"_s, QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), u"dd"_s, false ) );

  vl->setRenderer( renderer );

  auto widget = std::make_unique<QgsCategorizedSymbolRendererWidget>( vl.get(), nullptr, renderer );
  QgsCategorizedSymbolRendererModel *model = widget->mModel;
  QCOMPARE( model->rowCount(), 3 );
  QCOMPARE( model->data( model->index( 0, 1 ), Qt::DisplayRole ).toString(), u"b"_s );
  QCOMPARE( model->data( model->index( 1, 1 ), Qt::DisplayRole ).toString(), u"a;c"_s );
  QCOMPARE( model->data( model->index( 2, 1 ), Qt::DisplayRole ).toString(), u"d"_s );
  QCOMPARE( model->data( model->index( 0, 2 ), Qt::DisplayRole ).toString(), u"aa"_s );
  QCOMPARE( model->data( model->index( 1, 2 ), Qt::DisplayRole ).toString(), u"list"_s );
  QCOMPARE( model->data( model->index( 2, 2 ), Qt::DisplayRole ).toString(), u"dd"_s );

  QCOMPARE( model->data( model->index( 0, 0 ), Qt::CheckStateRole ).toInt(), static_cast<int>( Qt::Checked ) );
  QCOMPARE( model->data( model->index( 1, 0 ), Qt::CheckStateRole ).toInt(), static_cast<int>( Qt::Checked ) );
  QCOMPARE( model->data( model->index( 2, 0 ), Qt::CheckStateRole ).toInt(), static_cast<int>( Qt::Unchecked ) );
}

QGSTEST_MAIN( TestQgsCategorizedRendererWidget )
#include "testqgscategorizedrendererwidget.moc"
