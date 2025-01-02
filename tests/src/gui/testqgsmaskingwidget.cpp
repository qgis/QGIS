/***************************************************************************
    testqgsmaskingwidget.cpp
    ---------------------
    begin                : 2024/06/05
    copyright            : (C) 2024 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QTreeWidget>

#include "qgscategorizedsymbolrenderer.h"
#include "qgsmarkersymbol.h"
#include "qgsmasksymbollayer.h"
#include "qgsmaskingwidget.h"
#include "qgsproject.h"
#include "qgssinglesymbolrenderer.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QElapsedTimer>

class TestQgsMaskingWidget : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsMaskingWidget()
      : QgsTest( QStringLiteral( "Masking widget Tests" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testTreeWidget();
};

void TestQgsMaskingWidget::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsMaskingWidget::cleanupTestCase()
{
}

void TestQgsMaskingWidget::init()
{
}

void TestQgsMaskingWidget::cleanup()
{
}

void TestQgsMaskingWidget::testTreeWidget()
{
  const QString projectFilePath = testDataPath( QStringLiteral( "selective_masking.qgs" ) );
  QVERIFY( QgsProject::instance()->read( projectFilePath ) );

  // get layers
  QList<QgsMapLayer *> layers = QgsProject::instance()->mapLayersByName( "lines_with_labels" );
  QCOMPARE( layers.count(), 1 );
  QgsVectorLayer *linesWithLabels = qobject_cast<QgsVectorLayer *>( layers.at( 0 ) );
  QVERIFY( linesWithLabels );

  layers = QgsProject::instance()->mapLayersByName( "polys" );
  QCOMPARE( layers.count(), 1 );
  QgsVectorLayer *polys = qobject_cast<QgsVectorLayer *>( layers.at( 0 ) );
  QVERIFY( polys );

  layers = QgsProject::instance()->mapLayersByName( "points" );
  QCOMPARE( layers.count(), 1 );
  QgsVectorLayer *points = qobject_cast<QgsVectorLayer *>( layers.at( 0 ) );
  QVERIFY( points );

  // add masks on polys label and B52 points
  QgsPalLayerSettings *labelSettings = new QgsPalLayerSettings( polys->labeling()->settings() );
  QgsTextFormat format = labelSettings->format();
  format.mask().setEnabled( true );
  labelSettings->setFormat( format );
  polys->labeling()->setSettings( labelSettings );

  QgsMaskMarkerSymbolLayer *maskLayer = new QgsMaskMarkerSymbolLayer();
  maskLayer->setSubSymbol( QgsMarkerSymbol::createSimple( { { QStringLiteral( "size" ), 6 } } ) );
  QgsCategorizedSymbolRenderer *renderer = dynamic_cast<QgsCategorizedSymbolRenderer *>( points->renderer() );
  QVERIFY( renderer );
  const QgsCategoryList categories = renderer->categories();
  QCOMPARE( categories.count(), 3 );
  QCOMPARE( categories.at( 0 ).label(), QStringLiteral( "B52" ) );
  QgsSymbol *symbol = categories.at( 0 ).symbol();
  QVERIFY( symbol );
  symbol->appendSymbolLayer( maskLayer );
  QCOMPARE( maskLayer->masks().count(), 0 );

  // update masking widget
  std::unique_ptr<QgsMaskingWidget> mw = std::make_unique<QgsMaskingWidget>();
  QElapsedTimer timer;
  timer.start();
  mw->setLayer( linesWithLabels );

  mw->populate();

  QVERIFY( mw->mMaskTargetsWidget );
  QVERIFY( mw->mMaskSourcesWidget->mTree );
  QCOMPARE( mw->mMaskSourcesWidget->mTree->topLevelItemCount(), 2 );

  // check masking symbol, first branch : points > B52 > Mask symbol layer
  QTreeWidgetItem *item = mw->mMaskSourcesWidget->mTree->topLevelItem( 0 );
  QCOMPARE( item->text( 0 ), QStringLiteral( "points" ) );

  QCOMPARE( item->childCount(), 1 );
  item = item->child( 0 );
  QCOMPARE( item->text( 0 ), QStringLiteral( "B52" ) );

  QCOMPARE( item->childCount(), 1 );
  item = item->child( 0 );
  QCOMPARE( item->text( 0 ), QStringLiteral( "Mask symbol layer" ) );
  QTreeWidgetItem *pointMaskItem = item;

  // check masking symbol, second branch : polys > Label mask
  item = mw->mMaskSourcesWidget->mTree->topLevelItem( 1 );
  QCOMPARE( item->text( 0 ), QStringLiteral( "polys" ) );

  QCOMPARE( item->childCount(), 1 );
  item = item->child( 0 );
  QCOMPARE( item->text( 0 ), QStringLiteral( "Label mask" ) );

  // check masked symbol, one branch, 2 children
  QVERIFY( mw->mMaskTargetsWidget->mTree );
  QCOMPARE( mw->mMaskTargetsWidget->mTree->topLevelItemCount(), 1 );

  item = mw->mMaskTargetsWidget->mTree->topLevelItem( 0 );
  QCOMPARE( item->text( 0 ), QString() );
  QCOMPARE( item->childCount(), 2 );
  QTreeWidgetItem *blackLineItem = item->child( 0 );
  QCOMPARE( blackLineItem->text( 0 ), QString() );
  item = item->child( 1 );
  QCOMPARE( item->text( 0 ), QString() );

  // check 2 items
  QVERIFY( mw->mMessageBar );
  QVERIFY( !mw->mMessageBar->currentItem() );

  blackLineItem->setCheckState( 0, Qt::Checked );
  QVERIFY( mw->mMessageBar->currentItem() );

  pointMaskItem->setCheckState( 0, Qt::Checked );
  QVERIFY( !mw->mMessageBar->currentItem() );

  mw->apply();

  // check that mask have been set
  const QList<QgsSymbolLayerReference> maskedSls = maskLayer->masks();
  QCOMPARE( maskedSls.count(), 1 );
  const QgsSymbolLayerReference maskedSl = maskedSls.at( 0 );
  QCOMPARE( maskedSl.layerId(), linesWithLabels->id() );

  const QgsSingleSymbolRenderer *lineRenderer = dynamic_cast<QgsSingleSymbolRenderer *>( linesWithLabels->renderer() );
  QVERIFY( lineRenderer );
  const QgsSymbol *lineSymbol = lineRenderer->symbol();
  QVERIFY( lineSymbol );
  const QgsSymbolLayerList sls = lineSymbol->symbolLayers();
  QCOMPARE( sls.count(), 2 );
  QgsSymbolLayer *lineSl = sls.at( 0 );
  QVERIFY( lineSl );

  QCOMPARE( maskedSl.symbolLayerIdV2(), lineSl->id() );
}


QGSTEST_MAIN( TestQgsMaskingWidget )
#include "testqgsmaskingwidget.moc"
