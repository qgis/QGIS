/***************************************************************************
    testqgsmaptooleditextraitems.cpp
    ---------------------
    begin                : 2025/09/17
    copyright            : (C) 2025 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsextraitemutils.h"
#include "qgslinesymbollayer.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolextraitem.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbol.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "testqgsmaptoolutils.h"

#include <qstringliteral.h>

class TestQgsMapToolExtraItem : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsMapToolExtraItem()
      : QgsTest( u"Extra Item Map Tool Tests"_s ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testSelectFeature();
    void testAddModifyExtraItems();

  private:
    void compareExtraItems( const QString &strExtraItems, const QList<std::tuple<double, double, double>> &expected );
    int nbRubberBandVisible() const;

    QObjectUniquePtr<QgsMapToolModifyExtraItems> mMapToolModifyExtraItems;
    QObjectUniquePtr<QgsMapToolAddExtraItem> mMapToolAddExtraItems;
    std::unique_ptr<QgsMapCanvas> mCanvas;
    std::unique_ptr<QgsVectorLayer> mLayer;
    QgsMarkerLineSymbolLayer *mSymbolLayer = new QgsMarkerLineSymbolLayer();
};

void TestQgsMapToolExtraItem::initTestCase()
{
  mCanvas = std::make_unique<QgsMapCanvas>();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3946"_s ) );
  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 25, 10 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();

  mLayer = std::make_unique<QgsVectorLayer>( u"MultiPolygon?field=pk:int&field=bas:string&crs=EPSG:3946"_s, u"polys"_s, u"memory"_s );
  QVERIFY( mLayer->isValid() );
  QCOMPARE( mLayer->fields().count(), 2 );

  const QString wkt1 = "MultiPolygon (((0 0, 10 0, 10 3, 12 4, 8 5, 12 6, 8 7, 10 8, 10 10, 0 10, 0 0),(2 3, 6 3, 6 8, 2 8, 2 3)),((13 0, 16 0, 16 10, 13 10, 13 0)))";
  QgsFeature f1;
  f1.setGeometry( QgsGeometry::fromWkt( wkt1 ) );

  const QString wkt2 = "MultiPolygon (((20 0, 25 0, 25 5, 20 5, 20 0)))";
  QgsFeature f2;
  f2.setGeometry( QgsGeometry::fromWkt( wkt2 ) );

  mLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 );
  QCOMPARE( mLayer->featureCount(), ( long ) 2 );
  QCOMPARE( mLayer->getFeature( 1 ).geometry().asWkt(), wkt1 );
  QCOMPARE( mLayer->getFeature( 2 ).geometry().asWkt(), wkt2 );

  mCanvas->setCurrentLayer( mLayer.get() );

  QgsSingleSymbolRenderer *renderer = dynamic_cast<QgsSingleSymbolRenderer *>( mLayer->renderer() );
  QVERIFY( renderer );
  QVERIFY( renderer->symbol() );

  mSymbolLayer = new QgsMarkerLineSymbolLayer();
  mSymbolLayer->setPlacements( Qgis::MarkerLinePlacement::Interval );

  renderer->symbol()->changeSymbolLayer( 0, mSymbolLayer );
}

void TestQgsMapToolExtraItem::cleanupTestCase()
{
}

void TestQgsMapToolExtraItem::init()
{
  mMapToolAddExtraItems.reset( new QgsMapToolAddExtraItem( mCanvas.get(), mLayer.get(), mSymbolLayer, 1 ) );
  mCanvas->setMapTool( mMapToolAddExtraItems );

  mMapToolModifyExtraItems.reset( new QgsMapToolModifyExtraItems( mCanvas.get(), mLayer.get(), mSymbolLayer, 1 ) );
  mCanvas->setMapTool( mMapToolModifyExtraItems );
}

void TestQgsMapToolExtraItem::cleanup()
{
  mCanvas->unsetMapTool( mMapToolModifyExtraItems );
  mMapToolModifyExtraItems.reset();
}

int TestQgsMapToolExtraItem::nbRubberBandVisible() const
{
  int result = 0;
  for ( QGraphicsItem *item : mCanvas->items() )
  {
    if ( QgsRubberBand *rubberBand = dynamic_cast<QgsRubberBand *>( item );
         rubberBand && rubberBand->isVisible() )
    {
      result++;
    }
  }

  return result;
}

void TestQgsMapToolExtraItem::compareExtraItems( const QString &strExtraItems, const QgsExtraItemUtils::ExtraItems &expectedExtraItems )
{
  QString error;
  const QgsExtraItemUtils::ExtraItems &extraItems = QgsExtraItemUtils::parseExtraItems( strExtraItems, error );
  QVERIFY( error.isEmpty() );

  QVERIFY2( expectedExtraItems.count() == extraItems.count(), u"Extra item number differs (Actual: %1 != Expected: %2). Returned extra items: %3"_s.arg( extraItems.count() ).arg( expectedExtraItems.count() ).arg( strExtraItems ).toLatin1().constData() );

  for ( int iExtraItem = 0; iExtraItem < expectedExtraItems.count(); iExtraItem++ )
  {
    QVERIFY2( qgsDoubleNear( std::get<0>( extraItems.at( iExtraItem ) ), std::get<0>( expectedExtraItems.at( iExtraItem ) ), 0.1 ), QString( "Value differs (Actual: %1 != Expected: %2) for X value of extra item %3. Returned extra items: %4" ).arg( std::get<0>( extraItems.at( iExtraItem ) ) ).arg( std::get<0>( expectedExtraItems.at( iExtraItem ) ) ).arg( iExtraItem ).arg( strExtraItems ).toLatin1().constData() );
    QVERIFY2( qgsDoubleNear( std::get<1>( extraItems.at( iExtraItem ) ), std::get<1>( expectedExtraItems.at( iExtraItem ) ), 0.1 ), QString( "Value differs (Actual: %1 != Expected: %2) for Y value of extra item %3. Returned extra items: %4" ).arg( std::get<1>( extraItems.at( iExtraItem ) ) ).arg( std::get<1>( expectedExtraItems.at( iExtraItem ) ) ).arg( iExtraItem ).arg( strExtraItems ).toLatin1().constData() );
    QVERIFY2( qgsDoubleNear( std::get<2>( extraItems.at( iExtraItem ) ), std::get<2>( expectedExtraItems.at( iExtraItem ) ), 0.1 ), QString( "Value differs (Actual: %1 != Expected: %2) for rotation angle of extra item %3. Returned extra items: %4" ).arg( std::get<2>( extraItems.at( iExtraItem ) ) ).arg( std::get<2>( expectedExtraItems.at( iExtraItem ) ) ).arg( iExtraItem ).arg( strExtraItems ).toLatin1().constData() );
  }
}

void TestQgsMapToolExtraItem::testSelectFeature()
{
  TestQgsMapToolUtils utils( mMapToolAddExtraItems.get() );

  QVERIFY( FID_IS_NULL( mMapToolAddExtraItems->mFeatureId ) );

  // select first feature
  utils.mouseClick( 1, 1, Qt::LeftButton );
  QCOMPARE( mMapToolAddExtraItems->mFeatureId, 1 );
  QCOMPARE( nbRubberBandVisible(), 0 );

  // escape
  utils.keyClick( Qt::Key_Escape );
  QVERIFY( FID_IS_NULL( mMapToolAddExtraItems->mFeatureId ) );
  QCOMPARE( nbRubberBandVisible(), 0 );

  // select second feature
  utils.mouseClick( 21, 1, Qt::LeftButton );
  QCOMPARE( mMapToolAddExtraItems->mFeatureId, 2 );

  // escape
  utils.keyClick( Qt::Key_Escape );
  QVERIFY( FID_IS_NULL( mMapToolAddExtraItems->mFeatureId ) );
  QCOMPARE( nbRubberBandVisible(), 0 );
}

void TestQgsMapToolExtraItem::testAddModifyExtraItems()
{
  TestQgsMapToolUtils utilsAdd( mMapToolAddExtraItems.get() );
  mLayer->startEditing();

  QVERIFY( FID_IS_NULL( mMapToolAddExtraItems->mFeatureId ) );

  // select first feature
  utilsAdd.mouseClick( 1, 1, Qt::LeftButton );
  QCOMPARE( mMapToolAddExtraItems->mFeatureId, 1 );
  QCOMPARE( nbRubberBandVisible(), 0 );

  // create 3 extra items
  utilsAdd.mouseClick( 2, 3, Qt::LeftButton );
  QCOMPARE( nbRubberBandVisible(), 1 );

  utilsAdd.mouseClick( 4, 5, Qt::LeftButton );
  QCOMPARE( nbRubberBandVisible(), 2 );

  utilsAdd.mouseClick( 6, 7, Qt::LeftButton );
  QCOMPARE( nbRubberBandVisible(), 3 );

  QgsFeature feat = mLayer->getFeature( 1 );
  QVERIFY( feat.isValid() );

  compareExtraItems( feat.attribute( 1 ).toString(), { { 2., 3., 0. }, { 4, 5, 0 }, { 6, 7, 0 } } );

  // escape
  utilsAdd.keyClick( Qt::Key_Escape );
  QVERIFY( FID_IS_NULL( mMapToolModifyExtraItems->mFeatureId ) );
  QCOMPARE( nbRubberBandVisible(), 0 );

  TestQgsMapToolUtils utilsModify( mMapToolModifyExtraItems.get() );
  mLayer->startEditing();

  QVERIFY( FID_IS_NULL( mMapToolModifyExtraItems->mFeatureId ) );

  // select first feature
  utilsModify.mouseClick( 1, 1, Qt::LeftButton );
  QCOMPARE( mMapToolModifyExtraItems->mFeatureId, 1 );
  QCOMPARE( nbRubberBandVisible(), 3 );

  // modify second extra item
  utilsModify.mouseClick( 4, 5, Qt::LeftButton );
  QCOMPARE( mMapToolModifyExtraItems->mFeatureId, 1 );
  QCOMPARE( nbRubberBandVisible(), 3 );
  QVERIFY( mMapToolModifyExtraItems->mMouseHandles );
  QCOMPARE( mMapToolModifyExtraItems->mMouseHandles->selectedSceneItems().count(), 1 );

  const double mupp = mCanvas->mapSettings().mapUnitsPerPixel();

  QGraphicsItem *item = mMapToolModifyExtraItems->mMouseHandles->selectedSceneItems().first();
  QVERIFY( item );
  mMapToolModifyExtraItems->mMouseHandles->moveItem( item, 1 / mupp, -3 / mupp );
  mMapToolModifyExtraItems->mMouseHandles->rotateItem( item, 42, 2 / mupp, -4 / mupp );

  feat = mLayer->getFeature( 1 );
  QVERIFY( feat.isValid() );
  compareExtraItems( feat.attribute( 1 ).toString(), { { 2., 3., 0. }, { 7 /* 4 + 1 + 2 */, 12 /* 5 + 3 + 4 */, 42 }, { 6, 7, 0 } } );

  utilsModify.keyClick( Qt::Key_Delete );
  QCOMPARE( nbRubberBandVisible(), 2 );
  QVERIFY( mMapToolModifyExtraItems->mMouseHandles );
  QCOMPARE( mMapToolModifyExtraItems->mMouseHandles->selectedSceneItems().count(), 0 );

  feat = mLayer->getFeature( 1 );
  QVERIFY( feat.isValid() );
  compareExtraItems( feat.attribute( 1 ).toString(), { { 2., 3., 0. }, { 6, 7, 0 } } );

  // select first extra item
  utilsModify.mouseClick( 2, 3, Qt::LeftButton );
  QCOMPARE( mMapToolModifyExtraItems->mFeatureId, 1 );
  QCOMPARE( nbRubberBandVisible(), 2 );
  QVERIFY( mMapToolModifyExtraItems->mMouseHandles );
  QCOMPARE( mMapToolModifyExtraItems->mMouseHandles->selectedSceneItems().count(), 1 );

  // escape (items are no longer selected)
  utilsModify.keyClick( Qt::Key_Escape );
  QCOMPARE( mMapToolModifyExtraItems->mFeatureId, 1 );
  QCOMPARE( nbRubberBandVisible(), 2 );
  QCOMPARE( mMapToolModifyExtraItems->mMouseHandles->selectedSceneItems().count(), 0 );

  // escape (feature is no longer selected)
  utilsModify.keyClick( Qt::Key_Escape );
  QVERIFY( FID_IS_NULL( mMapToolModifyExtraItems->mFeatureId ) );
  QCOMPARE( nbRubberBandVisible(), 0 );

  mLayer->rollBack();
}

QGSTEST_MAIN( TestQgsMapToolExtraItem )
#include "testqgsmaptoolextraitem.moc"
