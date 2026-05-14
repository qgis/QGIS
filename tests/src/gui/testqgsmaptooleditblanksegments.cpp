/***************************************************************************
    testqgsmaptooleditblankareas.cpp
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

#include "qgsauxiliarystorage.h"
#include "qgslinesymbollayer.h"
#include "qgsmapcanvas.h"
#include "qgsmaptooleditblanksegments.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbollayer.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "testqgsmaptoolutils.h"

#include <QString>
#include <qstringliteral.h>

using namespace Qt::StringLiterals;

class TestQgsMapToolEditBlankSegments : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsMapToolEditBlankSegments()
      : QgsTest( u"Blank Segments Map Tool Tests"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testSelectFeature();
    void testCreateBlankSegment();
    void testAuxiliaryStorageAutoCreation();

  private:
    void compareBlankSegments( const QString &strBlankSegments, const QList<QList<QgsSymbolLayerUtils::BlankSegments>> &expected );
    int nbRubberBandVisible() const;

    QObjectUniquePtr<QgsMapToolEditBlankSegments<QgsMarkerLineSymbolLayer>> mMapToolEditBlankSegments;
    std::unique_ptr<QgsMapCanvas> mCanvas;
    std::unique_ptr<QgsVectorLayer> mLayer;
    std::unique_ptr<QgsPropertyOverrideButton> mPropertyButton;
    QgsMarkerLineSymbolLayer *mSymbolLayer = new QgsMarkerLineSymbolLayer();
};

void TestQgsMapToolEditBlankSegments::initTestCase()
{
  mCanvas = std::make_unique<QgsMapCanvas>();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3946"_s ) );
  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 25, 10 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();

  mLayer = std::make_unique<QgsVectorLayer>( u"MultiPolygon?field=pk:int&field=the_blank_segments:string&crs=EPSG:3946"_s, u"polys"_s, u"memory"_s );
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

  // initialize data defined property
  const int propertyKey = static_cast<int>( QgsSymbolLayer::Property::BlankSegments );
  QgsPropertyCollection c = mSymbolLayer->dataDefinedProperties();
  QgsProperty property = QgsProperty::fromField( "the_blank_segments", true );
  c.setProperty( propertyKey, property );
  mSymbolLayer->setDataDefinedProperties( c );

  mPropertyButton = std::make_unique<QgsPropertyOverrideButton>( nullptr, mLayer.get() );
  mPropertyButton->init( propertyKey, mSymbolLayer->dataDefinedProperties(), QgsSymbolLayer::propertyDefinitions(), mLayer.get(), true );
}

void TestQgsMapToolEditBlankSegments::cleanupTestCase()
{}

void TestQgsMapToolEditBlankSegments::init()
{
  mMapToolEditBlankSegments.reset( new QgsMapToolEditBlankSegments<QgsMarkerLineSymbolLayer>( mCanvas.get(), mLayer.get(), mSymbolLayer, mPropertyButton.get() ) );
  mCanvas->setMapTool( mMapToolEditBlankSegments );
}

void TestQgsMapToolEditBlankSegments::cleanup()
{
  mCanvas->unsetMapTool( mMapToolEditBlankSegments );
  mMapToolEditBlankSegments.reset();
}

int TestQgsMapToolEditBlankSegments::nbRubberBandVisible() const
{
  int result = 0;
  for ( QGraphicsItem *item : mCanvas->items() )
  {
    if ( QgsRubberBand *rubberBand = dynamic_cast<QgsRubberBand *>( item ); rubberBand && rubberBand->isVisible() )
    {
      result++;
    }
  }

  return result;
}

void TestQgsMapToolEditBlankSegments::compareBlankSegments( const QString &strBlankSegments, const QList<QList<QgsSymbolLayerUtils::BlankSegments>> &expectedBlankSegments )
{
  QString error;
  QList<QList<QgsSymbolLayerUtils::BlankSegments>> blankSegments = QgsSymbolLayerUtils::parseBlankSegments( strBlankSegments, QgsRenderContext(), Qgis::RenderUnit::Pixels, error );
  QVERIFY( error.isEmpty() );

  QVERIFY2(
    expectedBlankSegments.count() == blankSegments.count(),
    u"Part number differs (Actual: %1 != Expected: %2). Returned blank segments: %3"_s.arg( blankSegments.count() ).arg( expectedBlankSegments.count() ).arg( strBlankSegments ).toLatin1().constData()
  );

  for ( int iPart = 0; iPart < expectedBlankSegments.count(); iPart++ )
  {
    const QList<QgsSymbolLayerUtils::BlankSegments> &expectedRings = expectedBlankSegments.at( iPart );
    const QList<QgsSymbolLayerUtils::BlankSegments> &rings = blankSegments.at( iPart );
    QVERIFY2(
      expectedRings.count() == rings.count(),
      u"Rings number differs (Actual: %1 != Expected: %2) for part %3. Returned blank segments: %4"_s.arg( rings.count() ).arg( expectedRings.count() ).arg( iPart ).arg( strBlankSegments ).toLatin1().constData()
    );

    for ( int iRing = 0; iRing < rings.count(); iRing++ )
    {
      const QgsSymbolLayerUtils::BlankSegments &expectedSegments = expectedRings.at( iRing );
      const QgsSymbolLayerUtils::BlankSegments &segments = rings.at( iRing );
      QVERIFY2(
        expectedSegments.count() == segments.count(),
        u"Segments number differs (Actual: %1 != Expected: %2) for part %3 and ring %4. Returned blank segments: %5"_s.arg( segments.count() )
          .arg( expectedSegments.count() )
          .arg( iPart )
          .arg( iRing )
          .arg( strBlankSegments )
          .toLatin1()
          .constData()
      );
      for ( int iSegment = 0; iSegment < segments.count(); iSegment++ )
      {
        QVERIFY2(
          qgsDoubleNear( segments.at( iSegment ).first, expectedSegments.at( iSegment ).first, 0.1 ),
          QString( "Value differs (Actual: %1 != Expected: %2) for part %3, ring %4, segment %5, start distance. Returned blank segments: %6" )
            .arg( segments.at( iSegment ).first )
            .arg( expectedSegments.at( iSegment ).first )
            .arg( iPart )
            .arg( iRing )
            .arg( iSegment )
            .arg( strBlankSegments )
            .toLatin1()
            .constData()
        );

        QVERIFY2(
          qgsDoubleNear( segments.at( iSegment ).second, expectedSegments.at( iSegment ).second, 0.1 ),
          QString( "Value differs (Actual: %1 != Expected: %2) for part %3, ring %4, segment %5, end distance. Returned blank segments: %6" )
            .arg( segments.at( iSegment ).second )
            .arg( expectedSegments.at( iSegment ).second )
            .arg( iPart )
            .arg( iRing )
            .arg( iSegment )
            .arg( strBlankSegments )
            .toLatin1()
            .constData()
        );
      }
    }
  }
}


void TestQgsMapToolEditBlankSegments::testSelectFeature()
{
  TestQgsMapToolUtils utils( mMapToolEditBlankSegments.get() );

  QVERIFY( FID_IS_NULL( mMapToolEditBlankSegments->mCurrentFeatureId ) );

  // select first feature
  utils.mouseClick( 0, 1, Qt::LeftButton );
  QCOMPARE( mMapToolEditBlankSegments->mCurrentFeatureId, 1 );

  QCOMPARE( nbRubberBandVisible(), 0 );
  utils.mouseMove( 0, 1 );
  QCOMPARE( nbRubberBandVisible(), 1 ); // start point rubber band

  // escape
  utils.keyClick( Qt::Key_Escape );
  QVERIFY( FID_IS_NULL( mMapToolEditBlankSegments->mCurrentFeatureId ) );
  QCOMPARE( nbRubberBandVisible(), 0 );

  // select second feature
  utils.mouseClick( 20, 1, Qt::LeftButton );
  QCOMPARE( mMapToolEditBlankSegments->mCurrentFeatureId, 2 );
  utils.mouseMove( 21, 1 );
  QCOMPARE( nbRubberBandVisible(), 1 );

  // escape
  utils.keyClick( Qt::Key_Escape );
  QVERIFY( FID_IS_NULL( mMapToolEditBlankSegments->mCurrentFeatureId ) );
  QCOMPARE( nbRubberBandVisible(), 0 );
}

void TestQgsMapToolEditBlankSegments::testCreateBlankSegment()
{
  TestQgsMapToolUtils utils( mMapToolEditBlankSegments.get() );
  mLayer->startEditing();

  QVERIFY( FID_IS_NULL( mMapToolEditBlankSegments->mCurrentFeatureId ) );

  utils.mouseClick( 0, 1, Qt::LeftButton );
  QCOMPARE( mMapToolEditBlankSegments->mCurrentFeatureId, 1 );

  utils.mouseClick( 11, 2, Qt::LeftButton );
  utils.mouseMove( 9, 9 );
  utils.mouseClick( 9, 9, Qt::LeftButton );

  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.mouseMove( 7, 1 );
  utils.mouseClick( 7, 1, Qt::LeftButton );

  QgsFeature feat = mLayer->getFeature( 1 );
  QVERIFY( feat.isValid() );

  compareBlankSegments( feat.attribute( 1 ).toString(), { { { { 4, 7 }, { 12, 32.8 } } } } );

  // escape
  utils.keyClick( Qt::Key_Escape );
  QVERIFY( FID_IS_NULL( mMapToolEditBlankSegments->mCurrentFeatureId ) );
  QCOMPARE( nbRubberBandVisible(), 0 );

  mLayer->rollBack();
}

void TestQgsMapToolEditBlankSegments::testAuxiliaryStorageAutoCreation()
{
  // reset currently set data defined property
  QgsPropertyCollection c;
  mSymbolLayer->setDataDefinedProperties( c );
  const int propertyKey = static_cast<int>( QgsSymbolLayer::Property::BlankSegments );
  mPropertyButton->init( propertyKey, mSymbolLayer->dataDefinedProperties(), QgsSymbolLayer::propertyDefinitions(), mLayer.get(), true );

  QgsAuxiliaryStorage storage;
  QgsAuxiliaryLayer *layer = storage.createAuxiliaryLayer( mLayer->fields().field( "pk" ), mLayer.get() );
  layer->startEditing();
  mLayer->setAuxiliaryLayer( layer );

  TestQgsMapToolUtils utils( mMapToolEditBlankSegments.get() );

  QVERIFY( FID_IS_NULL( mMapToolEditBlankSegments->mCurrentFeatureId ) );

  utils.mouseClick( 0, 1, Qt::LeftButton );
  QCOMPARE( mMapToolEditBlankSegments->mCurrentFeatureId, 1 );

  int fieldIndex = mLayer->fields().lookupField( "auxiliary_storage_symbol_blanksegments" );
  QCOMPARE( fieldIndex, 2 );
  QCOMPARE( mMapToolEditBlankSegments->mBlankSegmentsFieldIndex, 2 );

  QCOMPARE( layer->featureCount(), 0 );

  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.mouseMove( 7, 1 );
  utils.mouseClick( 7, 1, Qt::LeftButton );

  QCOMPARE( layer->featureCount(), 1 );

  QgsFeature feat = mLayer->getFeature( 1 );
  QVERIFY( feat.isValid() );

  compareBlankSegments( feat.attribute( fieldIndex ).toString(), { { { { 4, 7 } } } } );

  // now test when property contains already an expression

  c = mSymbolLayer->dataDefinedProperties();
  const QgsProperty expressionProperty = QgsProperty::fromExpression( u"'(((5 5)))'"_s, true );
  c.setProperty( propertyKey, expressionProperty );
  mSymbolLayer->setDataDefinedProperties( c );
  mPropertyButton->setToProperty( expressionProperty );
  emit mPropertyButton->changed(); // setToProperty doesn't fire the changed signal

  QCOMPARE( mMapToolEditBlankSegments->mBlankSegmentsFieldIndex, -1 );
  QCOMPARE( nbRubberBandVisible(), 0 );
  QVERIFY( FID_IS_NULL( mMapToolEditBlankSegments->mCurrentFeatureId ) );
  QCOMPARE( mMapToolEditBlankSegments->mState, QgsMapToolEditBlankSegmentsBase::State::SelectFeature );

  // select first feature
  utils.mouseClick( 0, 1, Qt::LeftButton );
  QCOMPARE( mMapToolEditBlankSegments->mCurrentFeatureId, 1 );
  QCOMPARE( nbRubberBandVisible(), 0 );

  fieldIndex = mLayer->fields().lookupField( "auxiliary_storage_symbol_blanksegments_2" );
  QCOMPARE( fieldIndex, 3 );
  QCOMPARE( mMapToolEditBlankSegments->mBlankSegmentsFieldIndex, 3 );

  c = mSymbolLayer->dataDefinedProperties();
  QgsProperty newProperty = c.property( propertyKey );
  QVERIFY( newProperty.isActive() );
  QCOMPARE( newProperty.propertyType(), Qgis::PropertyType::Expression );
  QCOMPARE( newProperty.expressionString(), "coalesce(\"auxiliary_storage_symbol_blanksegments_2\",'(((5 5)))')" );

  QCOMPARE( layer->featureCount(), 1 );

  // create blank segment
  utils.mouseClick( 11, 2, Qt::LeftButton );
  utils.mouseMove( 9, 9 );
  utils.mouseClick( 9, 9, Qt::LeftButton );

  QCOMPARE( nbRubberBandVisible(), 2 ); // the one created + start point

  QCOMPARE( layer->featureCount(), 1 );

  feat = mLayer->getFeature( 1 );
  QVERIFY( feat.isValid() );

  compareBlankSegments( feat.attribute( fieldIndex ).toString(), { { { { 12, 32.8 } } } } );

  mLayer->rollBack();
}


QGSTEST_MAIN( TestQgsMapToolEditBlankSegments )
#include "testqgsmaptooleditblanksegments.moc"
