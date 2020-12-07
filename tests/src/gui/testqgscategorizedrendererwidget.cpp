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

#include "qgstest.h"

#include "qgscategorizedsymbolrenderer.h"
#include "qgscategorizedsymbolrendererwidget.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsvectordataprovider.h"

#include <memory>

class TestQgsCategorizedRendererWidget : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
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
  std::unique_ptr< QgsVectorLayer > vl = qgis::make_unique< QgsVectorLayer >( "Point?crs=EPSG:4326&field=idx:integer&field=name:string", QString(), QStringLiteral( "memory" ) );
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

  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer( QStringLiteral( "name" ) );
  vl->setRenderer( renderer );

  std::unique_ptr< QgsCategorizedSymbolRendererWidget > widget = qgis::make_unique< QgsCategorizedSymbolRendererWidget >( vl.get(), nullptr, renderer );
  QVERIFY( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().isEmpty() );

  widget->addCategories();
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().count(), 5 );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).value().toString(), QStringLiteral( "a" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).value().toString(), QStringLiteral( "b" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 2 ).value().toString(), QStringLiteral( "c" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 3 ).value().toString(), QStringLiteral( "d" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 4 ).value().toString(), QString() );

  // add a new value
  f.setAttributes( QgsAttributes() << 4 << "e" );
  vl->dataProvider()->addFeature( f );

  widget->addCategories();
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().count(), 6 );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 5 ).value().toString(), QStringLiteral( "e" ) );

  // test with a value list category
  widget.reset();
  renderer = new QgsCategorizedSymbolRenderer( QStringLiteral( "name" ) );
  renderer->addCategory( QgsRendererCategory( QStringLiteral( "b" ), QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), QString() ) );
  renderer->addCategory( QgsRendererCategory( QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "c" ), QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), QString() ) );

  vl->setRenderer( renderer );

  widget = qgis::make_unique< QgsCategorizedSymbolRendererWidget >( vl.get(), nullptr, renderer );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().count(), 2 );

  // values inside list categories should not be re-added
  widget->addCategories();
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().count(), 5 );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).value().toString(), QStringLiteral( "b" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).value().toList().at( 0 ).toString(), QStringLiteral( "a" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).value().toList().at( 1 ).toString(), QStringLiteral( "c" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 2 ).value().toString(), QStringLiteral( "d" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 3 ).value().toString(), QStringLiteral( "e" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 4 ).value().toString(), QString() );
}

void TestQgsCategorizedRendererWidget::merge()
{
  // test merging categories

  std::unique_ptr< QgsVectorLayer > vl = qgis::make_unique< QgsVectorLayer >( "Point?crs=EPSG:4326&field=idx:integer&field=name:string", QString(), QStringLiteral( "memory" ) );
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

  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer( QStringLiteral( "name" ) );
  vl->setRenderer( renderer );

  std::unique_ptr< QgsCategorizedSymbolRendererWidget > widget = qgis::make_unique< QgsCategorizedSymbolRendererWidget >( vl.get(), nullptr, renderer );
  widget->addCategories();
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().count(), 6 );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).value().toString(), QStringLiteral( "a" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).value().toString(), QStringLiteral( "b" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 2 ).value().toString(), QStringLiteral( "c" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 3 ).value().toString(), QStringLiteral( "d" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 4 ).value().toString(), QStringLiteral( "e" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 5 ).value().toString(), QString() );

  // no selection, should have no effect
  widget->mergeSelectedCategories();
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().count(), 6 );

  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 1, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  // one selection, should have no effect
  widget->mergeSelectedCategories();
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().count(), 6 );

  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 3, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 4, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  widget->mergeSelectedCategories();
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().count(), 4 );

  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).value().toString(), QStringLiteral( "a" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).value().toList().at( 0 ).toString(), QStringLiteral( "b" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).value().toList().at( 1 ).toString(), QStringLiteral( "d" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).value().toList().at( 2 ).toString(), QStringLiteral( "e" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 2 ).value().toString(), QStringLiteral( "c" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 3 ).value().toString(), QString() );

  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).label(), QStringLiteral( "a" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).label(), QStringLiteral( "b,d,e" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 2 ).label(), QStringLiteral( "c" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 3 ).label(), QString() );

  // selection should always "merge into" first selected item
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 2, 0 ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 0, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  widget->mergeSelectedCategories();
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().count(), 3 );

  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).value().toList().at( 0 ).toString(), QStringLiteral( "b" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).value().toList().at( 1 ).toString(), QStringLiteral( "d" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).value().toList().at( 2 ).toString(), QStringLiteral( "e" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).value().toList().at( 0 ).toString(), QStringLiteral( "c" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).value().toList().at( 1 ).toString(), QStringLiteral( "a" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 2 ).value().toString(), QString() );

  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).label(), QStringLiteral( "b,d,e" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).label(), QStringLiteral( "c,a" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 2 ).label(), QString() );

  // merging categories which are already lists
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 1, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  //"" entry should be ignored
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 2, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows );
  widget->mergeSelectedCategories();

  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().count(), 2 );

  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).value().toList().at( 0 ).toString(), QStringLiteral( "b" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).value().toList().at( 1 ).toString(), QStringLiteral( "d" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).value().toList().at( 2 ).toString(), QStringLiteral( "e" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).value().toList().at( 3 ).toString(), QStringLiteral( "c" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).value().toList().at( 4 ).toString(), QStringLiteral( "a" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).value().toString(), QString() );

  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).label(), QStringLiteral( "b,d,e,c,a" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).label(), QString() );

  widget->viewCategories->selectionModel()->clearSelection();
  // unmerge
  widget->unmergeSelectedCategories();
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().count(), 2 );
  // not a list
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 1, 0 ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  widget->unmergeSelectedCategories();
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().count(), 2 );
  // list
  widget->viewCategories->selectionModel()->select( widget->viewCategories->model()->index( 0, 0 ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  widget->unmergeSelectedCategories();

  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().count(), 6 );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).value().toString(), QStringLiteral( "b" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).value().toString(), QString() );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 2 ).value().toString(), QStringLiteral( "d" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 3 ).value().toString(), QStringLiteral( "e" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 4 ).value().toString(), QStringLiteral( "c" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 5 ).value().toString(), QStringLiteral( "a" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).label(), QStringLiteral( "b" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).label(), QString() );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 2 ).label(), QStringLiteral( "d" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 3 ).label(), QStringLiteral( "e" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 4 ).label(), QStringLiteral( "c" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 5 ).label(), QStringLiteral( "a" ) );

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
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().count(), 6 );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).value().toString(), QStringLiteral( "b" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).value().toString(), QString() );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 2 ).value().toString(), QStringLiteral( "d" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 3 ).value().toString(), QStringLiteral( "c" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 4 ).value().toString(), QStringLiteral( "e" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 5 ).value().toString(), QStringLiteral( "a" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 0 ).label(), QStringLiteral( "b" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 1 ).label(), QString() );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 2 ).label(), QStringLiteral( "d" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 3 ).label(), QStringLiteral( "c" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 4 ).label(), QStringLiteral( "e" ) );
  QCOMPARE( static_cast< QgsCategorizedSymbolRenderer * >( widget->renderer() )->categories().at( 5 ).label(), QStringLiteral( "a" ) );
}

void TestQgsCategorizedRendererWidget::model()
{
  std::unique_ptr< QgsVectorLayer > vl = qgis::make_unique< QgsVectorLayer >( "Point?crs=EPSG:4326&field=idx:integer&field=name:string", QString(), QStringLiteral( "memory" ) );
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

  QgsCategorizedSymbolRenderer *renderer = new QgsCategorizedSymbolRenderer( QStringLiteral( "name" ) );
  renderer = new QgsCategorizedSymbolRenderer( QStringLiteral( "name" ) );
  renderer->addCategory( QgsRendererCategory( QStringLiteral( "b" ), QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), QStringLiteral( "aa" ) ) );
  renderer->addCategory( QgsRendererCategory( QVariantList() << QStringLiteral( "a" ) << QStringLiteral( "c" ), QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), QStringLiteral( "list" ) ) );
  renderer->addCategory( QgsRendererCategory( QStringLiteral( "d" ), QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ), QStringLiteral( "dd" ), false ) );

  vl->setRenderer( renderer );

  std::unique_ptr< QgsCategorizedSymbolRendererWidget > widget = qgis::make_unique< QgsCategorizedSymbolRendererWidget >( vl.get(), nullptr, renderer );
  QgsCategorizedSymbolRendererModel *model = widget->mModel;
  QCOMPARE( model->rowCount(), 3 );
  QCOMPARE( model->data( model->index( 0, 1 ), Qt::DisplayRole ).toString(), QStringLiteral( "b" ) );
  QCOMPARE( model->data( model->index( 1, 1 ), Qt::DisplayRole ).toString(), QStringLiteral( "a;c" ) );
  QCOMPARE( model->data( model->index( 2, 1 ), Qt::DisplayRole ).toString(), QStringLiteral( "d" ) );
  QCOMPARE( model->data( model->index( 0, 2 ), Qt::DisplayRole ).toString(), QStringLiteral( "aa" ) );
  QCOMPARE( model->data( model->index( 1, 2 ), Qt::DisplayRole ).toString(), QStringLiteral( "list" ) );
  QCOMPARE( model->data( model->index( 2, 2 ), Qt::DisplayRole ).toString(), QStringLiteral( "dd" ) );

  QCOMPARE( model->data( model->index( 0, 0 ), Qt::CheckStateRole ).toInt(), static_cast< int >( Qt::Checked ) );
  QCOMPARE( model->data( model->index( 1, 0 ), Qt::CheckStateRole ).toInt(), static_cast< int >( Qt::Checked ) );
  QCOMPARE( model->data( model->index( 2, 0 ), Qt::CheckStateRole ).toInt(), static_cast< int >( Qt::Unchecked ) );
}

QGSTEST_MAIN( TestQgsCategorizedRendererWidget )
#include "testqgscategorizedrendererwidget.moc"
